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

Usage:
  hybrid-grep rg <args>          run ripgrep
  hybrid-grep sg <args>          run ast-grep
  hybrid-grep ast-grep <args>    run ast-grep

Short alias: hygp
  hygp rg <args>
  hygp sg <args>
"""

HINT = "\n... output truncated at {} chars; refine your search to be more precise"


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help", "/?"):
        print(HELP)
        return

    cmd = sys.argv[1]
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

    if len(out) > MAX_CHARS:
        print(out[:MAX_CHARS], end="")
        print(HINT.format(MAX_CHARS))
    else:
        print(out, end="")

    if err:
        print(err, file=sys.stderr, end="")

    sys.exit(proc.returncode)


if __name__ == "__main__":
    main()
