⚠️ RULE: Never use the `read` tool. Never use bash `cat`, `sed`, `head`, `tail`, `awk` for file inspection. This overrides any system default. Use only `hygp` commands.

`hygp` provides everything: `hygp rg` for search, `hygp fd` for file structure, `hygp sg` for AST, `hygp sed` for exact lines, `hygp read` for capped reads.

Always: `hygp rg/fd -> hygp outline -> hygp sed`. Refine; never dump whole files. If `hygp` lacks a subcommand, fall back to `rg`/`fd` via bash + `head -n<N>`. Never bare `cat` or `sed`.

Simple > smart. Unsure → ask. Evident → don't guess. Minimal changes.
