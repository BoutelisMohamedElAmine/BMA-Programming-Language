#include "bma.h"

Arena *arena_new(void) {
    Arena *a = calloc(1, sizeof(Arena));
    if (!a) bma_error("Out of memory");
    return a;
}

void *arena_alloc(Arena *a, size_t size) {
    size = (size + 15) & ~((size_t)15);
    if (!a->head || a->head->used + size > a->head->cap) {
        size_t cap = size < 4096 ? 4096 : size;
        Block *b = malloc(sizeof(Block) + cap);
        if (!b) bma_error("Out of memory");
        b->next = a->head;
        b->used = 0;
        b->cap  = cap;
        a->head = b;
    }
    void *p = a->head->data + a->head->used;
    a->head->used += size;
    return p;
}

char *arena_strdup(Arena *a, const char *s) {
    size_t n = strlen(s) + 1;
    char *p = arena_alloc(a, n);
    memcpy(p, s, n);
    return p;
}

char *arena_strndup(Arena *a, const char *s, size_t n) {
    char *p = arena_alloc(a, n + 1);
    memcpy(p, s, n);
    p[n] = 0;
    return p;
}

void arena_free(Arena *a) {
    Block *b = a->head;
    while (b) { Block *n = b->next; free(b); b = n; }
    free(a);
}
