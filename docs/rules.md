# Rules

## Constraint Management
- **Timing**: FPGA design must meet timing @ 50MHz (or target clock).
- **Memory**: M10K blocks are limited. Large LUTs/arrays must live in SDRAM if they exceed ~4Mb.

## Coding Standards
- **C style**: Linux kernel style (mostly).
- **Verilog**:
    - `input`, `output` declarations one per line.
    - `always_ff @(posedge clk)` for sequential.
    - `always_comb` for combinational.
    - Module names match filenames.
