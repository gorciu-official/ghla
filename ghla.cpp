#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>

struct GHLAProgram {
    std::string name;
    int bits;

    std::vector<std::string> imports;
    std::vector<std::string> exports;

    struct Section {
        std::string name;
        std::vector<std::string> code;
    };

    std::vector<Section> sections;
};

bool starts_with(const std::string& s, const std::string& p) {
    return s.rfind(p, 0) == 0;
}

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t");
    size_t e = s.find_last_not_of(" \t");
    if (b == std::string::npos) return "";
    return s.substr(b, e - b + 1);
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string replace_extension(const std::string& filename, const std::string& new_ext) {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return filename + new_ext;
    return filename.substr(0, pos) + new_ext;
}

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
        else if (starts_with(line, "import ")) {
            prog.imports.push_back(trim(line.substr(7)));
        }
        else if (starts_with(line, "section ")) {
            prog.sections.push_back({});
            current = &prog.sections.back();
            current->name = trim(line.substr(8));
        }
        else if (starts_with(line, "export ")) {
            prog.exports.push_back(trim(line.substr(7)));
        }
        else {
            if (!current)
                throw std::runtime_error("asm outside section");
            current->code.push_back(line);
        }
    }

    return prog;
}

void emit_nasm(const GHLAProgram& p, const std::string& out) {
    std::ofstream o(out);

    o << "bits " << p.bits << "\n\n";

    for (auto& i : p.imports)
        o << "extern " << i << "\n";

    for (auto& e : p.exports)
        o << "global " << e << "\n";

    o << "\n";

    for (auto& s : p.sections) {
        o << "section " << s.name << "\n";
        for (auto& l : s.code)
            o << "    " << l << "\n";
        o << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " file.ghla\n";
        return 1;
    }

    std::string input = argv[1];

    if (!ends_with(input, ".ghla")) {
        std::cerr << "Error: file must have .ghla extension\n";
        return 1;
    }

    std::string asm_file = replace_extension(input, ".asm");
    std::string obj_file = replace_extension(input, ".o");
    std::string elf_file = replace_extension(input, ".elf");

    try {
        GHLAProgram prog = parse_ghla(input);
        emit_nasm(prog, asm_file);
    } catch (const std::exception& e) {
        std::cerr << "GHLA error: " << e.what() << "\n";
        return 1;
    }

    std::string nasm_cmd = "nasm -f elf64 " + asm_file + " -o " + obj_file;
    std::string ld_cmd   = "ld " + obj_file + " -o " + elf_file;

    if (std::system(nasm_cmd.c_str()) != 0) {
        std::cerr << "Error: NASM failed\n";
        return 1;
    }

    if (std::system(ld_cmd.c_str()) != 0) {
        std::cerr << "Error: linker failed\n";
        return 1;
    }

    std::cout << "Build successful: " << elf_file << "\n";
    return 0;
}
