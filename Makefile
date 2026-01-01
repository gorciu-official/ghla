all:
	@g++ src/*.cpp -Iinclude -o ghla.bin

example_minimal: all
	@./ghla.bin examples/minimal.ghla

test_example_minimal: example_minimal
	@chmod +x ./examples/minimal.elf
	@./examples/minimal.elf

example_print: all
	@./ghla.bin examples/print.ghla

test_print_minimal: example_print
	@chmod +x ./examples/print.elf
	@./examples/print.elf

example_open: all
	@./ghla.bin examples/open.ghla

test_open_minimal: example_open
	@chmod +x ./examples/open.elf
	@./examples/open.elf

clean:
	rm -rf examples/*.asm
	rm -rf examples/*.o
	rm -rf examples/*.elf
	rm -rf ghla.bin