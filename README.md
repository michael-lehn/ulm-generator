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

### Requirements

The debugger requires an installation of the excellent `finalcut` library, which can be found at: [finalcut GitHub repository](https://github.com/gansm/finalcut). Make sure to modify the `config/finalcut` file to include the correct paths for `finalcut` library and header files.
