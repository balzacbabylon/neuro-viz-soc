# AGENTS.md

## Mission
You are working in this repo to: implement a Hardware Accelerated 3D reconstruction of EEG data on a DE1-SoC board, merging real-time raster-based rendering capability with neuro-imaging algorithms.
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

## GitHub Workflow (Issues / PRs)

### Source of truth
- All work MUST be tied to a GitHub issue (or a newly created issue).
- Do not implement features from vague prompts alone; if requirements are unclear, comment on the issue with questions and a proposed plan.

### Before coding
1. Read the linked issue and restate the Acceptance Criteria.
2. Identify relevant docs in `docs/` and read them first (especially subsystem docs).
3. Propose a short plan (3–7 steps). If the task is non-trivial or risky, write the plan into `docs/plan.md` and proceed step-by-step.

### Scope control
- Keep changes minimal and targeted. Avoid broad refactors unless explicitly requested.
- Prefer multiple small PRs over one large PR.
- If you discover additional work, open a follow-up issue rather than expanding scope.

### Branch + PR requirements
- Create a branch named: `issue-<number>-<short-slug>`.
- Open a PR using the PR template and link the issue using: `Fixes #<number>` or `Refs #<number>`.
- The PR description MUST include:
  - Summary of changes
  - Verification commands actually run (or explain why not possible)
  - Any docs updated and why

### Definition of Done
A task is complete only when:
- Acceptance Criteria are satisfied
- Tests are added/updated where appropriate
- CI passes (or failures are explained and unrelated)
- Docs are updated if behavior, APIs, or workflows changed

### If blocked / uncertain
- Do not guess. Add a comment to the issue describing:
  - what you tried
  - what you observed
  - what information is missing
  - 1–2 options to proceed
