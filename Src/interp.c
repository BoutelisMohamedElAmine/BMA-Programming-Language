// ═══════════════════════════════════════════════════════════════
// interp.c — Tree-Walker Interpreter (Full OOP, new = constructor)
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// ═══════════════════════════════════════════════════════════════

#include "bma.h"
#include "ast.h"
#include "value.h"
#include "natives.h"

#define MAX_VARS 1000
enum { EXEC_NORMAL, EXEC_BREAK, EXEC_CONTINUE, EXEC_RETURN };

typedef struct { char *name; Value val; bool is_const; } Var;
typedef struct { Var vars[MAX_VARS]; int var_count; } Env;

static Arena *g_arena = NULL;
static Program *g_prog = NULL;
static Value g_return_value;
static ClassDecl *g_current_class = NULL;

static Value eval_expr(Env *env, Expr *e);
static int exec_stmt(Env *env, Stmt *s);

static Value *env_get(Env *env, const char *name) {
    for (int i = 0; i < env->var_count; i++)
        if (strcmp(env->vars[i].name, name) == 0) return &env->vars[i].val;
    bma_error("Undefined variable '%s'", name);
    return NULL;
}

static Value *env_try_get(Env *env, const char *name) {
    for (int i = 0; i < env->var_count; i++)
        if (strcmp(env->vars[i].name, name) == 0) return &env->vars[i].val;
    return NULL;
}

static void env_declare(Env *env, const char *name, Value v, bool is_const) {
    for (int i = 0; i < env->var_count; i++)
        if (strcmp(env->vars[i].name, name) == 0)
            bma_error("Variable '%s' already declared", name);
    if (env->var_count >= MAX_VARS) bma_error("Too many variables");
    env->vars[env->var_count].name = (char*)name;
    env->vars[env->var_count].val = v;
    env->vars[env->var_count].is_const = is_const;
    env->var_count++;
}

static void env_set(Env *env, const char *name, Value v) {
    for (int i = 0; i < env->var_count; i++) {
        if (strcmp(env->vars[i].name, name) == 0) {
            if (env->vars[i].is_const) bma_error("Cannot assign to constant '%s'", name);
            env->vars[i].val = v;
            return;
        }
    }
    bma_error("Undefined variable '%s'", name);
}

static ClassDecl *find_class(const char *name) {
    for (int i = 0; i < g_prog->class_count; i++)
        if (strcmp(g_prog->classes[i]->name, name) == 0) return g_prog->classes[i];
    return NULL;
}

static Value default_val(const char *type) {
    if (strcmp(type, "integer") == 0) return val_int(0);
    if (strcmp(type, "real") == 0) return val_real(0.0);
    if (strcmp(type, "boolean") == 0) return val_bool(false);
    if (strcmp(type, "string") == 0) return val_str("");
    return val_null();
}

static bool is_same_or_subclass(ClassDecl *current, ClassDecl *target) {
    while (current) {
        if (current == target) return true;
        if (!current->parent_name) return false;
        current = find_class(current->parent_name);
    }
    return false;
}

static ClassDecl *find_field_owner(ClassDecl *cls, const char *field_name) {
    while (cls) {
        for (int i = 0; i < cls->field_count; i++)
            if (strcmp(cls->fields[i]->var_decl.name, field_name) == 0) return cls;
        if (!cls->parent_name) return NULL;
        cls = find_class(cls->parent_name);
    }
    return NULL;
}

static bool is_field_private(ClassDecl *owner, const char *field_name) {
    for (int i = 0; i < owner->field_count; i++)
        if (strcmp(owner->fields[i]->var_decl.name, field_name) == 0)
            return owner->field_private[i];
    return false;
}

static FunctionDecl *find_method(ClassDecl *cls, const char *name, int argc) {
    while (cls) {
        for (int i = 0; i < cls->method_count; i++) {
            if (strcmp(cls->methods[i]->name, name) == 0 &&
                cls->methods[i]->param_count == argc) {
                return cls->methods[i];
            }
        }
        if (!cls->parent_name) return NULL;
        cls = find_class(cls->parent_name);
    }
    return NULL;
}

static void collect_fields(ClassDecl *cls, Field *out, int *count) {
    if (cls->parent_name) {
        ClassDecl *parent = find_class(cls->parent_name);
        if (parent) collect_fields(parent, out, count);
    }
    for (int i = 0; i < cls->field_count; i++) {
        Stmt *f = cls->fields[i];
        out[*count].name = f->var_decl.name;
        out[*count].val = arena_alloc(g_arena, sizeof(Value));
        ClassDecl *nested = find_class(f->var_decl.type_name);
        if (nested) *out[*count].val = default_val("void");
        else        *out[*count].val = default_val(f->var_decl.type_name);
        (*count)++;
    }
}

static Value create_instance(ClassDecl *cls);

static Value create_instance(ClassDecl *cls) {
    Field *fields = arena_alloc(g_arena, sizeof(Field) * 200);
    int count = 0;
    if (cls->parent_name) {
        ClassDecl *parent = find_class(cls->parent_name);
        if (parent) collect_fields(parent, fields, &count);
    }
    for (int i = 0; i < cls->field_count; i++) {
        Stmt *f = cls->fields[i];
        fields[count].name = f->var_decl.name;
        fields[count].val = arena_alloc(g_arena, sizeof(Value));
        ClassDecl *nested = find_class(f->var_decl.type_name);
        if (nested) *fields[count].val = create_instance(nested);
        else        *fields[count].val = default_val(f->var_decl.type_name);
        count++;
    }
    return val_instance(fields, count, cls->name);
}

static char *value_to_str(Value v) {
    char *buf = NULL;
    switch (v.type) {
    case V_STRING: return v.s;
    case V_INT: { buf = arena_alloc(g_arena, 32); snprintf(buf, 32, "%lld", v.i); return buf; }
    case V_REAL: { buf = arena_alloc(g_arena, 64); snprintf(buf, 64, "%g", v.d); return buf; }
    case V_BOOL: return v.b ? "true" : "false";
    case V_NULL: return "null";
    default: return "<value>";
    }
}

static Value eval_binary(Value l, Value r, const char *op) {
    if (l.type == V_STRING || r.type == V_STRING) {
        if (strcmp(op, "+") == 0) {
            const char *ls = value_to_str(l);
            const char *rs = value_to_str(r);
            char *buf = arena_alloc(g_arena, strlen(ls) + strlen(rs) + 1);
            strcpy(buf, ls); strcat(buf, rs);
            return val_str(buf);
        }
        if (strcmp(op, "==") == 0 || strcmp(op, "<>") == 0) {
            if (l.type != V_STRING || r.type != V_STRING)
                bma_error("Cannot compare string with non-string using '%s'", op);
            bool eq = strcmp(l.s, r.s) == 0;
            return val_bool(strcmp(op, "==") == 0 ? eq : !eq);
        }
        bma_error("Unsupported string op '%s'", op);
    }
    if (l.type == V_REAL || r.type == V_REAL || strcmp(op, "/") == 0) {
        double a = (l.type == V_REAL) ? l.d : (double)l.i;
        double b = (r.type == V_REAL) ? r.d : (double)r.i;
        if (strcmp(op, "+") == 0) return val_real(a + b);
        if (strcmp(op, "-") == 0) return val_real(a - b);
        if (strcmp(op, "*") == 0) return val_real(a * b);
        if (strcmp(op, "/") == 0) { if (b==0) bma_error("Division by zero"); return val_real(a / b); }
        if (strcmp(op, "==") == 0) return val_bool(a == b);
        if (strcmp(op, "<>") == 0) return val_bool(a != b);
        if (strcmp(op, "<") == 0) return val_bool(a < b);
        if (strcmp(op, ">") == 0) return val_bool(a > b);
        if (strcmp(op, "<=") == 0) return val_bool(a <= b);
        if (strcmp(op, ">=") == 0) return val_bool(a >= b);
        bma_error("Unsupported real op '%s'", op);
    }
    long long a = l.i, b = r.i;
    if (strcmp(op, "+") == 0) return val_int(a + b);
    if (strcmp(op, "-") == 0) return val_int(a - b);
    if (strcmp(op, "*") == 0) return val_int(a * b);
    if (strcmp(op, "div") == 0) { if (b==0) bma_error("Division by zero"); return val_int(a / b); }
    if (strcmp(op, "mod") == 0) { if (b==0) bma_error("Division by zero"); return val_int(a % b); }
    if (strcmp(op, "==") == 0) return val_bool(a == b);
    if (strcmp(op, "<>") == 0) return val_bool(a != b);
    if (strcmp(op, "<") == 0) return val_bool(a < b);
    if (strcmp(op, ">") == 0) return val_bool(a > b);
    if (strcmp(op, "<=") == 0) return val_bool(a <= b);
    if (strcmp(op, ">=") == 0) return val_bool(a >= b);
    if (strcmp(op, "and") == 0) return val_bool(a && b);
    if (strcmp(op, "or") == 0) return val_bool(a || b);
    bma_error("Unsupported int op '%s'", op);
    return val_null();
}

static Value call_user_func(FunctionDecl *f, Value *args, int argc) {
    if (argc != f->param_count)
        bma_error("Function '%s' expects %d args, got %d", f->name, f->param_count, argc);
    Env local_env; local_env.var_count = 0;
    for (int i = 0; i < f->param_count; i++)
        env_declare(&local_env, f->params[i].name, args[i], false);
    for (int i = 0; i < f->body_count; i++) {
        int code = exec_stmt(&local_env, f->body[i]);
        if (code == EXEC_RETURN) return g_return_value;
    }
    return val_null();
}

static Value call_method(Value instance, ClassDecl *cls, FunctionDecl *f,
                         Value *args, int argc) {
    if (argc != f->param_count)
        bma_error("Method '%s.%s' expects %d args, got %d",
                  cls->name, f->name, f->param_count, argc);
    Env local_env; local_env.var_count = 0;
    env_declare(&local_env, "self", instance, false);
    for (int i = 0; i < f->param_count; i++)
        env_declare(&local_env, f->params[i].name, args[i], false);

    ClassDecl *saved = g_current_class;
    g_current_class = cls;

    Value result = val_null();
    for (int i = 0; i < f->body_count; i++) {
        int code = exec_stmt(&local_env, f->body[i]);
        if (code == EXEC_RETURN) { result = g_return_value; break; }
    }

    g_current_class = saved;
    return result;
}

static Value call_static_method(ClassDecl *cls, FunctionDecl *f,
                                Value *args, int argc) {
    if (argc != f->param_count)
        bma_error("Static method '%s.%s' expects %d args, got %d",
                  cls->name, f->name, f->param_count, argc);
    Env local_env; local_env.var_count = 0;
    for (int i = 0; i < f->param_count; i++)
        env_declare(&local_env, f->params[i].name, args[i], false);

    ClassDecl *saved = g_current_class;
    g_current_class = cls;

    Value result = val_null();
    for (int i = 0; i < f->body_count; i++) {
        int code = exec_stmt(&local_env, f->body[i]);
        if (code == EXEC_RETURN) { result = g_return_value; break; }
    }

    g_current_class = saved;
    return result;
}

static Value construct_instance(ClassDecl *cls, Value *args, int argc) {
    Value inst = create_instance(cls);
    FunctionDecl *init = find_method(cls, "init", argc);
    if (init) {
        call_method(inst, cls, init, args, argc);
    } else if (argc > 0) {
        bma_error("Class '%s' has no constructor 'init' with %d arguments",
                  cls->name, argc);
    }
    return inst;
}

static Value eval_expr(Env *env, Expr *e) {
    switch (e->kind) {
    case EX_INT: return val_int(e->int_val);
    case EX_REAL: return val_real(e->real_val);
    case EX_STRING: return val_str(e->str_val);
    case EX_BOOL: return val_bool(e->bool_val);
    case EX_IDENT: return *env_get(env, e->ident_name);

    case EX_ARRAY_LITERAL: {
        Value *items = arena_alloc(g_arena, sizeof(Value) * e->array_literal.count);
        for (int i = 0; i < e->array_literal.count; i++)
            items[i] = eval_expr(env, e->array_literal.items[i]);
        return val_array(items, e->array_literal.count);
    }

    case EX_INDEX: {
        Value target = eval_expr(env, e->index.target);
        if (target.type != V_ARRAY) bma_error("Cannot index non-array");
        Value idx = eval_expr(env, e->index.index);
        if (idx.type != V_INT) bma_error("Index must be integer");
        if (idx.i < 0 || idx.i >= target.arr.count) bma_error("Index out of bounds");
        return target.arr.items[idx.i];
    }

    case EX_MEMBER: {
        Value target = eval_expr(env, e->member.target);
        if (target.type != V_INSTANCE) bma_error("Cannot access member of non-instance");
        ClassDecl *cls = find_class(target.inst.class_name);
        ClassDecl *owner = find_field_owner(cls, e->member.member);
        if (!owner) bma_error("Class '%s' has no field '%s'",
                              target.inst.class_name, e->member.member);
        if (is_field_private(owner, e->member.member)) {
            if (!is_same_or_subclass(g_current_class, owner))
                bma_error("Cannot access private field '%s.%s' from here",
                          owner->name, e->member.member);
        }
        for (int i = 0; i < target.inst.count; i++)
            if (strcmp(target.inst.fields[i].name, e->member.member) == 0)
                return *target.inst.fields[i].val;
        bma_error("Internal: field not found");
        return val_null();
    }

    case EX_BINARY: {
        Value l = eval_expr(env, e->binary.left);
        Value r = eval_expr(env, e->binary.right);
        return eval_binary(l, r, e->binary.op);
    }

    case EX_UNARY: {
        Value v = eval_expr(env, e->unary.operand);
        if (strcmp(e->unary.op, "-") == 0) {
            if (v.type == V_INT) return val_int(-v.i);
            if (v.type == V_REAL) return val_real(-v.d);
            bma_error("Cannot negate non-numeric");
        }
        if (strcmp(e->unary.op, "not") == 0) {
            if (v.type == V_BOOL) return val_bool(!v.b);
            bma_error("Cannot 'not' non-boolean");
        }
        bma_error("Unknown unary op");
        return val_null();
    }

    case EX_CALL: {
        Value args[32];
        for (int i = 0; i < e->call.arg_count; i++)
            args[i] = eval_expr(env, e->call.args[i]);

        if (e->call.method_target != NULL) {
            bool is_constructor = (strcmp(e->call.name, "new") == 0);

            if (e->call.method_target->kind == EX_IDENT) {
                const char *name = e->call.method_target->ident_name;

                // ═══ Priority 1: If calling .new() → ALWAYS use class lookup ═══
                if (is_constructor) {
                    ClassDecl *cls = find_class(name);
                    if (!cls)
                        bma_error("'%s.new()': class '%s' not found", name, name);
                    return construct_instance(cls, args, e->call.arg_count);
                }

                // ═══ Priority 2: Variable lookup (instance method) ═══
                Value *var = env_try_get(env, name);
                if (var != NULL && var->type == V_INSTANCE) {
                    Value target = *var;
                    ClassDecl *cls = find_class(target.inst.class_name);
                    if (!cls) bma_error("Unknown class '%s'", target.inst.class_name);
                    FunctionDecl *m = find_method(cls, e->call.name, e->call.arg_count);
                    if (!m) bma_error("Class '%s' has no method '%s' with %d args",
                                      cls->name, e->call.name, e->call.arg_count);
                    if (m->is_static)
                        bma_error("Method '%s.%s' is static — call via class name",
                                  cls->name, e->call.name);
                    return call_method(target, cls, m, args, e->call.arg_count);
                }

                // ═══ Priority 3: Static method (class lookup) ═══
                ClassDecl *cls = find_class(name);
                if (cls) {
                    FunctionDecl *m = find_method(cls, e->call.name, e->call.arg_count);
                    if (!m) bma_error("Class '%s' has no static method '%s' with %d args",
                                      cls->name, e->call.name, e->call.arg_count);
                    if (!m->is_static)
                        bma_error("Method '%s.%s' is not static — use an instance",
                                  cls->name, e->call.name);
                    return call_static_method(cls, m, args, e->call.arg_count);
                }
            }

            // ═══ Fallback: evaluate expression (e.g., (expr).method()) ═══
            Value target = eval_expr(env, e->call.method_target);
            if (target.type != V_INSTANCE)
                bma_error("Cannot call method '%s' on non-instance", e->call.name);
            ClassDecl *cls = find_class(target.inst.class_name);
            if (!cls) bma_error("Unknown class '%s'", target.inst.class_name);
            FunctionDecl *m = find_method(cls, e->call.name, e->call.arg_count);
            if (!m) bma_error("Class '%s' has no method '%s' with %d args",
                              cls->name, e->call.name, e->call.arg_count);
            if (m->is_static)
                bma_error("Method '%s.%s' is static — call via class name",
                          cls->name, e->call.name);
            return call_method(target, cls, m, args, e->call.arg_count);
        }

        for (int i = 0; i < g_prog->func_count; i++)
            if (strcmp(g_prog->functions[i]->name, e->call.name) == 0)
                return call_user_func(g_prog->functions[i], args, e->call.arg_count);

        bool found = false;
        Value res = native_call(e->call.name, args, e->call.arg_count, &found);
        if (found) return res;

        bma_error("Unknown function '%s'", e->call.name);
    }

    default: bma_error("Unsupported expression kind %d", e->kind);
    }
    return val_null();
}

static int exec_stmt(Env *env, Stmt *s) {
    switch (s->kind) {
    case ST_VAR_DECL: {
        ClassDecl *cls = find_class(s->var_decl.type_name);
        if (cls) {
            env_declare(env, s->var_decl.name, create_instance(cls), false);
            break;
        }
        if (s->var_decl.array_size != NULL) {
            Value sz = eval_expr(env, s->var_decl.array_size);
            if (sz.type != V_INT || sz.i < 0) bma_error("Invalid array size");
            int count = (int)sz.i;
            Value *items = arena_alloc(g_arena, sizeof(Value) * (count == 0 ? 1 : count));
            Value def = default_val(s->var_decl.type_name);
            for (int i = 0; i < count; i++) items[i] = def;
            env_declare(env, s->var_decl.name, val_array(items, count), false);
        } else {
            env_declare(env, s->var_decl.name, default_val(s->var_decl.type_name), false);
        }
        break;
    }

    case ST_CONST_DECL:
        env_declare(env, s->const_decl.name, eval_expr(env, s->const_decl.value), true);
        break;

    case ST_INDEX_ASSIGN: {
        Value *arr = env_get(env, s->index_assign.name);
        if (arr->type != V_ARRAY) bma_error("Cannot index non-array");
        Value idx = eval_expr(env, s->index_assign.index);
        if (idx.type != V_INT) bma_error("Index must be integer");
        if (idx.i < 0 || idx.i >= arr->arr.count) bma_error("Index out of bounds");
        arr->arr.items[idx.i] = eval_expr(env, s->index_assign.value);
        break;
    }

    case ST_MEMBER_ASSIGN: {
        Value target = eval_expr(env, s->member_assign.target->member.target);
        if (target.type != V_INSTANCE) bma_error("Cannot assign member of non-instance");
        ClassDecl *cls = find_class(target.inst.class_name);
        ClassDecl *owner = find_field_owner(cls, s->member_assign.target->member.member);
        if (!owner) bma_error("Class '%s' has no field '%s'",
                              target.inst.class_name, s->member_assign.target->member.member);
        if (is_field_private(owner, s->member_assign.target->member.member)) {
            if (!is_same_or_subclass(g_current_class, owner))
                bma_error("Cannot access private field '%s.%s' from here",
                          owner->name, s->member_assign.target->member.member);
        }
        for (int i = 0; i < target.inst.count; i++) {
            if (strcmp(target.inst.fields[i].name,
                       s->member_assign.target->member.member) == 0) {
                *target.inst.fields[i].val = eval_expr(env, s->member_assign.value);
                break;
            }
        }
        break;
    }

    case ST_ASSIGN:
        env_set(env, s->assign.name, eval_expr(env, s->assign.value));
        break;

    case ST_EXPR:
        eval_expr(env, s->expr_stmt.expr);
        break;

    case ST_IF: {
        Value c = eval_expr(env, s->if_stmt.cond);
        if (c.type != V_BOOL) bma_error("if condition must be boolean");
        if (c.b) {
            for (int i = 0; i < s->if_stmt.then_count; i++) {
                int r = exec_stmt(env, s->if_stmt.then_stmts[i]);
                if (r) return r;
            }
        } else {
            for (int i = 0; i < s->if_stmt.else_count; i++) {
                int r = exec_stmt(env, s->if_stmt.else_stmts[i]);
                if (r) return r;
            }
        }
        break;
    }

    case ST_WHILE: {
        for (;;) {
            Value c = eval_expr(env, s->while_stmt.cond);
            if (c.type != V_BOOL) bma_error("while condition must be boolean");
            if (!c.b) break;
            for (int i = 0; i < s->while_stmt.body_count; i++) {
                int r = exec_stmt(env, s->while_stmt.body[i]);
                if (r == EXEC_BREAK) goto w_done;
                if (r == EXEC_RETURN) return r;
            }
        }
        w_done: break;
    }

    case ST_FOR: {
        Value start = eval_expr(env, s->for_stmt.start);
        Value end = eval_expr(env, s->for_stmt.end);
        Value step = s->for_stmt.step ? eval_expr(env, s->for_stmt.step) : val_int(1);
        if (start.type != V_INT || end.type != V_INT || step.type != V_INT)
            bma_error("for requires integers");
        env_set(env, s->for_stmt.var, start);
        long long st = step.i;
        if (st == 0) bma_error("for step cannot be zero");
        for (;;) {
            Value *cur = env_get(env, s->for_stmt.var);
            if (st > 0 ? (cur->i > end.i) : (cur->i < end.i)) break;
            for (int i = 0; i < s->for_stmt.body_count; i++) {
                int r = exec_stmt(env, s->for_stmt.body[i]);
                if (r == EXEC_BREAK) goto f_done;
                if (r == EXEC_RETURN) return r;
            }
            env_get(env, s->for_stmt.var)->i += st;
        }
        f_done: break;
    }

    case ST_RETURN:
        if (s->return_stmt.value) g_return_value = eval_expr(env, s->return_stmt.value);
        else g_return_value = val_null();
        return EXEC_RETURN;

    case ST_BREAK: return EXEC_BREAK;
    case ST_CONTINUE: return EXEC_CONTINUE;

    case ST_READ: {
        char buf[1024];
        if (scanf("%1023s", buf) != 1) bma_error("read failed");
        Value *v = env_get(env, s->read_stmt.var);
        if (v->type == V_INT) v->i = atoll(buf);
        else if (v->type == V_REAL) v->d = atof(buf);
        else if (v->type == V_STRING) v->s = arena_strdup(g_arena, buf);
        break;
    }

    case ST_READLINE: {
        char buf[1024];
        if (fgets(buf, sizeof(buf), stdin) == NULL) bma_error("readLine failed");
        buf[strcspn(buf, "\r\n")] = 0;
        if (s->readline_stmt.var != NULL) {
            Value *v = env_get(env, s->readline_stmt.var);
            if (v->type == V_INT) v->i = atoll(buf);
            else if (v->type == V_REAL) v->d = atof(buf);
            else if (v->type == V_STRING) v->s = arena_strdup(g_arena, buf);
        }
        break;
    }

    default: bma_error("Unsupported statement kind %d", s->kind);
    }
    return EXEC_NORMAL;
}

void interp_run(Program *prog, Arena *a) {
    g_arena = a;
    g_prog = prog;
    Env env; env.var_count = 0;
    for (int i = 0; i < prog->var_count; i++) exec_stmt(&env, prog->variables[i]);
    for (int i = 0; i < prog->stmt_count; i++) {
        if (exec_stmt(&env, prog->statements[i]) != EXEC_NORMAL)
            bma_error("break/continue outside loop");
    }
}