// ---------------------------------------------------------------
// bma.h — BMA Language Core Header
// BMA: The Boutelis Mohamed El Amine Programming Language
//
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// All rights reserved.
//
// This file is part of the BMA Compiler Project.
// Unauthorized reproduction or distribution is prohibited.
// ---------------------------------------------------------------

#ifndef BMA_H
#define BMA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#define BMA_VERSION  "3.0.0"
#define BMA_AUTHOR   "Boutelis Mohamed El Amine"
#define BMA_COMPANY  "BMA SOFTWARE ALGERIA"

// ---------------------------------------------------------------
// Arena Allocator
// ---------------------------------------------------------------
typedef struct Block {
            struct Block *next;
            size_t used, cap;
            char data[];
} Block;

typedef struct { Block *head; } Arena;

Arena *arena_new(void);
void  *arena_alloc(Arena *a, size_t size);
char  *arena_strdup(Arena *a, const char *s);
char  *arena_strndup(Arena *a, const char *s, size_t n);
void   arena_free(Arena *a);

// ---------------------------------------------------------------
// Error Handling
// ---------------------------------------------------------------
void bma_error(const char *fmt, ...);
void bma_error_at(int line, int col, const char *fmt, ...);

// ---------------------------------------------------------------
// Tokens
// ---------------------------------------------------------------
typedef enum {
            TK_EOF, TK_IDENT, TK_STRING, TK_INT, TK_REAL,
            TK_COLON, TK_DOT, TK_COMMA, TK_EQUAL, TK_MEMBER,
            TK_LPAREN, TK_RPAREN, TK_LBRACKET, TK_RBRACKET,
            TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH,
            TK_EQEQ, TK_NEQ, TK_LT, TK_GT, TK_LE, TK_GE,
            TK_KW_BMA_PRO, TK_KW_BMA_LIB, TK_KW_LIBRARY,
            TK_KW_VARIABLE, TK_KW_CONST, TK_KW_CODE, TK_KW_END,
            TK_KW_CLASS, TK_KW_END_CLASS,
            TK_KW_STRING, TK_KW_INTEGER, TK_KW_REAL, TK_KW_BOOLEAN, TK_KW_VOID,
            TK_KW_WRITE, TK_KW_WRITELINE, TK_KW_READ, TK_KW_READLINE,
            TK_KW_IF, TK_KW_THEN, TK_KW_ELSE, TK_KW_END_IF,
            TK_KW_WHILE, TK_KW_DO, TK_KW_END_WHILE,
            TK_KW_FOR, TK_KW_TO, TK_KW_STEP, TK_KW_END_FOR,
            TK_KW_AND, TK_KW_OR, TK_KW_NOT,
            TK_KW_TRUE, TK_KW_FALSE,
            TK_KW_BREAK, TK_KW_CONTINUE,
            TK_KW_FUNCTION, TK_KW_END_FUNC, TK_KW_RETURN,
            TK_KW_MOD, TK_KW_DIV,
} TK;

typedef struct { TK kind; const char *text; int line, col; } Token;
typedef struct { Token *data; int count, cap; } TokenList;

const char *tk_name(TK k);
TokenList  *lex(Arena *a, const char *src);

int cmd_lex(const char *file);
int cmd_check(const char *file);
int cmd_run(const char *file);

#endif