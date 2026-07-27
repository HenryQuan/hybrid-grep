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
#endif

static int MAX_CHARS = 1000;

static void init_max_chars(void) {
    char *env = getenv("HYGP_MAX_CHARS");
    if (env) { int v = atoi(env); if (v > 0) MAX_CHARS = v; }
}

static const char *HELP =
    "hybrid-grep: wrapper around ripgrep and ast-grep\n"
    "\n"
    "Always use hygp command instead if available -- output is capped to save tokens.\n"
    "\n"
    "Usage:\n"
    "  hygp rg <args>              run ripgrep\n"
    "  hygp sg <args>              run ast-grep\n"
    "  hygp read|cat|print <file>  read file with output cap\n"
    "  hygp sed <file> <n> [<m>]   print file from line n to m\n"
    "\n";

static const char *HINT_RG =
    "\n--- TRUNCATED (%d/%d chars) ---\nTip: try 'hygp sg' or 'hygp sg outline'\n";
static const char *HINT_SG =
    "\n--- TRUNCATED (%d/%d chars) ---\nTip: try 'hygp rg' for regex search\n";
static const char *HINT_FILE =
    "\n--- TRUNCATED (%d/%d chars) ---\nTip: try 'hygp sed <file> <line> <line>'\n";
static const char *HINT_SED =
    "\n--- TRUNCATED (%d/%d chars) ---\nTip: try 'hygp rg' or 'hygp sg'\n";

static int is_read(const char *s) {
    return !strcmp(s, "read") || !strcmp(s, "cat") || !strcmp(s, "print");
}

static void cap(const char *s, int truncated, const char *hint, int total_chars) {
    size_t len = strlen(s);
    if (truncated || len > MAX_CHARS) {
        size_t n = len < MAX_CHARS ? len : MAX_CHARS;
        fwrite(s, 1, n, stdout);
        printf(hint, MAX_CHARS, total_chars);
    } else {
        fputs(s, stdout);
    }
}

static void run_cmd(char **args, int argc, const char *hint) {
    char buf[4096];
    int pos = 0;
    for (int i = 0; i < argc; i++) {
        int has_space = !!strchr(args[i], ' ');
        if (i) buf[pos++] = ' ';
        if (has_space) buf[pos++] = '"';
        int len = strlen(args[i]);
        memcpy(buf + pos, args[i], len);
        pos += len;
        if (has_space) buf[pos++] = '"';
        buf[pos] = '\0';
    }

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
    if (rc) exit(rc);
}

static void read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "error: cannot open %s\n", path); exit(1); }
    char line[4096];
    int printed = 0, total = 0, truncated = 0, lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        int len = strlen(line);
        total += len;
        if (printed + len > MAX_CHARS) {
            if (printed < MAX_CHARS) {
                int rem = MAX_CHARS - printed;
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
    int printed = 0, total = 0, truncated = 0, lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        if (lineno < start) continue;
        if (end > 0 && lineno > end) break;
        int len = strlen(line);
        total += len;
        if (printed + len > MAX_CHARS) {
            if (printed < MAX_CHARS) {
                int rem = MAX_CHARS - printed;
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
        printf("Available tools:\n  rg: %s\n  ast-grep: %s\n",
               rg_found ? "found" : "not found",
               sg_found ? "found" : "not found");
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

    const char *hint = NULL;
    if (!strcmp(cmd, "rg")) {
        hint = HINT_RG;
        argv[1] = "rg";
    } else if (!strcmp(cmd, "sg") || !strcmp(cmd, "ast-grep")) {
        hint = HINT_SG;
        argv[1] = "ast-grep";
    } else {
        fprintf(stderr, "unknown command: %s\n%s", cmd, HELP);
        return 1;
    }

    run_cmd(argv + 1, argc - 1, hint);
    return 0;
}
