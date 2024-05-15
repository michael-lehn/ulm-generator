## ulm-generator

A generator for a ULM (Ulm Lecture Machine) virtual machine.

From a description of an ISA (Instruction Set Architecture), it generates the following tools:

- a virtual machine for your ISA: executable named `ulm.exe`
- a debugger with a text user interface: executable named `udb-tui.exe`
- an assembler for the ULM: executable named `ulmas.exe`

Additionally, it can generate documentation for the ISA:
- File name: `refman.pdf`

### Installation

A simple `make install` will install `ulm-generator` in `/usr/local/bin`. To change the installation directory you can specify a prefix. For example, `make install prefix=/foo/bar` will install `ulm-generator` in `/foo/bar/bin`.

### Usage

`ulm-generator --help` displays the usage and available options. Some first steps might be:

- `ulm-generator --fetch` displays a list of available demo ISAs. One ISA is `ulm-ice40.isa`.
- `ulm-generator --fetch ulm-ice40.isa` copies the ISA file `ulm-ice40.isa` to the current directory.
- `ulm-generator ulm-ice40.isa` creates a subdirectory `ulm-ise40` and builds the tools `ulm.exe`, `udb-tui.exe`, `ulmas.exe`.
- `ulm-generator ulm-ice40.isa --refman` generates in addition to these tools a reference manual for the assembler instructions.
- `ulm-generator ulm-ice40.isa --install` installs the tools in `/usr/local/bin`.
- `ulm-generator ulm-ice40.isa --install --prefix=/foo/bar` installs the tools in `/foo/bar/bin`.

The [ulm-on-ice](https://github.com/michael-lehn/ulm-on-ice) implements this ISA on a FPGA and therefore can execute all programs written for this architecture.

## Examples

This is a hello world program in machine code for the `ulm-ice40.isa`:

```
10 10 00 20 
20 21 00 00                                                                     
14 02 00 00                                                                     
04 00 00 04 
30 20 00 00 
12 11 00 01 
05 FF FF FB 
01 41 00 00 
68  65  6c  6c  6f  2c  20  77  6f  72  6c  64  21  0a 00
```

If you save it in a file `hello` you can execute it with `ulm hello`. For using the debugger run `udb-tui hello`:

- With `Control-S` you can execute the program step-wise.
- With `Control-R` the program gets executed until either a break point is reached or the program has terminated.
- With `Control-Q` you can exit the debugger.

The same program in assembly:

```s
    .data
msg .string "hello, world!\n"                                                   
                                                                                
    .equ    p, 1
    .equ    ch, 2

    .text
    loadz   msg,    %1
loop:
    movzbq  (%1),   %2
    subq    0,      %2,     %0
    jz      halt
    putc    %2
    addq    1,      %1,     %1
    jmp     loop
halt:
    halt    0
```

If you save it in a file `hello.s` machine code can be generated from it with `ulmas hello.s`. This creates `a.out`. With `ulmas -o hello hello.s` the output file will be called `hello`.

### Requirements

The debugger requires an installation of the excellent `finalcut` library, which can be found at: [finalcut GitHub repository](https://github.com/gansm/finalcut). Make sure to modify the `config/finalcut` file to include the correct paths for `finalcut` library and header files.
