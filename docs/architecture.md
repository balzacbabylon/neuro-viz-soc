# System Architecture

## Component Map
- **HPS (Host)**: Runs C code (`software/src`) handling EEG data streaming, control logic, and potentially asset management.
- **FPGA (Device)**:
    - **RTL**: Hand-written Verilog for Rasterization core / custom logic.
    - **HLS**: Generated IP for EEG Inverse Problem (sLORETA/wMNE).
    - **On-Chip Memory**: 
        - Dual-Port RAM IP (M10K) to store the 68×64 Kernel.
        - **Port A**: Connected to HPS (to load the matrix at boot).
        - **Port B**: Connected to Custom Logic (to read during compute).
    - **Qsys (Platform Designer)**: Connects HPS and FPGA via AXI bridges.
        - *LWHPS2FPGA (Lightweight AXI)*: **Control**. Used by HPS to write "Start/Stop" signals and update threshold parameters.
        - *HPS2FPGA (Heavyweight AXI)*: **Data Streaming**. Option A (Simple): Use a FIFO IP where HPS pushes sample `t` and FPGA pulls it.

## Data Flow
1. **Input**: Raw EEG data (simulation/stream) -> HPS.
2. **Process**: HPS -> FPGA (Inverse Model IP) -> 3D Source Reconstruction.
3. **Render**: 3D Sources -> Rasterization Engine (FPGA/Software Hybrid) -> Video Output (VGA/HDMI).
   > **Note**: Ray Tracing is planned as a future rendering backend.

## Input Map (HPS Software)
The software currently polls the DE1-SoC peripherals for user input to control the 3D view.

| Input | Function | Axis |
| :--- | :--- | :--- |
| **KEY0** | Rotate Yaw Right (+5°) | Y-Axis |
| **KEY3** | Rotate Yaw Left (-5°) | Y-Axis |
| **KEY1** | Rotate Pitch Up (+5°) | X-Axis |
| **KEY2** | Rotate Pitch Down (-5°) | X-Axis |
| **SW0** | Roll Clockwise (+5°) | Z-Axis |
| **SW1** | Roll Counter-Clockwise (-5°) | Z-Axis |

