#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "ghla-program.hpp"
#include "helpers.hpp"
#include "snippets.hpp"

void emit_syscall(std::ofstream& o,
                  const GHLAProgram& p,
                  const GHLAProgram::Line& l) {
    static const char* regs[] = {
        "rax",
        "rdi",
        "rsi",
        "rdx",
        "r10",
        "r8",
        "r9"
    };

    if (!p.shorten_syscalls && !l.args.empty()) 
        throw std::runtime_error("syscall should not be used with arguments\nHelp: you probably want to include this line at the beginning of your code: `feature enable shorten_syscalls`");

    if (!p.shorten_syscalls) {
        o << "    syscall\n";
        return;
    }

    if (l.args.empty())
        throw std::runtime_error("syscall requires arguments");

    if (l.args.size() > 7)
        throw std::runtime_error("too many syscall arguments");

    for (size_t i = 0; i < l.args.size(); i++) {
        o << "    mov " << regs[i] << ", " << l.args[i] << "\n";
    }

    o << "    syscall\n";
}

bool is_string_db(const std::string& line, std::string& label_out) {
    std::string l = trim(line);
    size_t db_pos = l.find("db ");
    if (db_pos == std::string::npos) return false;

    std::string before = trim(l.substr(0, db_pos));
    if (before.empty()) return false;

    label_out = before;
    return l.find('"') != std::string::npos;
}

void emit_nasm(const GHLAProgram& p, const std::string& out) {
    std::ofstream o(out);
    if (!o)
        throw std::runtime_error("cannot create asm file");

    o << "bits " << p.bits << "\n\n";

    for (auto& i : p.imports)
        o << "extern " << i << "\n";

    for (auto& e : p.exports)
        o << "global " << e << "\n";

    o << "\n";

    if (p.syscall_constants) {
        o << snippet_syscall_constants;
    }

    auto emit_line = [&](const GHLAProgram::Line& l, auto&& emit_line_ref) -> void {
        switch (l.type) {
            case GHLAProgram::Line::RAW_ASM:
                o << "    " << l.text << "\n";

                if (p.append_str_length && !l.text.empty()) {
                    std::string label;
                    if (is_string_db(l.text, label))
                        o << "    " << label << "_len equ $ - " << label << "\n";
                }
                break;

            case GHLAProgram::Line::SYSCALL:
                emit_syscall(o, p, l);
                break;

            case GHLAProgram::Line::POP_CREGS:
                if (!p.new_regs_instructions)
                    throw std::runtime_error(
                        "this instruction does not exist without the feature `new_regs_instructions`\n"
                        "Help: include `feature enable new_regs_instructions` at the beginning of your code"
                    );
                o << snippet_pop_cregs;
                break;

            case GHLAProgram::Line::PUSH_CREGS:
                if (!p.new_regs_instructions)
                    throw std::runtime_error(
                        "this instruction does not exist without the feature `new_regs_instructions`\n"
                        "Help: include `feature enable new_regs_instructions` at the beginning of your code"
                    );
                o << snippet_push_cregs;
                break;

            case GHLAProgram::Line::MACRO_CALL: {
                auto it = p.macros.find(l.text);
                if (it == p.macros.end())
                    throw std::runtime_error("macro not found: " + l.text);

                const GHLAProgram::Macro& m = it->second;

                if (l.args.size() != m.params.size())
                    throw std::runtime_error(
                        "macro argument count mismatch for " + l.text
                    );

                std::unordered_map<std::string, std::string> map;
                for (size_t i = 0; i < l.args.size(); i++)
                    map[m.params[i]] = l.args[i];

                for (auto& body_line : m.body) {
                    GHLAProgram::Line expanded = body_line;

                    if (expanded.type == GHLAProgram::Line::RAW_ASM) {
                        for (auto& kv : map)
                            expanded.text = replace_all(expanded.text, kv.first, kv.second);
                    } else if (expanded.type == GHLAProgram::Line::SYSCALL) {
                        for (auto& arg : expanded.args)
                            for (auto& kv : map)
                                arg = replace_all(arg, kv.first, kv.second);
                    }

                    emit_line_ref(expanded, emit_line_ref);
                }

                break;
            }

            default:
                throw std::runtime_error("token type unknown");
        }
    };

    for (auto& s : p.sections) {
        o << "section " << s.name << "\n";

        for (auto& l : s.lines)
            emit_line(l, emit_line);

        o << "\n";
    }
}