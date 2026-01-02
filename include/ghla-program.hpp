#include <vector>
#include <string>
#include <unordered_map>

struct GHLAProgram {
    std::string name;
    int bits = 64;

    bool shorten_syscalls = false;
    bool append_str_length = false;
    bool syscall_constants = false;
    bool new_regs_instructions = false;
    bool macros_enabled = false;

    std::vector<std::string> imports;
    std::vector<std::string> exports;

    struct Line {
        enum Type {
            RAW_ASM,
            SYSCALL,
            POP_CREGS,
            PUSH_CREGS,
            MACRO_DEF,
            MACRO_CALL,
        } type;

        std::string text;
        std::vector<std::string> args;
    };

    struct Section {
        std::string name;
        std::vector<Line> lines;
    };

    std::vector<Section> sections;

    struct Macro {
        std::string name;
        std::vector<std::string> params;
        std::vector<GHLAProgram::Line> body;
    };

    std::unordered_map<std::string, Macro> macros;
};

extern GHLAProgram parse_ghla(const std::string& filename);
extern void emit_nasm(const GHLAProgram& p, const std::string& out);