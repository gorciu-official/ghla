#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "ghla-program.hpp"
#include "helpers.hpp"

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

    for (auto& s : p.sections) {
        o << "section " << s.name << "\n";

        for (auto& l : s.lines) {
            if (l.type == GHLAProgram::Line::RAW_ASM) {
                o << "    " << l.text << "\n";

                if (p.append_str_length && s.name == ".data") {
                    std::string label;
                    if (is_string_db(l.text, label)) {
                        o << "    " << label
                        << "_len equ $ - " << label << "\n";
                    }
                }
            }
            else if (l.type == GHLAProgram::Line::SYSCALL) {
                emit_syscall(o, p, l);
            }
        }

        o << "\n";
    }
}