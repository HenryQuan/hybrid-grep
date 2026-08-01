# trim

Capped-output wrapper around any command — designed for coding agents, not humans.

## Why

Useful for bug fixing and code search, but intentionally limits information to save tokens and avoid context degradation during long agent runs. A human developer doesn't read an entire file before fixing a bug, coding agents shouldn't either.

`trim` caps output at 1000 characters per call, forcing agents to search precisely instead of dumping entire files. This saves tokens, reduces noise, and keeps the context window focused over long sessions.

If you need unrestricted browsing, run commands directly. This tool is built for coding agents.

## Usage

```
trim rg <args>              run ripgrep
trim sg <args>              run ast-grep
trim fd <args>              run fd
trim outline <file>         extract function/class signatures (ast-grep)
trim diff [<file>]          git diff (read-only)
trim blame <file>           git blame (read-only)
trim log [<args>]           git log (read-only)
trim read|cat|print <file>  read file with output cap
trim sed <file> <n> [<m>]   print file from line n to m
trim <command> [args]       run ANY command, output capped
```

`trim` is a generic wrapper: any command not listed above is run as-is with capped output. Shorthands are convenience mappings — `trim diff` becomes `git diff`, `trim sg` becomes `ast-grep`, and so on. `trm` is a shorter alias for `trim`.

```
TRIM_MAX_CHARS=500 trim rg pattern
```

## Install

### Binary (C, ~25 KB)

Download from [releases](https://github.com/henryquan/trim/releases) or build:

```sh
gcc -O2 -s -o trim trim.c       # Linux/macOS
gcc -O2 -s -o trim.exe trim.c   # Windows (MinGW)
make trim                        # or use the Makefile
```

## PI

Pi agent extensions that enforce the same discipline at the tool level:

| File | Purpose |
|------|---------|
| `enforce-trm.ts` | Removes `read`/`grep`/`find`/`ls` from pi toolset — model can't call them |
| `bash-cap.ts` | Caps ALL bash output at `TRIM_MAX_CHARS` (default 1000), same format as `trim.c` |
| `APPEND_SYSTEM.md` | Appends trim rules to system prompt as a text-level reminder |

Without these, the model may still use built-in tools like `read` or run `cat`/`rg`/`grep` directly, bypassing `trim`'s capping. The extensions remove the tools entirely and cap all bash output regardless of command.

## License

MIT
