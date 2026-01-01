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
        o << "SYSCALL_READ          equ 0\n";
        o << "SYSCALL_WRITE         equ 1\n";
        o << "SYSCALL_OPEN          equ 2\n";
        o << "SYSCALL_CLOSE         equ 3\n";
        o << "SYSCALL_STAT          equ 4\n";
        o << "SYSCALL_FSTAT         equ 5\n";
        o << "SYSCALL_LSTAT         equ 6\n";
        o << "SYSCALL_POLL          equ 7\n";
        o << "SYSCALL_LSEEK         equ 8\n";
        o << "SYSCALL_MMAP          equ 9\n";
        o << "SYSCALL_MPROTECT      equ 10\n";
        o << "SYSCALL_MUNMAP        equ 11\n";
        o << "SYSCALL_BRK           equ 12\n";
        o << "SYSCALL_RT_SIGACTION  equ 13\n";
        o << "SYSCALL_RT_SIGPROCMASK equ 14\n";
        o << "SYSCALL_RT_SIGRETURN  equ 15\n";
        o << "SYSCALL_IOCTL         equ 16\n";
        o << "SYSCALL_PREAD64       equ 17\n";
        o << "SYSCALL_PWRITE64      equ 18\n";
        o << "SYSCALL_READV         equ 19\n";
        o << "SYSCALL_WRITEV        equ 20\n";
        o << "SYSCALL_ACCESS        equ 21\n";
        o << "SYSCALL_PIPE          equ 22\n";
        o << "SYSCALL_SELECT        equ 23\n";
        o << "SYSCALL_SCHED_YIELD   equ 24\n";
        o << "SYSCALL_MREMAP        equ 25\n";
        o << "SYSCALL_MSYNC         equ 26\n";
        o << "SYSCALL_MINCORE       equ 27\n";
        o << "SYSCALL_MADVISE       equ 28\n";
        o << "SYSCALL_SHMGET        equ 29\n";
        o << "SYSCALL_SHMAT         equ 30\n";
        o << "SYSCALL_SHMCTL        equ 31\n";
        o << "SYSCALL_DUP           equ 32\n";
        o << "SYSCALL_DUP2          equ 33\n";
        o << "SYSCALL_PAUSE         equ 34\n";
        o << "SYSCALL_NANOSLEEP     equ 35\n";
        o << "SYSCALL_GETITIMER     equ 36\n";
        o << "SYSCALL_ALARM         equ 37\n";
        o << "SYSCALL_SETITIMER     equ 38\n";
        o << "SYSCALL_GETPID        equ 39\n";
        o << "SYSCALL_SENDSIG       equ 40\n";
        o << "SYSCALL_SENDFILE      equ 41\n";
        o << "SYSCALL_SOCKET        equ 41\n";
        o << "SYSCALL_CONNECT       equ 42\n";
        o << "SYSCALL_ACCEPT        equ 43\n";
        o << "SYSCALL_EXIT          equ 60\n";
        o << "SYSCALL_FORK          equ 57\n";
        o << "SYSCALL_VFORK         equ 58\n";
        o << "SYSCALL_EXECVE        equ 59\n";
        o << "SYSCALL_WAIT4         equ 61\n";
        o << "SYSCALL_KILL          equ 62\n";
        o << "SYSCALL_UNAME         equ 63\n";
        o << "SYSCALL_GETTIMEOFDAY  equ 96\n";
        o << "SYSCALL_CLOCK_GETTIME equ 228\n\n";
    }

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