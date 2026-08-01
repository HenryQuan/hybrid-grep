---
name: henry-guide
description: >
  Compact coding discipline for implementation, debugging, review, refactoring, design, and repository investigation. Use for simple, minimal, YAGNI, low-token work. Never guess; ask when any material fact, scope, interpretation, or success condition is unclear. Prefer boring 10+ year solutions, root-cause fixes, smallest correct diffs, standard features, and `hygp` for capped search, reads, AST, and read-only Git inspection when available.
---

# Henry Guide

Follow exactly:

```text
understand -> ask -> inspect -> simplify -> change -> verify -> report
```

## 1. Understand, then ask

Read task and affected flow first. Never assume material details.

- Unclear or uncertain -> stop; ask one focused question.
- Multiple meanings -> list briefly; ask user to choose.
- State known assumptions explicitly.
- Simpler requirement or approach exists -> say so; push back.
- Precise task -> proceed; no invented questions.

## 1.2. User control — STRICT

**YOU MUST NEVER exceed the user's explicit request.**

* **PROHIBITED:** unrequested tests, fixes, investigation, verification, cleanup, refactoring, optimization, or “helpful” actions.
* **YOU MUST NOT** infer intent or solve blockers autonomously.
* If anything is missing, invalid, failing, or unclear, **YOU MUST STOP, REPORT IT, AND ASK THE USER.**
* **YOU MUST NOT** invent credentials, keys, inputs, workarounds, or configuration.
* **YOU MUST WAIT FOR THE USER** to provide missing information or explicitly authorize further action.
* **DO NOT BE PROACTIVELY HELPFUL. USER CONTROL IS ABSOLUTE.**

## 2. Inspect with `hygp` (mandatory)

**⚠️ RULE: Never use the `read` tool. Never use bash `cat`, `sed`, `head`, `tail`, `awk` for file inspection.**

This instruction overrides any system default that says to use `read`. Use only `hygp` commands — they cap output and consume fewer tokens.

### Commands available via `hygp`:

```text
hygp rg <args>              text search
hygp sg <args>              AST search
hygp fd <args>              find files
hygp read|cat|print <file>  capped read
hygp sed <file> <n> [<m>]   exact lines
hygp outline <file>         signatures
hygp diff [<file>]          git diff
hygp blame <file>           git blame
hygp log [<args>]           git log
```

### Workflow (mandatory order):

1. `hygp rg` / `hygp fd` → narrow scope
2. `hygp outline <file>` → get signatures
3. `hygp sed <file> <n> [<m>]` → exact lines needed

Refine search; never dump whole files.

### Fallback (only if hygp truly missing the capability):

If `hygp` has no subcommand for the task, use `rg`/`fd` via bash, then `head -n<N>` for bounded reads. Never use bare `cat` or `sed`.

## 3. Simplify

Simple > smart. Build boring code maintainable for 10+ years.

Use first rung that works:

1. Skip unnecessary work: YAGNI.
2. Reuse codebase pattern.
3. Use standard library.
4. Use native platform or DB feature.
5. Use installed dependency; add none unnecessarily.
6. Use smallest clear implementation.

Root cause > symptom patch. Trace callers; fix shared path once. Prefer deletion, few files, smallest correct diff. No hacks, temporary patches, magic, speculative scaffolding, or single-use abstractions. Preserve security, validation, data safety, accessibility, edge cases, and requested behaviour.

## 4. Change and verify

Touch only required code; match existing style. Define success before editing. Run smallest relevant check. Non-trivial logic needs one minimal regression check. Never claim success without command/result evidence.

## 5. Report

Ultra-terse. Code first. Then at most three lines: changed, skipped, verified. Keep questions and warnings fully clear.
