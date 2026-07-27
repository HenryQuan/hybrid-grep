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
    "\n... output truncated at %d chars; try ast-grep for AST-based search\n";
static const char *HINT_SG =
    "\n... output truncated at %d chars; try rg for regex search\n";
static const char *HINT_FILE =
    "\n... output truncated at %d chars; try rg or ast-grep outline\n";

static int is_read(const char *s) {
    return !strcmp(s, "read") || !strcmp(s, "cat") || !strcmp(s, "print");
}

static void cap(const char *s, int truncated, const char *hint) {
    size_t len = strlen(s);
    if (truncated || len > MAX_CHARS) {
        size_t n = len < MAX_CHARS ? len : MAX_CHARS;
        fwrite(s, 1, n, stdout);
        printf(hint, MAX_CHARS);
    } else {
        fputs(s, stdout);
    }
}

static void run_cmd(char **args, int n, const char *hint) {
    char buf[4096];
    int pos = 0;
    for (int i = 0; i < n; i++) {
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
    size_t total = 0;
    int ch;
    while ((ch = fgetc(p)) != EOF && total < sizeof(out) - 1)
        out[total++] = (char)ch;
    out[total] = '\0';
    cap(out, total > MAX_CHARS, hint);
    int rc = pclose(p);
    if (rc) exit(rc);
}

static void read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "error: cannot open %s\n", path); exit(1); }
    char line[4096];
    size_t content = 0;
    int truncated = 0, lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        size_t len = strlen(line);
        if (content + len > MAX_CHARS) {
            if (content < MAX_CHARS) {
                size_t rem = MAX_CHARS - content;
                printf("%d: ", lineno);
                fwrite(line, 1, rem, stdout);
            }
            truncated = 1;
            while (fgets(line, sizeof(line), f)) lineno++;
            break;
        }
        printf("%d: %s", lineno, line);
        content += len;
    }
    fclose(f);
    if (truncated) printf(HINT_FILE, MAX_CHARS);
}

static void read_lines(const char *path, int start, int end) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "error: cannot open %s\n", path); exit(1); }
    char line[4096];
    size_t content = 0;
    int truncated = 0, lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        if (lineno < start) continue;
        if (end > 0 && lineno > end) break;
        size_t len = strlen(line);
        if (content + len > MAX_CHARS) {
            if (content < MAX_CHARS) {
                size_t rem = MAX_CHARS - content;
                printf("%d: ", lineno);
                fwrite(line, 1, rem, stdout);
            }
            truncated = 1;
            while (fgets(line, sizeof(line), f)) lineno++;
            break;
        }
        printf("%d: %s", lineno, line);
        content += len;
    }
    fclose(f);
    if (truncated) printf(HINT_FILE, MAX_CHARS);
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
