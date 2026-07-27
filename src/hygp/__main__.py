import os, sys, subprocess, shutil

MAX_CHARS = int(os.environ.get("HYGP_MAX_CHARS") or 1000)
BIN = {"rg": "rg", "sg": "ast-grep", "ast-grep": "ast-grep"}
READ = {"read", "cat", "print"}

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


def cap(s):
    if len(s) > MAX_CHARS:
        s = s[:MAX_CHARS] + HINT.format(MAX_CHARS)
    sys.stdout.write(s)


def open_err(path):
    try:
        return open(path, encoding="utf-8", errors="replace")
    except (FileNotFoundError, IsADirectoryError, PermissionError) as e:
        sys.exit(f"error: {e}")


def check_tools():
    for name in ("rg", "ast-grep"):
        found = shutil.which(name)
        print(f"  {name}: {'found' if found else 'not found'}")

def main():
    a = sys.argv
    if len(a) < 2 or a[1] in ("-h", "--help", "/?"):
        print(HELP)
        print("Available tools:")
        check_tools()
        return

    cmd = a[1]

    if cmd in READ:
        if len(a) < 3:
            sys.exit("error: missing file path")
        with open_err(a[2]) as f:
            cap(f.read())

    elif cmd == "sed":
        if len(a) < 4:
            sys.exit("error: usage: hygp sed <file> <n> [<m>]")
        try:
            start = int(a[3])
            end = int(a[4]) if len(a) > 4 else None
        except ValueError:
            sys.exit("error: line numbers must be integers")
        if start < 1:
            sys.exit("error: start line must be >= 1")
        with open_err(a[2]) as f:
            cap("".join(f.readlines()[start - 1:end]))

    else:
        name = BIN.get(cmd)
        if not name:
            sys.exit(f"unknown command: {cmd}\n{HELP}")
        bin_path = shutil.which(name)
        if not bin_path:
            sys.exit(f"error: {name} not found on PATH")
        proc = subprocess.run([bin_path] + a[2:], capture_output=True)
        cap(proc.stdout.decode("utf-8", errors="replace"))
        if proc.stderr:
            print(proc.stderr.decode("utf-8", errors="replace"), file=sys.stderr, end="")
        sys.exit(proc.returncode)


if __name__ == "__main__":
    main()
