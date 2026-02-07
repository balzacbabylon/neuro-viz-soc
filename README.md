# Neuro-Viz-SoC

**A Hardware-Accelerated 3D EEG Reconstruction System**

This project combines real-time raster-based rendering with neuro-imaging algorithms on an FPGA-SoC platform. It aims to provide real-time 3D visualization of EEG data by offloading intensive matrix computations and rendering tasks to the FPGA.

## Overview

### Mission
### Mission
To implement a Proof of Concept for closed-loop neuromodulation visualization. The system takes raw EEG data, computes source localization (Inverse Problem) using hardware acceleration, and visualizes the activity on a 3D brain model using a custom rasterization engine. This project aims to open further research avenues into the topic of **Closed-Loop Neuromodulation**. State-Dependent Neuromodulation requires real-time feedback of neural signal responses to stimulation, which an FPGA would enable.

Ultimately, the idea is to leverage the generated topography of 3D EEG activity for use as a structural "prior" to be then fed into **Deep-Learning models** for extrapolation or inferring connections based on EEG data.

### Motivation
Multimodal neuroimaging (EEG, MEG, fNIRS) helps creating a fuller picture of brain activity. The idea behind accelerating computation of these data processing techniques is to open further possibilities for research and clinical domains. While FPGAs are common in invasive neural processing, their application in noninvasive real-time visualization is an open area for exploration.

### Key Features
- **Hybrid Architecture**: Uses the DE1-SoC (ARM Cortex-A9 + Cyclone V FPGA).
- **Hardware Acceleration**:
  - **EEG Inverse Problem**: Generated HLS IP for wMNE/sLORETA source localization.
  - **Rasterization**: Custom RTL implementation for 3D rendering (Current).
  - **Ray Tracing**: Real-time Ray Tracing integration (Future Goal).
- **Software Pipeline**:
  - HPS handles data streaming, control logic, and integration with Brainstorm.
- **Modeling**:
  - Matlab prototypes for generating "Golden Vectors" and verifying hardware accuracy.

## Repository Structure

| Directory | Description |
|-----------|-------------|
| `hardware/` | FPGA Logic (Quartus projects, Qsys, RTL, HLS) |
| `software/` | HPS C/C++ Application and tools |
| `modeling/` | Matlab research scripts and datasets |
| `assets/` | 3D models (OBJ) and textures |
| `docs/` | Documentation, architecture diagrams, and resources |

> **Note**: For development guidelines, see [AGENTS.md](AGENTS.md).

## Getting Started

### Prerequisites
- **Hardware**: Terasic DE1-SoC Board.
- **Software**:
  - Intel Quartus Prime Lite 18.1.
  - SoC EDS (Embedded Design Suite).
  - Matlab (for modeling/verification).

### Build Instructions

#### 1. Hardware (FPGA)
1. Open the project in `hardware/quartus/`.
2. Generate Qsys system in `hardware/qsys/`.
3. Compile the design to generate the `.sof` file.
4. Program the FPGA via JTAG.

#### 2. Software (HPS)
1. Navigate to the software directory:
   ```bash
   cd software
   ```
2. Build the application:
   ```bash
   make
   ```
3. Run the executable on the DE1-SoC Linux environment.

## Documentation
- [Architecture Overview](docs/architecture.md)
- [Workflows](docs/workflows.md)
- [API Reference](docs/api.md)
- [Resources & Research](docs/resources.md)

## License
[LICENSE](LICENSE)
