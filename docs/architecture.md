# System Architecture

## Component Map
- **HPS (Host)**: Runs C code (`software/src`) handling EEG data streaming, control logic, and potentially asset management.
- **FPGA (Device)**:
    - **RTL**: Hand-written Verilog for Ray Tracing core / custom logic.
    - **HLS**: Generated IP for EEG Inverse Problem (sLORETA/wMNE).
    - **Qsys**: Connects HPS and FPGA via AXI bridges.
        - *LWHPS2FPGA*: Control signals (CSRs).
        - *HPS2FPGA*: High-bandwidth data streaming (EEG samples).

## Data Flow
1. **Input**: Raw EEG data (simulation/stream) -> HPS.
2. **Process**: HPS -> FPGA (Inverse Model IP) -> 3D Source Reconstruction.
3. **Render**: 3D Sources -> Ray Tracing Engine (FPGA/Software Hybrid) -> Video Output (VGA/HDMI).
