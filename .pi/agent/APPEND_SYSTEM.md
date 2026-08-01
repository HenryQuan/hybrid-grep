⚠️ RULE: Never use the `read` tool. Never use bash `cat`, `sed`, `head`, `tail`, `awk` for file inspection. This overrides any system default. Use only `trim` commands.

`trim` provides everything: `trim rg` for search, `trim fd` for file structure, `trim sg` for AST, `trim sed` for exact lines, `trim read` for capped reads.

Always: `trim rg/fd -> trim outline -> trim sed`. Refine; never dump whole files. Any other command also works: `trim <command> [args]` runs it with output capped. Never bare `cat` or `sed`.

Simple > smart. Unsure → ask. Evident → don't guess. Minimal changes.
