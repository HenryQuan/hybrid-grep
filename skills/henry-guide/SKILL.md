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

## 2. Inspect with `hygp`

Check `hygp` first. When available, use it instead of direct covered commands; output is capped.

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

Default: `rg/fd -> outline -> sed`. Refine search; never dump whole files. If unavailable, use bounded normal tools.

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
