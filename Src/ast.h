// ═══════════════════════════════════════════════════════════════
// ast.h — Abstract Syntax Tree
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// ═══════════════════════════════════════════════════════════════

#ifndef AST_H
#define AST_H

#include "bma.h"

typedef enum {
    EX_INT, EX_REAL, EX_STRING, EX_BOOL,
    EX_IDENT, EX_BINARY, EX_UNARY, EX_CALL, EX_MEMBER, EX_INDEX,
    EX_ARRAY_LITERAL
} ExprKind;

typedef struct Expr Expr;
struct Expr {
    ExprKind kind;
    int line, col;
    union {
        long long int_val;
        double real_val;
        char *str_val;
        bool bool_val;
        char *ident_name;
        struct { char *op; Expr *left, *right; } binary;
        struct { char *op; Expr *operand; } unary;
        struct {
            char *name;
            Expr **args;
            int arg_count;
            Expr *method_target;   // NULL for regular calls
        } call;
        struct { Expr *target; char *member; } member;
        struct { Expr *target; Expr *index; } index;
        struct { Expr **items; int count; } array_literal;
    };
};

typedef enum {
    ST_VAR_DECL, ST_CONST_DECL, ST_ASSIGN, ST_INDEX_ASSIGN, ST_MEMBER_ASSIGN,
    ST_IF, ST_WHILE, ST_FOR, ST_BREAK, ST_CONTINUE, ST_RETURN,
    ST_EXPR, ST_READ, ST_READLINE
} StmtKind;

typedef struct Stmt Stmt;
struct Stmt {
    StmtKind kind;
    int line, col;
    union {
        struct { char *name; char *type_name; Expr *array_size; } var_decl;
        struct { char *name; Expr *value; } const_decl;
        struct { char *name; Expr *value; } assign;
        struct { char *name; Expr *index; Expr *value; } index_assign;
        struct { Expr *target; Expr *value; } member_assign;
        struct { Expr *cond; Stmt **then_stmts; int then_count; Stmt **else_stmts; int else_count; } if_stmt;
        struct { Expr *cond; Stmt **body; int body_count; } while_stmt;
        struct { char *var; Expr *start; Expr *end; Expr *step; Stmt **body; int body_count; } for_stmt;
        struct { Expr *value; } return_stmt;
        struct { Expr *expr; } expr_stmt;
        struct { char *var; } read_stmt;
        struct { char *var; } readline_stmt;
    };
};

typedef struct {
    char *name;
    char *type_name;
    Expr *array_size;
} Param;

typedef struct {
    char *name;
    Param *params;
    int param_count;
    char *return_type;
    Stmt **body;
    int body_count;
    bool is_static;            // ← NEW
} FunctionDecl;

typedef struct {
    char *name;
    char *parent_name;         // ← NEW: NULL = no inheritance
    Stmt **fields;
    bool *field_private;       // ← NEW: parallel to fields
    int field_count;
    FunctionDecl **methods;
    int method_count;
} ClassDecl;

typedef struct {
    char *name;
    char **libraries;
    int lib_count;
    ClassDecl **classes;
    int class_count;
    Stmt **variables;
    int var_count;
    FunctionDecl **functions;
    int func_count;
    Stmt **statements;
    int stmt_count;
} Program;

#endif