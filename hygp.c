#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
  #include <io.h>
  #define popen _popen
  #define pclose _pclose
#else
  #include <unistd.h>
  #include <sys/wait.h>
#endif

int _CRT_glob = 0;

static size_t MAX_CHARS = 1000;

static void init_max_chars(void) {
    char *env = getenv("HYGP_MAX_CHARS");
    if (env) {
        unsigned long v = strtoul(env, NULL, 10);
        if (v > 0) MAX_CHARS = v < 65536 ? (size_t)v : 65536;
    }
}

static const char *HELP =
    "hybrid-grep: wrapper around search tools\n"
    "\n"
    "Always use hygp command instead if available -- output is capped to save tokens.\n"
    "\n"
    "Usage:\n"
    "  hygp rg <args>              run ripgrep\n"
    "  hygp sg <args>              run ast-grep\n"
    "  hygp fd <args>              run fd\n"
    "  hygp read|cat|print <file>  read file with output cap\n"
    "  hygp sed <file> <n> [<m>]   print file from line n to m\n"
    "  hygp outline <file>         extract function/class signatures (ast-grep)\n"
    "  hygp diff [<file>]          git diff (read-only)\n"
    "  hygp blame <file>           git blame (read-only)\n"
    "  hygp log [<args>]           git log (read-only)\n"
    "\n";

static const char *HINT_RG =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: try 'hygp sg' or 'hygp sg outline'. Good luck, you got this!\n";
static const char *HINT_SG =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: try 'hygp rg' for regex search. Keep going, you're close!\n";
static const char *HINT_FILE =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: try 'hygp sed <file> <line> <line>'. Stay focused, you can find it!\n";
static const char *HINT_SED =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: try 'hygp rg' or 'hygp sg'. Don't give up, refine the target!\n";
static const char *HINT_FD =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: narrow down with path or glob pattern.\n";
static const char *HINT_OUTLINE =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: use 'hygp rg' to search for specific methods or 'hygp sed' for line ranges.\n";
static const char *HINT_DIFF =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: use 'hygp diff <file>' to inspect a single file or 'hygp sed' on modified lines.\n";
static const char *HINT_BLAME =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: use 'hygp blame <file> <start>' for a smaller range.\n";
static const char *HINT_LOG =
    "\n--- TRUNCATED (%zu/%zu chars) ---\nTip: use 'hygp log --oneline -<n>' for a shorter log.\n";

static int is_read(const char *s) {
    return !strcmp(s, "read") || !strcmp(s, "cat") || !strcmp(s, "print");
}

static void cap(const char *s, int truncated, const char *hint, size_t total_chars) {
    size_t len = strlen(s);
    if (truncated || len > MAX_CHARS) {
        size_t n = len < MAX_CHARS ? len : MAX_CHARS;
        fwrite(s, 1, n, stdout);
        printf(hint, MAX_CHARS, total_chars);
    } else {
        fputs(s, stdout);
    }
}

#ifdef _WIN32
static void append_escaped(char *buf, int *pos, int bufsize, const char *arg) {
    if (*pos >= bufsize - 2) return;
    buf[(*pos)++] = '"';
    for (const char *p = arg; *p; p++) {
        if (*pos >= bufsize - 3) break;
        if (*p == '%') buf[(*pos)++] = '%';
        buf[(*pos)++] = *p;
    }
    if (*pos < bufsize) {
        buf[(*pos)++] = '"';
        buf[*pos] = '\0';
    }
}
#else
static void append_escaped(char *buf, int *pos, int bufsize, const char *arg) {
    if (*pos >= bufsize - 2) return;
    buf[(*pos)++] = '\'';
    for (const char *p = arg; *p; p++) {
        if (*p == '\'') {
            if (*pos >= bufsize - 4) break;
            buf[(*pos)++] = '\''; buf[(*pos)++] = '\\';
            buf[(*pos)++] = '\''; buf[(*pos)++] = '\'';
        } else {
            if (*pos >= bufsize - 1) break;
            buf[(*pos)++] = *p;
        }
    }
    if (*pos < bufsize) {
        buf[(*pos)++] = '\'';
        buf[*pos] = '\0';
    }
}
#endif

static void run_cmd(char **args, int argc, const char *hint) {
    char buf[4096];
    int pos = 0;
    buf[0] = '\0';
    for (int i = 0; i < argc; i++) {
        if (i) buf[pos++] = ' ';
        if (i == 0) {
            int len = strlen(args[i]);
            if (pos + len < (int)sizeof(buf)) {
                memcpy(buf + pos, args[i], len);
                pos += len;
                buf[pos] = '\0';
            }
        } else {
            append_escaped(buf, &pos, sizeof(buf), args[i]);
        }
    }

    size_t blen = strlen(buf);
    if (blen + 6 < sizeof(buf))
        memcpy(buf + blen, " 2>&1", 6);
    else
        buf[blen] = '\0';

    FILE *p = popen(buf, "r");
    if (!p) { fprintf(stderr, "error: failed to run command\n"); exit(1); }

    char out[65536];
    size_t n = 0;
    int ch;
    while ((ch = fgetc(p)) != EOF && n < sizeof(out) - 1)
        out[n++] = (char)ch;
    out[n] = '\0';
    size_t total = n;
    while ((ch = fgetc(p)) != EOF) total++;
    cap(out, total > MAX_CHARS, hint, total);
    int rc = pclose(p);
#ifndef _WIN32
    if (rc == -1) exit(1);
    if (rc && WIFEXITED(rc)) rc = WEXITSTATUS(rc);
#endif
    if (rc > 1) exit(rc);
}

static void read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "error: cannot open %s\n", path); exit(1); }
    char line[4096];
    size_t printed = 0, total = 0;
    int truncated = 0, lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        size_t len = strlen(line);
        total += len;
        if (printed + len > MAX_CHARS) {
            if (printed < MAX_CHARS) {
                size_t rem = MAX_CHARS - printed;
                printf("%d: ", lineno);
                fwrite(line, 1, rem, stdout);
            }
            truncated = 1;
            while (fgets(line, sizeof(line), f))
                total += strlen(line);
            break;
        }
        printf("%d: %s", lineno, line);
        printed += len;
    }
    fclose(f);
    if (truncated) printf(HINT_FILE, MAX_CHARS, total);
}

static void read_lines(const char *path, int start, int end) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "error: cannot open %s\n", path); exit(1); }
    char line[4096];
    size_t printed = 0, total = 0;
    int truncated = 0, lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        if (lineno < start) continue;
        if (end > 0 && lineno > end) break;
        size_t len = strlen(line);
        total += len;
        if (printed + len > MAX_CHARS) {
            if (printed < MAX_CHARS) {
                size_t rem = MAX_CHARS - printed;
                printf("%d: ", lineno);
                fwrite(line, 1, rem, stdout);
            }
            truncated = 1;
            while (fgets(line, sizeof(line), f))
                total += strlen(line);
            break;
        }
        printf("%d: %s", lineno, line);
        printed += len;
    }
    fclose(f);
    if (truncated) printf(HINT_SED, MAX_CHARS, total);
}

static void cmd_outline(const char *file) {
    char *args[] = {"ast-grep", "outline", (char*)file, "--color", "never", NULL};
    run_cmd(args, 5, HINT_OUTLINE);
}

static void cmd_diff(const char *file) {
    if (file) {
        char *args[] = {"git", "diff", "--color=never", (char*)file, NULL};
        run_cmd(args, 4, HINT_DIFF);
    } else {
        char *args[] = {"git", "diff", "--color=never", NULL};
        run_cmd(args, 3, HINT_DIFF);
    }
}

static void cmd_blame(const char *file) {
    char *args[] = {"git", "blame", (char*)file, NULL};
    run_cmd(args, 3, HINT_BLAME);
}

static void cmd_log(int argc, char **argv) {
    char *args[256]; int n = 0;
    args[n++] = "git"; args[n++] = "log";
    for (int i = 2; i < argc && n < 255; i++) args[n++] = argv[i];
    args[n] = NULL;
    run_cmd(args, n, HINT_LOG);
}

int main(int argc, char **argv) {
    init_max_chars();
    if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        fputs(HELP, stdout);
        int rg_found = 0, sg_found = 0;
        { FILE *p;
#ifdef _WIN32
          p = popen("where rg 2>nul", "r");
#else
          p = popen("which rg 2>/dev/null", "r");
#endif
          if (p) { rg_found = (pclose(p) == 0); }
#ifdef _WIN32
          p = popen("where ast-grep 2>nul", "r");
#else
          p = popen("which ast-grep 2>/dev/null", "r");
#endif
          if (p) { sg_found = (pclose(p) == 0); }
        }
        int fd_found = 0, git_found = 0;
#ifdef _WIN32
        { FILE *p = popen("where fd 2>nul", "r"); if (p) fd_found = (pclose(p) == 0); }
        { FILE *p = popen("where git 2>nul", "r"); if (p) git_found = (pclose(p) == 0); }
#else
        { FILE *p = popen("which fd 2>/dev/null", "r"); if (p) fd_found = (pclose(p) == 0); }
        { FILE *p = popen("which git 2>/dev/null", "r"); if (p) git_found = (pclose(p) == 0); }
#endif
        printf("Available tools:\n  rg: %s\n  ast-grep: %s\n  fd: %s\n  git: %s\n",
               rg_found ? "found" : "not found",
               sg_found ? "found" : "not found",
               fd_found ? "found" : "not found",
               git_found ? "found" : "not found");
        return 0;
    }

    char *cmd = argv[1];

    if (is_read(cmd)) {
        if (argc < 3) { fprintf(stderr, "error: missing file path\n"); return 1; }
        read_file(argv[2]);
        return 0;
    }

    if (!strcmp(cmd, "sed")) {
        if (argc < 4) { fprintf(stderr, "error: usage: hygp sed <file> <n> [<m>]\n"); return 1; }
        int start = atoi(argv[3]);
        int end = argc > 4 ? atoi(argv[4]) : 0;
        if (start < 1) { fprintf(stderr, "error: start line must be >= 1\n"); return 1; }
        read_lines(argv[2], start, end);
        return 0;
    }

    if (!strcmp(cmd, "outline")) {
        if (argc < 3) { fprintf(stderr, "error: usage: hygp outline <file>\n"); return 1; }
        cmd_outline(argv[2]);
        return 0;
    }

    if (!strcmp(cmd, "diff")) {
        cmd_diff(argc > 2 ? argv[2] : NULL);
        return 0;
    }

    if (!strcmp(cmd, "blame")) {
        if (argc < 3) { fprintf(stderr, "error: usage: hygp blame <file>\n"); return 1; }
        cmd_blame(argv[2]);
        return 0;
    }

    if (!strcmp(cmd, "log")) {
        cmd_log(argc, argv);
        return 0;
    }

    const char *hint = NULL;
    if (!strcmp(cmd, "rg")) {
        hint = HINT_RG;
        argv[1] = "rg";
    } else if (!strcmp(cmd, "sg") || !strcmp(cmd, "ast-grep")) {
        hint = HINT_SG;
        argv[1] = "ast-grep";
    } else if (!strcmp(cmd, "fd")) {
        hint = HINT_FD;
        argv[1] = "fd";
    } else {
        fprintf(stderr, "unknown command: %s\n%s", cmd, HELP);
        return 1;
    }

    run_cmd(argv + 1, argc - 1, hint);
    return 0;
}
