#include <sstream>
#include <fstream>
#include "ghla-program.hpp"
#include "helpers.hpp"

static std::vector<std::string> split_args(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string arg;

    while (std::getline(ss, arg, ',')) {
        arg = trim(arg);
        if (!arg.empty())
            out.push_back(arg);
    }
    return out;
}

GHLAProgram parse_ghla(const std::string& filename) {
    std::ifstream in(filename);
    if (!in)
        throw std::runtime_error("cannot open file");

    GHLAProgram prog;
    GHLAProgram::Section* current = nullptr;

    bool in_macro = false;
    GHLAProgram::Macro current_macro;

    std::string line;
    size_t line_no = 0;

    while (std::getline(in, line)) {
        line_no++;

        auto comment = line.find(';');
        if (comment != std::string::npos)
            line = line.substr(0, comment);

        line = trim(line);
        if (line.empty())
            continue;

        if (starts_with(line, "program ")) {
            prog.name = trim(line.substr(8));
        }
        else if (starts_with(line, "bits ")) {
            prog.bits = std::stoi(trim(line.substr(5)));
        }
        else if (starts_with(line, "feature enable ")) {
            std::string feat = trim(line.substr(15));

            if (feat == "shorten_syscalls") prog.shorten_syscalls = true;
            else if (feat == "append_str_length") prog.append_str_length = true;
            else if (feat == "syscall_constants") prog.syscall_constants = true;
            else if (feat == "new_regs_instructions") prog.new_regs_instructions = true;
            else if (feat == "macros") prog.macros_enabled = true;
            else
                throw std::runtime_error("unknown feature: " + feat);
        }
        else if (starts_with(line, "macro ")) {
            if (!prog.macros_enabled)
                throw std::runtime_error("macros used without feature enable macros");

            if (in_macro)
                throw std::runtime_error("nested macros not allowed");

            in_macro = true;
            current_macro = {};

            std::string rest = trim(line.substr(6));
            size_t bang = rest.find('!');

            if (bang == std::string::npos || bang == 0)
                throw std::runtime_error("invalid macro name");

            current_macro.name = rest.substr(0, bang + 1);

            if (prog.macros.count(current_macro.name))
                throw std::runtime_error("macro redefinition: " + current_macro.name);

            std::string params = trim(rest.substr(bang + 1));
            current_macro.params = split_args(params);
        }
        else if (line == "endmacro") {
            if (!in_macro)
                throw std::runtime_error("endmacro outside macro");

            prog.macros[current_macro.name] = current_macro;
            in_macro = false;
        }
        else if (in_macro) {
            GHLAProgram::Line l;

            if (starts_with(line, "syscall ")) {
                l.type = GHLAProgram::Line::SYSCALL;
                l.args = split_args(trim(line.substr(8)));
            } else {
                l.type = GHLAProgram::Line::RAW_ASM;
                l.text = line;
            }

            current_macro.body.push_back(l);
        }
        else if (starts_with(line, "section ")) {
            prog.sections.push_back({});
            current = &prog.sections.back();
            current->name = trim(line.substr(8));
        }
        else if (starts_with(line, "import ")) {
            prog.imports.push_back(trim(line.substr(7)));
        }
        else if (starts_with(line, "export ")) {
            prog.exports.push_back(trim(line.substr(7)));
        }
        else if (starts_with(line, "push_cregs")) {
            if (!current)
                throw std::runtime_error("push_cregs outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::PUSH_CREGS;
            l.args = split_args(trim(line.substr(10)));
            current->lines.push_back(l);
        }
        else if (starts_with(line, "pop_cregs")) {
            if (!current)
                throw std::runtime_error("pop_cregs outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::POP_CREGS;
            l.args = split_args(trim(line.substr(9)));
            current->lines.push_back(l);
        }
        else if (starts_with(line, "syscall ")) {
            if (!current)
                throw std::runtime_error("syscall outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::SYSCALL;
            l.args = split_args(trim(line.substr(8)));
            current->lines.push_back(l);
        }
        else if (ends_with(line, "!")) {
            if (!current)
                throw std::runtime_error("macro call outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::MACRO_CALL;

            std::string name = trim(line);
            size_t space = name.find(' ');
            if (space != std::string::npos) {
                l.text = name.substr(0, space + 1);
                l.args = split_args(name.substr(space + 1));
            } else {
                l.text = name;
            }

            current->lines.push_back(l);
        }
        else if (current) {
            std::string token = line.substr(0, line.find_first_of(" \t"));
            if (prog.macros.count(token)) {
                GHLAProgram::Line l;
                l.type = GHLAProgram::Line::MACRO_CALL;
                l.text = token;

                std::string args = trim(line.substr(token.size()));
                if (!args.empty())
                    l.args = split_args(args);

                current->lines.push_back(l);
            } else {
                GHLAProgram::Line l;
                l.type = GHLAProgram::Line::RAW_ASM;
                l.text = line;
                current->lines.push_back(l);
            }
        } else {
            throw std::runtime_error("asm outside section");
        }
    }

    if (in_macro)
        throw std::runtime_error("unexpected EOF inside macro");

    return prog;
}
