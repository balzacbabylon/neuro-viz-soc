# AGENTS.md

## Mission
You are working in this repo to: implement a Hardware Accelerated 3D reconstruction of EEG data on a DE1-SoC board, merging real-time ray tracing capability with neuro-imaging algorithms.
Optimize for: correctness > maintainability > speed.

## Non-negotiables (guardrails)
- Do not change public APIs without updating docs and tests.
- Do not add new dependencies unless asked (or justify clearly).
- Prefer small diffs; avoid broad refactors unless requested.
- If uncertain, write a plan in `docs/research_requests.md` or `implementation_plan.md` and ask for confirmation.
- **Documentation**: All major architectural changes must be reflected in `docs/architecture.md`.

## How to run the project (commands you must use)
- **Software (HPS) Build**:
  ```bash
  cd software
  make
  ```
- **Hardware (FPGA)**:
  - Quartus project located in `hardware/quartus/`. Open `.qpf` with Quartus Prime 18.1.
  - Qsys system located in `hardware/qsys/`.
- **Modeling**:
  - Matlab scripts in `modeling/matlab/`. Run in Matlab environment.

## Repo map (where things live)
- `hardware/` = FPGA Logic & Synthesis (Quartus, Qsys, RTL, HLS, IP)
- `software/` = HPS C/C++ Application (Source, Headers, Tools, Generated)
- `modeling/` = Research & Data Science (Matlab, Python prototypes, Data)
- `assets/` = 3D Assets (OBJ files, textures)
- `docs/` = Subsystem docs (must read before changes)
- `scripts/` = Global build/automation scripts

## When to read docs (routing rules)
If the task mentions **Architecture / Component Relationships**, you MUST read:
- `docs/architecture.md`
If the task mentions **Workflows / Procedures**, you MUST read:
- `docs/workflows.md`
If the task mentions **Constraints / Rules**, you MUST read:
- `docs/rules.md`
If the task mentions **APIs / Interfaces**, you MUST read:
- `docs/api.md`

## Output format (how you should work)
1) Write a short plan (bullets).
2) Execute step-by-step; run relevant commands.
3) Summarize changes + include how to verify (commands).
4) Update docs in `docs/` if behavior/architecture changed.
