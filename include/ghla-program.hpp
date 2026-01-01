#include <vector>
#include <string>

struct GHLAProgram {
    std::string name;
    int bits = 64;

    bool shorten_syscalls = false;
    bool append_str_length = false;

    std::vector<std::string> imports;
    std::vector<std::string> exports;

    struct Line {
        enum Type {
            RAW_ASM,
            SYSCALL
        } type;

        std::string text;
        std::vector<std::string> args;
    };

    struct Section {
        std::string name;
        std::vector<Line> lines;
    };

    std::vector<Section> sections;
};

extern GHLAProgram parse_ghla(const std::string& filename);
extern void emit_nasm(const GHLAProgram& p, const std::string& out);