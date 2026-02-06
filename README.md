# De1SoC Ray Tracer & Neuro-RTL

**A Hardware-Accelerated 3D EEG Reconstruction System**

This project combines real-time ray tracing with neuro-imaging algorithms on an FPGA-SoC platform. It aims to provide real-time 3D visualization of EEG data by offloading intensive matrix computations and rendering tasks to the FPGA.

## Overview

### Mission
To implement a Proof of Concept for closed-loop neuromodulation visualization. The system takes raw EEG data, computes source localization (Inverse Problem) using hardware acceleration, and visualizes the activity on a 3D brain model using a custom ray tracing engine.

### Key Features
- **Hybrid Architecture**: Uses the DE1-SoC (ARM Cortex-A9 + Cyclone V FPGA).
- **Hardware Acceleration**:
  - **EEG Inverse Problem**: Generated HLS IP for wMNE/sLORETA source localization.
  - **Ray Tracing**: Experimental RTL implementation for 3D rendering.
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
