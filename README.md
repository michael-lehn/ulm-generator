# ulm-generator

A generator for a ULM (Ulm Lecture Machine) virtual machine.

From a description of an ISA (Instruction Set Architecture), it generates the following tools using `make`:

- a virtual machine for your ISA: executable named `ulm`
- a debugger with a text user interface: executable named `udb-tui`
- an assembler for the ULM: executable named `ulmas`

Additionally, it can generate documentation for the ISA (using `make refman`):
- File name: `refman.pdf`

You can define multiple ISAs, and for each ISA, the above tools are generated. For each ISA variant, all the tools are generated in `1_ulm_build/NAME_OF_VARIANT/`.

See `0_ulm_variants/ulm/isa.txt` for a fully-featured example and `0_ulm_variants/my_isa/isa.txt` for a starting point example. After running `make`, you will find all the tools generated from `0_ulm_variants/ulm/isa.txt` in `1_ulm_build/ulm/`, and the tools generated from `0_ulm_variants/my_isa/isa.txt` in `1_ulm_build/my_isa/`.

After running `make refman`, you will also have `1_ulm_build/ulm/refman.pdf` generated from `0_ulm_variants/ulm/isa.txt` and `1_ulm_build/my_isa/refman.pdf` generated from `0_ulm_variants/my_isa/isa.txt`.

# Requirements

The debugger requires an installation of the excellent `finalcut` library: https://github.com/gansm/finalcut. Modify `config/finalcut` to contain the correct include and library path for `finalcut`.

# Compile

A simple `make` should suffice. However, if `finalcut` is not installed or `config/finalcut` is incorrect, the debugger module will be ignored.

# Examples

For testing `udb-tui`, run `./1_ulm_build/ulm/udb-tui 0_ulm_variants/ulm/examples/hello`.
