#include <sstream>
#include <fstream>
#include "ghla-program.hpp"
#include "helpers.hpp"

GHLAProgram parse_ghla(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("cannot open file");

    GHLAProgram prog;
    GHLAProgram::Section* current = nullptr;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') continue;

        if (starts_with(line, "program ")) {
            prog.name = trim(line.substr(8));
        }
        else if (starts_with(line, "bits ")) {
            prog.bits = std::stoi(trim(line.substr(5)));
        }
        else if (starts_with(line, "feature enable ")) {
            std::string feat = trim(line.substr(15));
            if (feat == "shorten_syscalls") {
                prog.shorten_syscalls = true;
            } else if (feat == "append_str_length") {
                prog.append_str_length = true;
            } else if (feat == "syscall_constants") {
                prog.syscall_constants = true;
            } else if (feat == "new_regs_instructions") {
                prog.new_regs_instructions = true;
            } else {
                throw std::runtime_error("unknown feature: " + feat);
            }
        }
        else if (starts_with(line, "push_cregs")) {
            if (!current)
                throw std::runtime_error("push_cregs outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::PUSH_CREGS;

            std::string args = trim(line.substr(8));
            std::stringstream ss(args);
            std::string arg;

            while (std::getline(ss, arg, ',')) {
                l.args.push_back(trim(arg));
            }

            current->lines.push_back(l);
        }
        else if (starts_with(line, "pop_cregs")) {
            if (!current)
                throw std::runtime_error("pop_cregs outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::POP_CREGS;

            std::string args = trim(line.substr(8));
            std::stringstream ss(args);
            std::string arg;

            while (std::getline(ss, arg, ',')) {
                l.args.push_back(trim(arg));
            }

            current->lines.push_back(l);
        }
        else if (starts_with(line, "import ")) {
            prog.imports.push_back(trim(line.substr(7)));
        }
        else if (starts_with(line, "section ")) {
            prog.sections.push_back({});
            current = &prog.sections.back();
            current->name = trim(line.substr(8));
        }
        else if (starts_with(line, "export ") && ends_with(line, ":")) {
            std::string lbl = trim(line.substr(7));
            lbl.pop_back(); 
            prog.exports.push_back(lbl);

            if (!current)
                throw std::runtime_error("export label outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::RAW_ASM;

            l.text = trim(line.substr(7, line.size() - 1));
            current->lines.push_back(l);
        }
        else if (starts_with(line, "export ")) {
            prog.exports.push_back(trim(line.substr(7)));
        }
        else if (starts_with(line, "syscall ")) {
            if (!current)
                throw std::runtime_error("syscall outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::SYSCALL;

            std::string args = trim(line.substr(8));
            std::stringstream ss(args);
            std::string arg;

            while (std::getline(ss, arg, ',')) {
                l.args.push_back(trim(arg));
            }

            current->lines.push_back(l);
        }
        else {
            if (!current)
                throw std::runtime_error("asm outside section");

            GHLAProgram::Line l;
            l.type = GHLAProgram::Line::RAW_ASM;
            l.text = line;
            current->lines.push_back(l);
        }
    }

    return prog;
}