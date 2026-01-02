#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include "ghla-program.hpp"
#include "helpers.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [options] file1.ghla|.o [file2.ghla|.o ...]\n";
        std::cerr << "Options:\n";
        std::cerr << "  -o <output>           Set output ELF file name (overrides default)\n";
        std::cerr << "  --linker-flags <flags> Pass extra flags to the linker\n";
        return 1;
    }

    std::vector<std::string> obj_files;
    std::string elf_file;
    std::string linker_flags;
    std::vector<std::string> input_files;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -o requires a file name\n";
                return 1;
            }
            elf_file = argv[++i];
        }
        else if (arg == "--linker-flags") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --linker-flags requires a string of flags\n";
                return 1;
            }
            linker_flags = argv[++i];
        }
        else {
            input_files.push_back(arg);
            std::string input = arg;

            if (ends_with(input, ".ghla")) {
                std::string asm_file = replace_extension(input, ".asm");
                std::string obj_file = replace_extension(input, ".o");

                try {
                    GHLAProgram prog = parse_ghla(input);
                    emit_nasm(prog, asm_file);
                } catch (const std::exception& e) {
                    std::cerr << "GHLA error in " << input << ": " << e.what() << "\n";
                    return 1;
                }

                std::string nasm_cmd = "nasm -f elf64 " + asm_file + " -o " + obj_file;
                if (std::system(nasm_cmd.c_str()) != 0) {
                    std::cerr << "Error: NASM failed for " << asm_file << "\n";
                    return 1;
                }

                obj_files.push_back(obj_file);
            }
            else if (ends_with(input, ".o")) {
                obj_files.push_back(input);
            }
            else {
                std::cerr << "Error: file must have .ghla or .o extension: " << input << "\n";
                return 1;
            }
        }
    }

    if (obj_files.empty()) {
        std::cerr << "Error: no input files to link\n";
        return 1;
    }

    // jeśli użytkownik nie podał -o, ustawiamy nazwę ELF
    if (elf_file.empty()) {
        if (input_files.size() == 1) {
            elf_file = replace_extension(input_files[0], ".elf");
        } else {
            elf_file = "output.elf";
        }
    }

    std::string ld_cmd = "ld";
    for (auto& o : obj_files)
        ld_cmd += " " + o;
    if (!linker_flags.empty())
        ld_cmd += " " + linker_flags;
    ld_cmd += " -o " + elf_file;

    if (std::system(ld_cmd.c_str()) != 0) {
        std::cerr << "Error: linker failed\n";
        return 1;
    }

    std::cout << "Build successful: " << elf_file << "\n";
    return 0;
}