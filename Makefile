all:
	g++ ghla.cpp -o ghla.bin

example_minimal: all
	./ghla.bin examples/minimal.ghla

test_example_minimal: example_minimal
	chmod +x ./examples/minimal.elf
	./examples/minimal.elf

example_print: all
	./ghla.bin examples/print.ghla

test_print_minimal: example_print
	chmod +x ./examples/print.elf
	./examples/print.elf

clean:
	rm -rf examples/*.asm
	rm -rf examples/*.o
	rm -rf examples/*.elf
	rm -rf ghla.bin