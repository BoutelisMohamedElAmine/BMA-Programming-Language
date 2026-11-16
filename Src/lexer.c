#include "bma.h"

const char *tk_name(TK k) {
    switch (k) {
    case TK_EOF: return "EOF";
    case TK_IDENT: return "IDENT";
    case TK_STRING: return "STRING";
    case TK_INT: return "INT";
    case TK_REAL: return "REAL";
    case TK_COLON: return ":";
    case TK_DOT: return ".";
    case TK_COMMA: return ",";
    case TK_EQUAL: return "=";
    case TK_MEMBER: return "MEMBER";
    case TK_LPAREN: return "(";
    case TK_RPAREN: return ")";
    case TK_LBRACKET: return "[";
    case TK_RBRACKET: return "]";
    case TK_PLUS: return "+";
    case TK_MINUS: return "-";
    case TK_STAR: return "*";
    case TK_SLASH: return "/";
    case TK_EQEQ: return "==";
    case TK_NEQ: return "<>";
    case TK_LT: return "<";
    case TK_GT: return ">";
    case TK_LE: return "<=";
    case TK_GE: return ">=";
    case TK_KW_BMA_PRO: return "BMA_Pro";
    case TK_KW_BMA_LIB: return "BMA_Lib";
    case TK_KW_LIBRARY: return "library";
    case TK_KW_VARIABLE: return "variable";
    case TK_KW_CODE: return "code";
    case TK_KW_END: return "end";
    case TK_KW_CLASS: return "class";
    case TK_KW_END_CLASS: return "end_class";
    case TK_KW_STRING: return "string";
    case TK_KW_INTEGER: return "integer";
    case TK_KW_REAL: return "real";
    case TK_KW_BOOLEAN: return "boolean";
    case TK_KW_VOID: return "void";
    case TK_KW_WRITE: return "write";
    case TK_KW_WRITELINE: return "writeline";
    case TK_KW_READ: return "read";
    case TK_KW_READLINE: return "readLine";
    case TK_KW_IF: return "if";
    case TK_KW_THEN: return "then";
    case TK_KW_ELSE: return "else";
    case TK_KW_END_IF: return "end_if";
    case TK_KW_WHILE: return "while";
    case TK_KW_DO: return "do";
    case TK_KW_END_WHILE: return "end_while";
    case TK_KW_FOR: return "for";
    case TK_KW_TO: return "to";
    case TK_KW_STEP: return "step";
    case TK_KW_END_FOR: return "end_for";
    case TK_KW_AND: return "and";
    case TK_KW_OR: return "or";
    case TK_KW_NOT: return "not";
    case TK_KW_TRUE: return "true";
    case TK_KW_FALSE: return "false";
    case TK_KW_BREAK: return "break";
    case TK_KW_CONTINUE: return "continue";
    case TK_KW_FUNCTION: return "function";
    case TK_KW_END_FUNC: return "end_func";
    case TK_KW_RETURN: return "return";
    case TK_KW_MOD: return "mod";
    case TK_KW_DIV: return "div";
    }
    return "?";
}

typedef struct {
    const char *src;
    int pos, line, col;
    Arena *arena;
} Lexer;

static char L_peek(Lexer *L) { return L->src[L->pos]; }
static char L_peek2(Lexer *L) { return L->src[L->pos] ? L->src[L->pos+1] : 0; }

static void L_adv(Lexer *L) {
    if (L->src[L->pos] == '\n') { L->line++; L->col = 1; }
    else L->col++;
    L->pos++;
}

static void skip_ws(Lexer *L) {
    for (;;) {
        while (isspace((unsigned char)L_peek(L))) L_adv(L);
        if (L_peek(L) == '/' && L_peek2(L) == '/') {
            while (L_peek(L) && L_peek(L) != '\n') L_adv(L);
            continue;
        }
        if (L_peek(L) == '/' && L_peek2(L) == '*') {
            L_adv(L); L_adv(L);
            while (L_peek(L) && !(L_peek(L)=='*' && L_peek2(L)=='/')) L_adv(L);
            if (L_peek(L)) { L_adv(L); L_adv(L); }
            continue;
        }
        break;
    }
}

typedef struct { const char *word; TK kind; } KW;

static KW keywords[] = {
    {"bma_pro",TK_KW_BMA_PRO}, {"bma_lib",TK_KW_BMA_LIB},
    {"library",TK_KW_LIBRARY}, {"const",TK_KW_CONST}, 
     {"variable",TK_KW_VARIABLE},
    {"code",TK_KW_CODE},       {"end",TK_KW_END},
    {"class",TK_KW_CLASS},     {"end_class",TK_KW_END_CLASS},
    {"string",TK_KW_STRING},   {"integer",TK_KW_INTEGER},
    {"real",TK_KW_REAL},       {"boolean",TK_KW_BOOLEAN},
    {"void",TK_KW_VOID},
    {"write",TK_KW_WRITE},     {"writeline",TK_KW_WRITELINE},
    {"read",TK_KW_READ},       {"readline",TK_KW_READLINE},
    {"if",TK_KW_IF},           {"then",TK_KW_THEN},
    {"else",TK_KW_ELSE},       {"end_if",TK_KW_END_IF},
    {"while",TK_KW_WHILE},     {"do",TK_KW_DO},
    {"end_while",TK_KW_END_WHILE},
    {"for",TK_KW_FOR},         {"to",TK_KW_TO},
    {"step",TK_KW_STEP},       {"end_for",TK_KW_END_FOR},
    {"and",TK_KW_AND},         {"or",TK_KW_OR},
    {"not",TK_KW_NOT},
    {"true",TK_KW_TRUE},       {"false",TK_KW_FALSE},
    {"break",TK_KW_BREAK},     {"continue",TK_KW_CONTINUE},
    {"function",TK_KW_FUNCTION},{"end_func",TK_KW_END_FUNC},
    {"return",TK_KW_RETURN},
    {"mod",TK_KW_MOD},         {"div",TK_KW_DIV},
};

static TK lookup_kw(const char *s) {
    for (size_t i = 0; i < sizeof(keywords)/sizeof(*keywords); i++)
        if (strcmp(keywords[i].word, s) == 0) return keywords[i].kind;
    return TK_IDENT;
}

static Token lex_one(Lexer *L) {
    skip_ws(L);
    int line = L->line, col = L->col;
    char c = L_peek(L);
    if (!c) return (Token){ TK_EOF, "", line, col };

    switch (c) {
    case ':': L_adv(L); return (Token){TK_COLON,    ":", line, col};
    case ',': L_adv(L); return (Token){TK_COMMA,    ",", line, col};
    case '(': L_adv(L); return (Token){TK_LPAREN,   "(", line, col};
    case ')': L_adv(L); return (Token){TK_RPAREN,   ")", line, col};
    case '[': L_adv(L); return (Token){TK_LBRACKET, "[", line, col};
    case ']': L_adv(L); return (Token){TK_RBRACKET, "]", line, col};
    case '+': L_adv(L); return (Token){TK_PLUS,     "+", line, col};
    case '-': L_adv(L); return (Token){TK_MINUS,    "-", line, col};
    case '*': L_adv(L); return (Token){TK_STAR,     "*", line, col};
    case '/': L_adv(L); return (Token){TK_SLASH,    "/", line, col};
    case '=':
        if (L_peek2(L) == '=') { L_adv(L); L_adv(L); return (Token){TK_EQEQ, "==", line, col}; }
        L_adv(L); return (Token){TK_EQUAL, "=", line, col};
    case '<':
        if (L_peek2(L) == '=') { L_adv(L); L_adv(L); return (Token){TK_LE, "<=", line, col}; }
        if (L_peek2(L) == '>') { L_adv(L); L_adv(L); return (Token){TK_NEQ, "<>", line, col}; }
        L_adv(L); return (Token){TK_LT, "<", line, col};
    case '>':
        if (L_peek2(L) == '=') { L_adv(L); L_adv(L); return (Token){TK_GE, ">=", line, col}; }
        L_adv(L); return (Token){TK_GT, ">", line, col};
    case '.':
        if (L->pos > 0 && isspace((unsigned char)L->src[L->pos-1])) {
            L_adv(L); return (Token){TK_DOT, ".", line, col};
        }
        L_adv(L); return (Token){TK_MEMBER, ".", line, col};
    case '\'':
    {
        L_adv(L);
        char *buf = arena_alloc(L->arena, strlen(L->src) + 1);
        int n = 0;
        while (L_peek(L) && L_peek(L) != '\'') {
            if (L_peek(L) == '\'' && L_peek2(L) == '\'') {
                buf[n++] = '\''; L_adv(L); L_adv(L);
            } else {
                buf[n++] = L_peek(L); L_adv(L);
            }
        }
        if (L_peek(L) == '\'') L_adv(L);
        buf[n] = 0;
        return (Token){TK_STRING, buf, line, col};
    }
    }

    if (isdigit((unsigned char)c)) {
        int start = L->pos;
        while (isdigit((unsigned char)L_peek(L))) L_adv(L);
        bool is_real = false;
        if (L_peek(L) == '.' && isdigit((unsigned char)L_peek2(L))) {
            is_real = true;
            L_adv(L);
            while (isdigit((unsigned char)L_peek(L))) L_adv(L);
        }
        char *num = arena_strndup(L->arena, L->src + start, L->pos - start);
        return (Token){ is_real ? TK_REAL : TK_INT, num, line, col };
    }

    if (isalpha((unsigned char)c) || c == '_') {
        int start = L->pos;
        while (isalnum((unsigned char)L_peek(L)) || L_peek(L) == '_') L_adv(L);
        char *w = arena_strndup(L->arena, L->src + start, L->pos - start);
        for (char *p = w; *p; p++) *p = (char)tolower((unsigned char)*p);
        return (Token){ lookup_kw(w), w, line, col };
    }

    bma_error_at(line, col, "Unexpected character '%c' (0x%02X)", c, (unsigned char)c);
    return (Token){TK_EOF, "", line, col};
}

TokenList *lex(Arena *a, const char *src) {
    Lexer L = { src, 0, 1, 1, a };
    TokenList *list = arena_alloc(a, sizeof(TokenList));
    list->cap = 4096;
    list->count = 0;
    list->data = arena_alloc(a, sizeof(Token) * list->cap);
    for (;;) {
        Token t = lex_one(&L);
        if (list->count >= list->cap)
            bma_error("Too many tokens (max %d)", list->cap);
        list->data[list->count++] = t;
        if (t.kind == TK_EOF) break;
    }
    return list;
}
