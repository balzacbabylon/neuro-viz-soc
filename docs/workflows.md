# Workflows

## Data Preparation
**Goal**: Generate "Golden Vectors" to verify hardware implementation. The DE1-SoC has ~4.4 Mbits of on-chip block RAM (M10K). A Full Cortex Model (15,000 sources × 64 sensors × 16 bit ≈ 15.3 Mbits) won't fit in BRAM, so we use a Scout-based (ROI) kernel.

### Steps (Host PC - Brainstorm)
1. **Generate Inverse**: In Brainstorm, compute sources using wMNE.
2. **Extract Scouts**: Use the "Scouts" tab to project sources to the ~68 anatomical regions (Select atlas -> Scout time series).
   - *Note*: Using the Desikan-Killiany atlas (approx. 68 regions), the matrix becomes 68×64×16bit ≈ 70 Kbits. This fits easily and leaves room for standard vertex-based localized patches to scale up later.
3. **Export Matrix**: You need the `ImagingKernel` that maps Sensors → Scouts.
   - *Important*: Brainstorm usually computes Sources=K⋅Data, then averages sources into Scouts. For the FPGA, pre-multiply the Scout interpolation matrix with the Imaging Kernel to get a single 68×64 matrix.
4. **Export Data**: Export a 10-second chunk of raw EEG data (64×Nsamples) as `eeg_stream.bin`.
5. **Quantization**: Convert both the Matrix and Data to Fixed Point (Q15.16) in Matlab/Python and save as binary files.
6. **Transfer**: Transfer binary files to the HPS (SD Card or Ethernet).

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
