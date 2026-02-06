# API Reference

## HPS-FPGA Interface
### Control (LWHPS2FPGA)
Base Address: `0xFF200000` (check Qsys)
- `offset 0x00`: CSR (Control/Status Register)
    - bit 0: START
    - bit 1: RESET
    - bit 2: DONE (Read-only)

### Data (HPS2FPGA)
Base Address: `0xC0000000` (check Qsys)
- Mapped to FPGA On-chip Memory or FIFO.
- Write 16-bit / 32-bit integers representing Q15.16 fixed point values.

## Software API
See `software/include/*.h` for details.
