# GHLA

<div align="center"><img width="522" height="379" alt="image" src="https://github.com/user-attachments/assets/043738d5-d57b-4ee0-9741-ae39f8861e67" /></div>

Assembly. Simplified.

## Features

GHLA extends Assembly with these optional features:

- shorten syscalls
- append string length
- syscall constants (SYSCALL_WRITE, SYSCALL_OPEN, etc.)
- new register instructions

## Building

This project requires these tools in order to run:
- nasm
- glibc

This project requires these tools in order to build:
- sh
- make
- g++
- ld

To build GHLA, please run the following command: `make`.

## ToDo

All of things here shall be a `feature` that can be turned on, but it isn't by default.

- namespaces
- better ifs
- printf
- better variables
- shorten function calls
- better for-loop and while-loop
- structures