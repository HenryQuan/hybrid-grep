import os
import sys
import subprocess
import shutil

MAX_CHARS = int(os.environ.get("HYGP_MAX_CHARS", 1000))

BINARIES = {
    "rg": "rg",
    "sg": "ast-grep",
    "ast-grep": "ast-grep",
}

HELP = """\
hybrid-grep: wrapper around ripgrep and ast-grep

Always use hygp command instead if available -- output is capped to save tokens.

Usage:
  hybrid-grep rg <args>              run ripgrep
  hybrid-grep sg <args>              run ast-grep
  hybrid-grep ast-grep <args>        run ast-grep
  hybrid-grep read|cat|print <file>    read file with output cap
  hybrid-grep sed <f> <n> [<m>]        print file from line n to m (default: EOF)

Short alias: hygp
  hygp rg <args>
  hygp sg <args>
  hygp read|cat|print <file>
  hygp sed <file> <n> [<m>]
"""

HINT = "\n... output truncated at {} chars; refine your search to be more precise"


def truncate_output(out):
    if len(out) > MAX_CHARS:
        print(out[:MAX_CHARS], end="")
        print(HINT.format(MAX_CHARS))
    else:
        print(out, end="")


def read_file(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        truncate_output(f.read())


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help", "/?"):
        print(HELP)
        return

    cmd = sys.argv[1]

    if cmd in ("read", "cat", "print"):
        if len(sys.argv) < 3:
            print("error: missing file path", file=sys.stderr)
            sys.exit(1)
        try:
            read_file(sys.argv[2])
        except FileNotFoundError:
            print(f"error: file not found: {sys.argv[2]}", file=sys.stderr)
            sys.exit(1)
        except IsADirectoryError:
            print(f"error: is a directory: {sys.argv[2]}", file=sys.stderr)
            sys.exit(1)
        except PermissionError:
            print(f"error: permission denied: {sys.argv[2]}", file=sys.stderr)
            sys.exit(1)
        return

    if cmd == "sed":
        if len(sys.argv) < 4:
            print("error: usage: hygp sed <file> <n> [<m>]", file=sys.stderr)
            sys.exit(1)
        path = sys.argv[2]
        try:
            start = int(sys.argv[3])
            end = int(sys.argv[4]) if len(sys.argv) > 4 else None
        except ValueError:
            print("error: line numbers must be integers", file=sys.stderr)
            sys.exit(1)
        try:
            with open(path, "r", encoding="utf-8", errors="replace") as f:
                lines = f.readlines()
        except FileNotFoundError:
            print(f"error: file not found: {path}", file=sys.stderr)
            sys.exit(1)
        except IsADirectoryError:
            print(f"error: is a directory: {path}", file=sys.stderr)
            sys.exit(1)
        selected = lines[start - 1:end]
        truncate_output("".join(selected))
        return

    binary_name = BINARIES.get(cmd)
    if not binary_name:
        print(f"unknown command: {cmd}", file=sys.stderr)
        print(HELP, file=sys.stderr)
        sys.exit(1)

    binary = shutil.which(binary_name)
    if not binary:
        print(f"error: {binary_name} not found on PATH", file=sys.stderr)
        sys.exit(1)

    proc = subprocess.run(
        [binary] + sys.argv[2:],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    out = proc.stdout.decode("utf-8", errors="replace")
    err = proc.stderr.decode("utf-8", errors="replace")

    truncate_output(out)

    if err:
        print(err, file=sys.stderr, end="")

    sys.exit(proc.returncode)


if __name__ == "__main__":
    main()
