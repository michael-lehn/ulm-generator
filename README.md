# ulm-generator

Generator for a ULM (Ulm Lecture Machine) virtual machine.

From a description of an ISA (Instruction Set Architecture) it generates:

- the ULM (executable name `ulm`)
- a debugger for the ULM with a text user interface (executable name `udb-tui`)
- an assembler for the ULM (executable name `ulmas`)

You can define several ISAs, and for each ISA the above tools are generated. See `0_ulm_variants/ulm/isa.txt` for a full featured example and `0_ulm_variants/ex/isa.txt` for a starting point example. The tool are then generated in `1_ulm_build/NAME_OF_VARIANT/`.

# Requirements

The debugger requires an installtion of the great `finalcut` library: https://github.com/gansm/finalcut. Modify `config/finalcut` such that it contains the correct inlcude and library path for `finalcut`.

# Compile

A simple `make` should do. However, if `finalcut` is not installed or `config/finalcut` not correct the debugger module will be ignored.

# Examples

For testing `udb-tui` run `./1_ulm_build/ulm/udb-tui 0_ulm_variants/ulm/examples/hello`
