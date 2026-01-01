#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "ghla-program.hpp"
#include "helpers.hpp"

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
