// ═══════════════════════════════════════════════════════════════
// main.c — BMA Compiler CLI Entry Point (with -pack support)
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// ═══════════════════════════════════════════════════════════════

#include "bma.h"
#include "ast.h"
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

Program *parse_program(Arena *a, TokenList *tokens);
Program *parse_library(Arena *a, TokenList *tokens);
void interp_run(Program *prog, Arena *a);

// ═══════════════════════════════════════════════════════════════
// Path of current executable
// ═══════════════════════════════════════════════════════════════
static int GetSelfPath(char *out, size_t out_len) {
#ifdef _WIN32
    DWORD len = GetModuleFileNameA(NULL, out, (DWORD)out_len);
    if (len == 0 || len >= out_len) return 0;
    return 1;
#elif defined(__APPLE__)
    extern int _NSGetExecutablePath(char *buf, uint32_t *bufsize);
    uint32_t size = (uint32_t)out_len;
    if (_NSGetExecutablePath(out, &size) != 0) return 0;
    return 1;
#else
    ssize_t len = readlink("/proc/self/exe", out, out_len - 1);
    if (len <= 0) return 0;
    out[len] = 0;
    return 1;
#endif
}

static void GetSelfDir(char *out, size_t out_len) {
    char path[1024];
    if (!GetSelfPath(path, sizeof(path))) {
        strncpy(out, ".", out_len); out[out_len - 1] = 0;
        return;
    }
    char *last_sep = NULL;
    for (char *p = path; *p; p++)
        if (*p == '/' || *p == '\\') last_sep = p;
    if (last_sep) {
        size_t len = (size_t)(last_sep - path) + 1;
        if (len >= out_len) len = out_len - 1;
        memcpy(out, path, len); out[len] = 0;
    } else {
        strncpy(out, ".", out_len); out[out_len - 1] = 0;
    }
}

// ═══════════════════════════════════════════════════════════════
// Embedded source detection (for packed exes)
// ═══════════════════════════════════════════════════════════════
static const char EMBED_MAGIC[8] = { 'B','M','A','B','I','N','0','1' };

static char *TryLoadEmbeddedSource(size_t *out_size) {
    char path[1024];
    if (!GetSelfPath(path, sizeof(path))) return NULL;

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    if (file_size < 16) { fclose(f); return NULL; }

    fseek(f, file_size - 16, SEEK_SET);
    char magic[8];
    long long source_size = 0;
    if (fread(magic, 1, 8, f) != 8) { fclose(f); return NULL; }
    if (fread(&source_size, 1, 8, f) != 8) { fclose(f); return NULL; }

    if (memcmp(magic, EMBED_MAGIC, 8) != 0) { fclose(f); return NULL; }
    if (source_size <= 0 || source_size > file_size - 16) { fclose(f); return NULL; }

    fseek(f, file_size - 16 - (long)source_size, SEEK_SET);
    char *source = malloc((size_t)source_size + 1);
    if (!source) { fclose(f); return NULL; }
    if (fread(source, 1, (size_t)source_size, f) != (size_t)source_size) {
        free(source); fclose(f); return NULL;
    }
    source[source_size] = 0;
    fclose(f);

    if (out_size) *out_size = (size_t)source_size;
    return source;
}

// ═══════════════════════════════════════════════════════════════
// Pack: copy compiler + append source + append trailer
// ═══════════════════════════════════════════════════════════════
static int PackProgram(const char *source_path, const char *output_path) {
    char self_path[1024];
    if (!GetSelfPath(self_path, sizeof(self_path))) {
        fprintf(stderr, "Error: cannot determine self path\n");
        return 1;
    }

    FILE *src = fopen(source_path, "rb");
    if (!src) { fprintf(stderr, "Error: cannot open '%s'\n", source_path); return 1; }
    fseek(src, 0, SEEK_END);
    long src_size = ftell(src);
    fseek(src, 0, SEEK_SET);
    if (src_size < 0) { fprintf(stderr, "Error: source file too large\n"); fclose(src); return 1; }

    char *source_data = malloc((size_t)src_size);
    if (!source_data) { fprintf(stderr, "Error: OOM\n"); fclose(src); return 1; }
    if (fread(source_data, 1, (size_t)src_size, src) != (size_t)src_size) {
        fprintf(stderr, "Error: read failed\n"); free(source_data); fclose(src); return 1;
    }
    fclose(src);

    FILE *in = fopen(self_path, "rb");
    if (!in) { fprintf(stderr, "Error: cannot open self\n"); free(source_data); return 1; }

    FILE *out = fopen(output_path, "wb");
    if (!out) { fprintf(stderr, "Error: cannot create '%s'\n", output_path); fclose(in); free(source_data); return 1; }

    char buf[65536]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fprintf(stderr, "Error: write failed\n");
            fclose(in); fclose(out); free(source_data); return 1;
        }
    }
    fclose(in);

    if (src_size > 0)
        fwrite(source_data, 1, (size_t)src_size, out);
    free(source_data);

    long long src_size_ll = (long long)src_size;
    fwrite(EMBED_MAGIC, 1, 8, out);
    fwrite(&src_size_ll, 1, 8, out);

    fclose(out);
    printf("OK: Packed successfully -> %s\n", output_path);
    return 0;
}

// ═══════════════════════════════════════════════════════════════
// File reading & library loading
// ═══════════════════════════════════════════════════════════════
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); exit(1); }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fprintf(stderr, "OOM\n"); fclose(f); exit(1); }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = 0;
    fclose(f);
    return buf;
}

static void load_library(Arena *a, Program *prog, const char *libname) {
    char self_dir[1024];
    GetSelfDir(self_dir, sizeof(self_dir));

    char candidates[4][512];
    snprintf(candidates[0], sizeof(candidates[0]), "%slibrary/%s.bmalib", self_dir, libname);
    snprintf(candidates[1], sizeof(candidates[1]), "%s../library/%s.bmalib", self_dir, libname);
    snprintf(candidates[2], sizeof(candidates[2]), "library/%s.bmalib", libname);
    snprintf(candidates[3], sizeof(candidates[3]), "%s.bmalib", libname);

    FILE *f = NULL;
    for (int i = 0; i < 4; i++) {
        f = fopen(candidates[i], "rb");
        if (f) break;
    }

    if (!f) {
        fprintf(stderr, "Warning: library '%s' not found\n", libname);
        return;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *src = malloc((size_t)sz + 1);
    size_t rd = fread(src, 1, (size_t)sz, f);
    src[rd] = 0;
    fclose(f);

    TokenList *toks = lex(a, src);
    Program *lib = parse_library(a, toks);
    for (int i = 0; i < lib->func_count; i++) {
        if (prog->func_count < 500)
            prog->functions[prog->func_count++] = lib->functions[i];
    }
    free(src);
}

// ═══════════════════════════════════════════════════════════════
// Commands
// ═══════════════════════════════════════════════════════════════
static int run_source(const char *src) {
    Arena *a = arena_new();
    TokenList *toks = lex(a, src);
    Program *prog = parse_program(a, toks);
    for (int i = 0; i < prog->lib_count; i++)
        load_library(a, prog, prog->libraries[i]);
    interp_run(prog, a);
    arena_free(a);
    return 0;
}

int cmd_lex(const char *file) {
    char *src = read_file(file);
    Arena *a = arena_new();
    TokenList *toks = lex(a, src);
    for (int i = 0; i < toks->count; i++) {
        Token *t = &toks->data[i];
        if (t->kind == TK_EOF) { printf("%4d:%-3d  EOF\n", t->line, t->col); break; }
        printf("%4d:%-3d  %-12s  %s\n", t->line, t->col, tk_name(t->kind), t->text);
    }
    arena_free(a); free(src);
    return 0;
}

int cmd_check(const char *file) {
    char *src = read_file(file);
    Arena *a = arena_new();
    TokenList *toks = lex(a, src);
    Program *prog = parse_program(a, toks);
    for (int i = 0; i < prog->lib_count; i++)
        load_library(a, prog, prog->libraries[i]);
    printf("OK: Syntax is valid.\n");
    arena_free(a); free(src);
    return 0;
}

int cmd_run(const char *file) {
    char *src = read_file(file);
    int r = run_source(src);
    free(src);
    return r;
}

static void print_version(void) {
    printf("BMA Compiler %s\n", BMA_VERSION);
    printf("Author:  %s\n", BMA_AUTHOR);
    printf("Company: %s\n", BMA_COMPANY);
}

static void print_help(void) {
    printf("BMA Compiler %s\n", BMA_VERSION);
    printf("Author:  %s\n", BMA_AUTHOR);
    printf("Company: %s\n", BMA_COMPANY);
    printf("\n");
    printf("Usage:\n");
    printf("  bma --version                       show version\n");
    printf("  bma lex   <file.bma>                tokenize\n");
    printf("  bma check <file.bma>                validate syntax\n");
    printf("  bma run   <file.bma>                execute\n");
    printf("  bma -pack <source.bma> <out.exe>    create standalone exe\n");
}

// ═══════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════
int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    srand((unsigned)time(NULL));

    // Priority 1: is this a packed exe?
    size_t embedded_size = 0;
    char *embedded = TryLoadEmbeddedSource(&embedded_size);
    if (embedded != NULL) {
        run_source(embedded);
        free(embedded);

        int silent = 0;
        for (int i = 1; i < argc; i++)
            if (strcmp(argv[i], "-silent") == 0) { silent = 1; break; }

        if (!silent) {
            printf("\nPress Enter to exit...\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        }
        return 0;
    }

    // Priority 2: CLI
    if (argc < 2) { print_help(); return 1; }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) { print_version(); return 0; }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) { print_help(); return 0; }

    if (strcmp(argv[1], "-pack") == 0) {
        if (argc < 4) { fprintf(stderr, "Usage: bma -pack <source.bma> <output.exe>\n"); return 1; }
        return PackProgram(argv[2], argv[3]);
    }

    if (strcmp(argv[1], "lex") == 0 && argc >= 3) return cmd_lex(argv[2]);
    if (strcmp(argv[1], "check") == 0 && argc >= 3) return cmd_check(argv[2]);
    if (strcmp(argv[1], "run") == 0 && argc >= 3) return cmd_run(argv[2]);

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
}
