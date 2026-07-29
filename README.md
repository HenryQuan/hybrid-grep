# hybrid-grep

Capped-output wrappers for `rg` (ripgrep), `ast-grep`, and file reading — designed for coding agents, not humans.

## Why

Useful for bug fixing and code search, but intentionally limits information to save tokens and avoid context degradation during long agent runs. A human developer doesn't read an entire file before fixing a bug, coding agents shouldn't either.

`hybrid-grep` caps output at 1000 characters per call, forcing agents to search precisely instead of dumping entire files. This saves tokens, reduces noise, and keeps the context window focused over long sessions.

If you need unrestricted browsing, use `rg`, `ast-grep`, or your editor directly. This tool is built for coding agents.

## Usage

```
hygp rg <args>              run ripgrep
hygp sg <args>              run ast-grep
hygp fd <args>              run fd
hygp outline <file>         extract function/class signatures (ast-grep)
hygp read|cat|print <file>  read file with line numbers
hygp sed <file> <n> [<m>]   print file from line n to m
hygp diff [<file>]          git diff (read-only)
hygp blame <file>           git blame (read-only)
hygp log [<args>]           git log (read-only)
```

Output exceeding 1000 chars is truncated with a context-aware hint.
Override with `HYGP_MAX_CHARS` env var:

```
HYGP_MAX_CHARS=500 hygp rg pattern
```

## Install

### Binary (C, ~25 KB)

Download from [releases](https://github.com/henryquan/hybrid-grep/releases) or build:

```sh
gcc -O2 -s -o hygp hygp.c       # Linux/macOS
gcc -O2 -s -o hygp.exe hygp.c   # Windows (MinGW)
make hygp                        # or use the Makefile
```

## PI

Pi agent extensions that enforce the same discipline at the tool level:

| File | Purpose |
|------|---------|
| `enforce-hygp.ts` | Removes `read`/`grep`/`find`/`ls` from pi toolset — model can't call them |
| `bash-cap.ts` | Caps ALL bash output at `HYGP_MAX_CHARS` (default 1000), same format as `hygp.c` |
| `APPEND_SYSTEM.md` | Appends hygp rules to system prompt as a text-level reminder |

Without these, the model may still use built-in tools like `read` or run `cat`/`rg`/`grep` directly, bypassing `hygp`'s capping. The extensions remove the tools entirely and cap all bash output regardless of command.

## License

MIT
