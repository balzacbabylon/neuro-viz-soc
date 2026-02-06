# Workflows

## Data Preparation
1. **Brainstorm**: Export `ImagingKernel` and raw EEG data.
2. **Matlab**: Run `modeling/matlab/quantize.m` (to be created) to convert double -> fixed-point (Q15.16).
3. **HPS**: Transfer binary files to SD Card or stream via Ethernet.

## Build Process
1. **FPGA**:
    - Open `hardware/quartus/project.qpf`.
    - Components (IP) in `hardware/qsys`.
    - Compile -> `output_files/project.sof`.
    - Program FPGA.
2. **Software**:
    - `cd software`
    - `make`
    - `./app`
