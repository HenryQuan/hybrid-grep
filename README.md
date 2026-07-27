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
hygp read|cat|print <file>  read file with line numbers (cap excludes line numbers)
hygp sed <file> <n> [<m>]   print file from line n to m
```

Output exceeding 1000 chars is truncated with a context-aware hint:
- `rg` → suggests `ast-grep` for AST-based search
- `ast-grep` → suggests `rg` for regex search
- `read`/`sed` → suggests `rg` or `ast-grep outline`

Override with `HYGP_MAX_CHARS` env var:

```
HYGP_MAX_CHARS=500 hygp rg pattern
```

## Install

### Binary (C, ~25 KB)

Download from [releases](https://github.com/anomalyco/hybrid-grep/releases) or build:

```sh
gcc -O2 -s -o hygp hygp.c       # Linux/macOS
gcc -O2 -s -o hygp.exe hygp.c   # Windows (MinGW)
make hygp                        # or use the Makefile
```

### Python (via uv)

```sh
uv run python -m hygp rg foo
uv tool install .
hybrid-grep rg foo
```

Python 3.8+ required.

## License

MIT
