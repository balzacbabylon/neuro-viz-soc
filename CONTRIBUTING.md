# Contributing to Neuro-Viz-SoC

Thank you for your interest in contributing to the **Neuro-Viz-SoC** project. This document outlines the standards and workflows for contributing to our hardware-accelerated neuroimaging platform.

## 1. Core Principles
- **Mission First**: Every contribution should advance the goal of real-time, hardware-accelerated EEG visualization.
- **Correctness > Speed**: In medical/scientific contexts, data integrity is paramount.
- **Hybrid Awareness**: Acknowledge the boundary between HPS (Software) and FPGA (Hardware). Changes in one often affect the other.

## 2. Issue & Workflow
All work must be tracked via GitHub Issues.

1.  **Find or Create an Issue**: Check for existing issues. If creating a new one, use the [Issue Templates](.github/ISSUE_TEMPLATE/).
2.  **Assignment**: Assign yourself (or an agent) to the issue to avoid duplication.
3.  **Branching**:
    - Format: `issue-<number>-<short-slug>` (e.g., `issue-42-fix-hps-fifo`).
    - Base branch: `main` (unless targeting a specific release branch).
4.  **Pull Request**:
    - Use the [PR Template](.github/pull_request_template.md).
    - Link the issue (e.g., `Fixes #42`).
    - **Verification IS MANDATORY**. You must document how you verified your changes (simulation waveforms, hardware test output, or software logs).

## 3. Label Taxonomy
We use the standard GitHub labels for categorization.

| Label | Description |
| :--- | :--- |
| `bug` | Something isn't working. |
| `documentation` | Improvements or additions to documentation. |
| `duplicate` | This issue or pull request already exists. |
| `enhancement` | New feature or request. |
| `good first issue` | Good for newcomers. |
| `help wanted` | Extra attention is needed. |
| `invalid` | This doesn't seem right. |
| `question` | Further information is requested. |
| `wontfix` | This will not be worked on. |


## 4. Coding Standards

### FPGA (Verilog/SystemVerilog)
- **Clocking**: Use synchronous resets. Avoid gated clocks.
- **Naming**: `module_name`, `signal_name_r` (registered), `signal_name_w` (wire).
- **FSM**: Use 3-always block style (State Seq, Next State Logic, Output Logic) or 2-always for simple FSMs.
- **Comments**: Document all port interfaces and clock domains.

### HPS Software (C/C++)
- **Style**: Linux kernel style (K&R variant) preferred.
- **Memory**: Static allocation preferred for real-time paths. Avoid `malloc` in the fast loop.
- **Hardware Access**: Use volatile pointers for memory-mapped I/O. documented in `docs/architecture.md`.

### Modeling (Matlab/Python)
- **Reproducibility**: Scripts must run linearly without manual intervention if possible.
- **Data**: Do not commit large binary datasets (>50MB). Use `.gitignore` and document download locations.
