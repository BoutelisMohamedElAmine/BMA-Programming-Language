// ═══════════════════════════════════════════════════════════════
// parser.c — Recursive Descent Parser (Full OOP)
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// ═══════════════════════════════════════════════════════════════

#include "bma.h"
#include "ast.h"

typedef struct { TokenList *tokens; int pos; Arena *arena; } Parser;

static Token *P_current(Parser *P) { return &P->tokens->data[P->pos]; }
static void P_adv(Parser *P) { if (P->pos < P->tokens->count - 1) P->pos++; }

static Token *P_expect(Parser *P, TK kind, const char *msg) {
    if (P_current(P)->kind != kind)
        bma_error_at(P_current(P)->line, P_current(P)->col, "%s (got '%s')", msg, P_current(P)->text);
    Token *t = P_current(P);
    P_adv(P);
    return t;
}

static bool P_match(Parser *P, TK kind) {
    if (P_current(P)->kind == kind) { P_adv(P); return true; }
    return false;
}

// Check if current token is an identifier with given text
static bool P_is_word(Parser *P, const char *word) {
    Token *t = P_current(P);
    return t->kind == TK_IDENT && strcmp(t->text, word) == 0;
}

static Expr *new_expr(Parser *P, ExprKind kind) {
    Expr *e = arena_alloc(P->arena, sizeof(Expr));
    e->kind = kind; e->line = P_current(P)->line; e->col = P_current(P)->col;
    return e;
}

static Stmt *new_stmt(Parser *P, StmtKind kind) {
    Stmt *s = arena_alloc(P->arena, sizeof(Stmt));
    s->kind = kind; s->line = P_current(P)->line; s->col = P_current(P)->col;
    return s;
}

static Expr *parse_expr(Parser *P);

static Expr *parse_primary(Parser *P) {
    Token *t = P_current(P);

    if (t->kind == TK_INT) { P_adv(P); Expr *e = new_expr(P, EX_INT); e->int_val = atoll(t->text); return e; }
    if (t->kind == TK_REAL) { P_adv(P); Expr *e = new_expr(P, EX_REAL); e->real_val = atof(t->text); return e; }
    if (t->kind == TK_STRING) { P_adv(P); Expr *e = new_expr(P, EX_STRING); e->str_val = (char*)t->text; return e; }
    if (t->kind == TK_KW_TRUE) { P_adv(P); Expr *e = new_expr(P, EX_BOOL); e->bool_val = true; return e; }
    if (t->kind == TK_KW_FALSE) { P_adv(P); Expr *e = new_expr(P, EX_BOOL); e->bool_val = false; return e; }

    if (P_match(P, TK_LBRACKET)) {
        Expr *e = new_expr(P, EX_ARRAY_LITERAL);
        e->array_literal.items = arena_alloc(P->arena, sizeof(Expr*) * 100);
        e->array_literal.count = 0;
        if (P_current(P)->kind != TK_RBRACKET) {
            e->array_literal.items[e->array_literal.count++] = parse_expr(P);
            while (P_match(P, TK_COMMA))
                e->array_literal.items[e->array_literal.count++] = parse_expr(P);
        }
        P_expect(P, TK_RBRACKET, "Expected ']'");
        return e;
    }

    if (t->kind == TK_IDENT) {
        P_adv(P);
        if (P_match(P, TK_LPAREN)) {
            Expr *e = new_expr(P, EX_CALL);
            e->call.name = (char*)t->text;
            e->call.method_target = NULL;
            e->call.args = arena_alloc(P->arena, sizeof(Expr*) * 32);
            e->call.arg_count = 0;
            if (P_current(P)->kind != TK_RPAREN) {
                e->call.args[e->call.arg_count++] = parse_expr(P);
                while (P_match(P, TK_COMMA))
                    e->call.args[e->call.arg_count++] = parse_expr(P);
            }
            P_expect(P, TK_RPAREN, "Expected ')'");
            return e;
        }
        if (P_match(P, TK_LBRACKET)) {
            Expr *e = new_expr(P, EX_INDEX);
            e->index.target = new_expr(P, EX_IDENT);
            e->index.target->ident_name = (char*)t->text;
            e->index.index = parse_expr(P);
            P_expect(P, TK_RBRACKET, "Expected ']'");
            return e;
        }
        Expr *e = new_expr(P, EX_IDENT);
        e->ident_name = (char*)t->text;
        while (P_match(P, TK_MEMBER)) {
            Token *mem = P_expect(P, TK_IDENT, "Expected member name");
            if (P_current(P)->kind == TK_LPAREN) {
                P_adv(P);
                Expr *call = new_expr(P, EX_CALL);
                call->call.name = (char*)mem->text;
                call->call.method_target = e;
                call->call.args = arena_alloc(P->arena, sizeof(Expr*) * 32);
                call->call.arg_count = 0;
                if (P_current(P)->kind != TK_RPAREN) {
                    call->call.args[call->call.arg_count++] = parse_expr(P);
                    while (P_match(P, TK_COMMA))
                        call->call.args[call->call.arg_count++] = parse_expr(P);
                }
                P_expect(P, TK_RPAREN, "Expected ')'");
                e = call;
            } else {
                Expr *m = new_expr(P, EX_MEMBER);
                m->member.target = e;
                m->member.member = (char*)mem->text;
                e = m;
            }
        }
        return e;
    }

    if (P_match(P, TK_LPAREN)) {
        Expr *e = parse_expr(P);
        P_expect(P, TK_RPAREN, "Expected ')'");
        return e;
    }
    bma_error_at(t->line, t->col, "Expected expression, got '%s'", t->text);
    return NULL;
}

static Expr *parse_unary(Parser *P) {
    if (P_match(P, TK_MINUS)) {
        Expr *e = new_expr(P, EX_UNARY); e->unary.op = "-"; e->unary.operand = parse_unary(P); return e;
    }
    if (P_match(P, TK_KW_NOT)) {
        Expr *e = new_expr(P, EX_UNARY); e->unary.op = "not"; e->unary.operand = parse_unary(P); return e;
    }
    return parse_primary(P);
}

static Expr *parse_mul(Parser *P) {
    Expr *left = parse_unary(P);
    while (P_current(P)->kind == TK_STAR || P_current(P)->kind == TK_SLASH ||
           P_current(P)->kind == TK_KW_MOD || P_current(P)->kind == TK_KW_DIV) {
        Token *op = P_current(P); P_adv(P);
        Expr *right = parse_unary(P);
        Expr *e = new_expr(P, EX_BINARY);
        e->binary.op = (char*)op->text; e->binary.left = left; e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr *parse_add(Parser *P) {
    Expr *left = parse_mul(P);
    while (P_current(P)->kind == TK_PLUS || P_current(P)->kind == TK_MINUS) {
        Token *op = P_current(P); P_adv(P);
        Expr *right = parse_mul(P);
        Expr *e = new_expr(P, EX_BINARY);
        e->binary.op = (char*)op->text; e->binary.left = left; e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr *parse_cmp(Parser *P) {
    Expr *left = parse_add(P);
    while (P_current(P)->kind >= TK_EQEQ && P_current(P)->kind <= TK_GE) {
        Token *op = P_current(P); P_adv(P);
        Expr *right = parse_add(P);
        Expr *e = new_expr(P, EX_BINARY);
        e->binary.op = (char*)op->text; e->binary.left = left; e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr *parse_and(Parser *P) {
    Expr *left = parse_cmp(P);
    while (P_match(P, TK_KW_AND)) {
        Expr *right = parse_cmp(P);
        Expr *e = new_expr(P, EX_BINARY);
        e->binary.op = "and"; e->binary.left = left; e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr *parse_or(Parser *P) {
    Expr *left = parse_and(P);
    while (P_match(P, TK_KW_OR)) {
        Expr *right = parse_and(P);
        Expr *e = new_expr(P, EX_BINARY);
        e->binary.op = "or"; e->binary.left = left; e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr *parse_expr(Parser *P) { return parse_or(P); }

static Stmt *parse_stmt(Parser *P);

static void parse_var_block(Parser *P, Stmt **out_vars, int *count) {
    P_expect(P, TK_KW_VARIABLE, "Expected 'variable'");
    P_expect(P, TK_COLON, "Expected ':'");
    do {
        Token *name = P_expect(P, TK_IDENT, "Expected var name");
        P_expect(P, TK_EQUAL, "Expected '='");
        Token *type = P_current(P); P_adv(P);
        Stmt *s = new_stmt(P, ST_VAR_DECL);
        s->var_decl.name = (char*)name->text;
        s->var_decl.type_name = (char*)type->text;
        if (P_match(P, TK_LBRACKET)) {
            s->var_decl.array_size = parse_expr(P);
            P_expect(P, TK_RBRACKET, "Expected ']'");
        }
        out_vars[(*count)++] = s;
    } while (P_match(P, TK_COMMA));
    P_expect(P, TK_DOT, "Expected '.'");
}

static Stmt *parse_stmt(Parser *P) {
    Token *t = P_current(P);

    if (t->kind == TK_KW_IF) {
        P_adv(P);
        Expr *cond = parse_expr(P);
        P_expect(P, TK_KW_THEN, "Expected 'then'");
        Stmt *s = new_stmt(P, ST_IF);
        s->if_stmt.cond = cond;
        s->if_stmt.then_stmts = arena_alloc(P->arena, sizeof(Stmt*) * 100);
        s->if_stmt.then_count = 0;
        while (P_current(P)->kind != TK_KW_ELSE && P_current(P)->kind != TK_KW_END_IF)
            s->if_stmt.then_stmts[s->if_stmt.then_count++] = parse_stmt(P);
        s->if_stmt.else_stmts = arena_alloc(P->arena, sizeof(Stmt*) * 100);
        s->if_stmt.else_count = 0;
        if (P_match(P, TK_KW_ELSE))
            while (P_current(P)->kind != TK_KW_END_IF)
                s->if_stmt.else_stmts[s->if_stmt.else_count++] = parse_stmt(P);
        P_expect(P, TK_KW_END_IF, "Expected 'end_if'");
        P_expect(P, TK_DOT, "Expected '.'");
        return s;
    }

    if (t->kind == TK_KW_WHILE) {
        P_adv(P);
        Expr *cond = parse_expr(P);
        P_expect(P, TK_KW_DO, "Expected 'do'");
        Stmt *s = new_stmt(P, ST_WHILE);
        s->while_stmt.cond = cond;
        s->while_stmt.body = arena_alloc(P->arena, sizeof(Stmt*) * 100);
        s->while_stmt.body_count = 0;
        while (P_current(P)->kind != TK_KW_END_WHILE)
            s->while_stmt.body[s->while_stmt.body_count++] = parse_stmt(P);
        P_expect(P, TK_KW_END_WHILE, "Expected 'end_while'");
        P_expect(P, TK_DOT, "Expected '.'");
        return s;
    }

    if (t->kind == TK_KW_FOR) {
        P_adv(P);
        Token *var = P_expect(P, TK_IDENT, "Expected var");
        P_expect(P, TK_EQUAL, "Expected '='");
        Expr *start = parse_expr(P);
        P_expect(P, TK_KW_TO, "Expected 'to'");
        Expr *end = parse_expr(P);
        Expr *step = NULL;
        if (P_match(P, TK_KW_STEP)) step = parse_expr(P);
        P_expect(P, TK_KW_DO, "Expected 'do'");
        Stmt *s = new_stmt(P, ST_FOR);
        s->for_stmt.var = (char*)var->text;
        s->for_stmt.start = start; s->for_stmt.end = end; s->for_stmt.step = step;
        s->for_stmt.body = arena_alloc(P->arena, sizeof(Stmt*) * 100);
        s->for_stmt.body_count = 0;
        while (P_current(P)->kind != TK_KW_END_FOR)
            s->for_stmt.body[s->for_stmt.body_count++] = parse_stmt(P);
        P_expect(P, TK_KW_END_FOR, "Expected 'end_for'");
        P_expect(P, TK_DOT, "Expected '.'");
        return s;
    }

    if (t->kind == TK_KW_BREAK) { P_adv(P); P_expect(P, TK_DOT, "Expected '.'"); return new_stmt(P, ST_BREAK); }
    if (t->kind == TK_KW_CONTINUE) { P_adv(P); P_expect(P, TK_DOT, "Expected '.'"); return new_stmt(P, ST_CONTINUE); }

    if (t->kind == TK_KW_RETURN) {
        P_adv(P);
        Stmt *s = new_stmt(P, ST_RETURN);
        if (P_current(P)->kind != TK_DOT) s->return_stmt.value = parse_expr(P);
        P_expect(P, TK_DOT, "Expected '.'");
        return s;
    }

    if (t->kind == TK_KW_WRITE || t->kind == TK_KW_WRITELINE) {
        char *name = (char*)t->text;
        P_adv(P);
        P_expect(P, TK_LPAREN, "Expected '('");
        Expr *e = new_expr(P, EX_CALL);
        e->call.name = name;
        e->call.method_target = NULL;
        e->call.args = arena_alloc(P->arena, sizeof(Expr*) * 32);
        e->call.arg_count = 0;
        if (P_current(P)->kind != TK_RPAREN) {
            e->call.args[e->call.arg_count++] = parse_expr(P);
            while (P_match(P, TK_COMMA)) e->call.args[e->call.arg_count++] = parse_expr(P);
        }
        P_expect(P, TK_RPAREN, "Expected ')'");
        P_expect(P, TK_DOT, "Expected '.'");
        Stmt *s = new_stmt(P, ST_EXPR);
        s->expr_stmt.expr = e;
        return s;
    }

    if (t->kind == TK_KW_READ || t->kind == TK_KW_READLINE) {
        bool is_rl = (t->kind == TK_KW_READLINE);
        P_adv(P);
        Stmt *s = new_stmt(P, is_rl ? ST_READLINE : ST_READ);
        if (P_match(P, TK_LPAREN)) {
            Token *var = P_expect(P, TK_IDENT, "Expected var name");
            if (is_rl) s->readline_stmt.var = (char*)var->text;
            else       s->read_stmt.var = (char*)var->text;
            P_expect(P, TK_RPAREN, "Expected ')'");
        } else {
            if (is_rl) s->readline_stmt.var = NULL;
            else       s->read_stmt.var = NULL;
        }
        P_expect(P, TK_DOT, "Expected '.'");
        return s;
    }

    if (t->kind == TK_IDENT) {
        Expr *e = parse_expr(P);
        if (P_match(P, TK_EQUAL)) {
            Expr *val = parse_expr(P);
            P_expect(P, TK_DOT, "Expected '.'");
            Stmt *s = new_stmt(P, ST_ASSIGN);
            if (e->kind == EX_IDENT) {
                s->assign.name = e->ident_name; s->assign.value = val;
            } else if (e->kind == EX_INDEX) {
                s->kind = ST_INDEX_ASSIGN;
                s->index_assign.name = e->index.target->ident_name;
                s->index_assign.index = e->index.index;
                s->index_assign.value = val;
            } else if (e->kind == EX_MEMBER) {
                s->kind = ST_MEMBER_ASSIGN;
                s->member_assign.target = e;
                s->member_assign.value = val;
            } else {
                bma_error("Invalid assignment target");
            }
            return s;
        }
        P_expect(P, TK_DOT, "Expected '.'");
        Stmt *s = new_stmt(P, ST_EXPR);
        s->expr_stmt.expr = e;
        return s;
    }

    bma_error_at(t->line, t->col, "Unexpected statement starting with '%s'", t->text);
    return NULL;
}

static FunctionDecl *parse_method(Parser *P, bool is_static) {
    P_expect(P, TK_KW_FUNCTION, "Expected 'function'");
    P_expect(P, TK_COLON, "Expected ':'");
    Token *fname = P_expect(P, TK_IDENT, "Expected method name");
    P_expect(P, TK_LPAREN, "Expected '('");

    FunctionDecl *f = arena_alloc(P->arena, sizeof(FunctionDecl));
    f->name = (char*)fname->text;
    f->is_static = is_static;
    f->params = arena_alloc(P->arena, sizeof(Param) * 20);
    f->param_count = 0;

    if (P_current(P)->kind != TK_RPAREN) {
        do {
            Token *pname = P_expect(P, TK_IDENT, "Expected param name");
            P_expect(P, TK_EQUAL, "Expected '='");
            Token *ptype = P_current(P); P_adv(P);
            f->params[f->param_count].name = (char*)pname->text;
            f->params[f->param_count].type_name = (char*)ptype->text;
            f->param_count++;
        } while (P_match(P, TK_COMMA));
    }
    P_expect(P, TK_RPAREN, "Expected ')'");
    P_expect(P, TK_KW_RETURN, "Expected 'return'");
    Token *rtype = P_current(P); P_adv(P);
    f->return_type = (char*)rtype->text;
    P_expect(P, TK_DOT, "Expected '.'");

    f->body = arena_alloc(P->arena, sizeof(Stmt*) * 200);
    f->body_count = 0;

    if (P_current(P)->kind == TK_KW_VARIABLE) {
        Stmt **locals = arena_alloc(P->arena, sizeof(Stmt*) * 100);
        int local_count = 0;
        parse_var_block(P, locals, &local_count);
        for (int i = 0; i < local_count; i++)
            f->body[f->body_count++] = locals[i];
    }

    while (P_current(P)->kind != TK_KW_END_FUNC && P_current(P)->kind != TK_EOF)
        f->body[f->body_count++] = parse_stmt(P);

    P_expect(P, TK_KW_END_FUNC, "Expected 'end_func'");
    P_expect(P, TK_DOT, "Expected '.'");
    return f;
}

Program *parse_program(Arena *a, TokenList *tokens) {
    Parser P = { tokens, 0, a };
    Program *prog = arena_alloc(a, sizeof(Program));
    prog->libraries = arena_alloc(a, sizeof(char*) * 20);
    prog->lib_count = 0;
    prog->classes = arena_alloc(a, sizeof(ClassDecl*) * 50);
    prog->class_count = 0;
    prog->variables = arena_alloc(a, sizeof(Stmt*) * 100);
    prog->var_count = 0;
    prog->functions = arena_alloc(a, sizeof(FunctionDecl*) * 500);
    prog->func_count = 0;
    prog->statements = arena_alloc(a, sizeof(Stmt*) * 500);
    prog->stmt_count = 0;

    P_expect(&P, TK_KW_BMA_PRO, "Program must start with BMA_Pro");
    P_expect(&P, TK_COLON, "Expected ':'");
    Token *name = P_expect(&P, TK_IDENT, "Expected program name");
    prog->name = (char*)name->text;
    P_expect(&P, TK_DOT, "Expected '.'");

    if (P_match(&P, TK_KW_LIBRARY)) {
        P_expect(&P, TK_COLON, "Expected ':'");
        do {
            Token *lib = P_expect(&P, TK_IDENT, "Expected library name");
            prog->libraries[prog->lib_count++] = (char*)lib->text;
        } while (P_match(&P, TK_COMMA));
        P_expect(&P, TK_DOT, "Expected '.'");
    }

    // ═══ Classes with all OOP features ═══
    while (P_current(&P)->kind == TK_KW_CLASS) {
        P_adv(&P);
        P_expect(&P, TK_COLON, "Expected ':'");
        Token *cname = P_expect(&P, TK_IDENT, "Expected class name");

        ClassDecl *c = arena_alloc(a, sizeof(ClassDecl));
        c->name = (char*)cname->text;
        c->parent_name = NULL;

        // extends clause
        if (P_is_word(&P, "extends")) {
            P_adv(&P);
            Token *pname = P_expect(&P, TK_IDENT, "Expected parent class name");
            c->parent_name = (char*)pname->text;
        }
        P_expect(&P, TK_DOT, "Expected '.'");

        c->fields = arena_alloc(a, sizeof(Stmt*) * 50);
        c->field_private = arena_alloc(a, sizeof(bool) * 50);
        c->field_count = 0;
        c->methods = arena_alloc(a, sizeof(FunctionDecl*) * 50);
        c->method_count = 0;

        while (P_current(&P)->kind != TK_KW_END_CLASS) {
            if (P_current(&P)->kind == TK_EOF)
                bma_error("Unexpected EOF inside class '%s'", c->name);

            // static function
            if (P_is_word(&P, "static")) {
                P_adv(&P);
                if (P_current(&P)->kind != TK_KW_FUNCTION)
                    bma_error("'static' must be followed by 'function'");
                if (c->method_count >= 50) bma_error("Too many methods");
                c->methods[c->method_count++] = parse_method(&P, true);
                continue;
            }

            // regular function
            if (P_current(&P)->kind == TK_KW_FUNCTION) {
                if (c->method_count >= 50) bma_error("Too many methods");
                c->methods[c->method_count++] = parse_method(&P, false);
                continue;
            }

            // private prefix
            bool is_private = false;
            if (P_is_word(&P, "private")) {
                P_adv(&P);
                is_private = true;
            }

            // field
            Token *fname = P_expect(&P, TK_IDENT, "Expected field name");
            P_expect(&P, TK_EQUAL, "Expected '='");
            Token *ftype = P_current(&P); P_adv(&P);
            Stmt *fs = new_stmt(&P, ST_VAR_DECL);
            fs->var_decl.name = (char*)fname->text;
            fs->var_decl.type_name = (char*)ftype->text;
            P_expect(&P, TK_DOT, "Expected '.'");
            if (c->field_count >= 50) bma_error("Too many fields");
            c->fields[c->field_count] = fs;
            c->field_private[c->field_count] = is_private;
            c->field_count++;
        }
        P_expect(&P, TK_KW_END_CLASS, "Expected 'end_class'");
        P_expect(&P, TK_DOT, "Expected '.'");
        prog->classes[prog->class_count++] = c;
    }

    if (P_current(&P)->kind == TK_KW_CONST) {
        P_adv(&P);
        P_expect(&P, TK_COLON, "Expected ':'");
        do {
            Token *cname = P_expect(&P, TK_IDENT, "Expected const name");
            P_expect(&P, TK_EQUAL, "Expected '='");
            Expr *val = parse_expr(&P);
            Stmt *s = new_stmt(&P, ST_CONST_DECL);
            s->const_decl.name = (char*)cname->text;
            s->const_decl.value = val;
            prog->variables[prog->var_count++] = s;
        } while (P_match(&P, TK_COMMA));
        P_expect(&P, TK_DOT, "Expected '.'");
    }

    if (P_current(&P)->kind == TK_KW_VARIABLE)
        parse_var_block(&P, prog->variables, &prog->var_count);

    while (P_current(&P)->kind == TK_KW_FUNCTION) {
        FunctionDecl *f = parse_method(&P, false);
        prog->functions[prog->func_count++] = f;
    }

    P_expect(&P, TK_KW_CODE, "Expected 'code' block");
    P_expect(&P, TK_COLON, "Expected ':'");
    while (P_current(&P)->kind != TK_KW_END) {
        if (P_current(&P)->kind == TK_EOF) bma_error("Unexpected EOF inside code block");
        prog->statements[prog->stmt_count++] = parse_stmt(&P);
    }
    P_expect(&P, TK_KW_END, "Expected 'end'");
    P_expect(&P, TK_DOT, "Expected '.'");
    if (P_current(&P)->kind != TK_EOF) bma_error("Unexpected content after 'end .'");
    return prog;
}

Program *parse_library(Arena *a, TokenList *tokens) {
    Parser P = { tokens, 0, a };
    Program *prog = arena_alloc(a, sizeof(Program));
    prog->libraries = arena_alloc(a, sizeof(char*) * 20); prog->lib_count = 0;
    prog->classes = arena_alloc(a, sizeof(ClassDecl*) * 50); prog->class_count = 0;
    prog->variables = arena_alloc(a, sizeof(Stmt*) * 100); prog->var_count = 0;
    prog->functions = arena_alloc(a, sizeof(FunctionDecl*) * 500); prog->func_count = 0;
    prog->statements = arena_alloc(a, sizeof(Stmt*) * 100); prog->stmt_count = 0;

    P_expect(&P, TK_KW_BMA_LIB, "Expected BMA_Lib");
    P_expect(&P, TK_COLON, "Expected ':'");
    Token *name = P_expect(&P, TK_IDENT, "Expected library name");
    prog->name = (char*)name->text;
    P_expect(&P, TK_DOT, "Expected '.'");

    while (P_current(&P)->kind != TK_EOF) {
        Token *t = P_current(&P);
        if (t->kind == TK_IDENT && strcmp(t->text, "native") == 0) {
            while (P_current(&P)->kind != TK_DOT && P_current(&P)->kind != TK_EOF) P_adv(&P);
            if (P_current(&P)->kind == TK_DOT) P_adv(&P);
            continue;
        }
        if (t->kind == TK_IDENT && (strcmp(t->text, "extern_lib") == 0 || strcmp(t->text, "extern") == 0)) {
            while (P_current(&P)->kind != TK_DOT && P_current(&P)->kind != TK_EOF) P_adv(&P);
            if (P_current(&P)->kind == TK_DOT) P_adv(&P);
            continue;
        }
        if (t->kind == TK_IDENT && strcmp(t->text, "end_lib") == 0) {
            P_adv(&P);
            if (P_current(&P)->kind == TK_DOT) P_adv(&P);
            break;
        }
        if (t->kind == TK_KW_FUNCTION) {
            FunctionDecl *f = parse_method(&P, false);
            prog->functions[prog->func_count++] = f;
            continue;
        }
        P_adv(&P);
    }
    return prog;
}