// ═══════════════════════════════════════════════════════════════
// value.c — Runtime Values Implementation
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// ═══════════════════════════════════════════════════════════════

#include "value.h"

Value val_int(long long i) { Value v; v.type = V_INT; v.i = i; return v; }
Value val_real(double d) { Value v; v.type = V_REAL; v.d = d; return v; }
Value val_str(char *s) { Value v; v.type = V_STRING; v.s = s; return v; }
Value val_bool(bool b) { Value v; v.type = V_BOOL; v.b = b; return v; }
Value val_null(void) { Value v; v.type = V_NULL; v.i = 0; return v; }

Value val_array(Value *items, int count) {
    Value v;
    v.type = V_ARRAY;
    v.arr.items = items;
    v.arr.count = count;
    return v;
}

Value val_instance(Field *fields, int count, char *class_name) {
    Value v;
    v.type = V_INSTANCE;
    v.inst.fields = fields;
    v.inst.count = count;
    v.inst.class_name = class_name;
    return v;
}

void val_print(Value v) {
    switch (v.type) {
    case V_INT: printf("%lld", v.i); break;
    case V_REAL: printf("%g", v.d); break;
    case V_STRING: printf("%s", v.s); break;
    case V_BOOL: printf("%s", v.b ? "true" : "false"); break;
    case V_NULL: printf("null"); break;
    case V_ARRAY: {
        printf("[");
        for (int i = 0; i < v.arr.count; i++) {
            if (i > 0) printf(", ");
            val_print(v.arr.items[i]);
        }
        printf("]");
        break;
    }
    case V_INSTANCE: {
        printf("%s{", v.inst.class_name ? v.inst.class_name : "?");
        for (int i = 0; i < v.inst.count; i++) {
            if (i > 0) printf(", ");
            printf("%s: ", v.inst.fields[i].name);
            val_print(*v.inst.fields[i].val);
        }
        printf("}");
        break;
    }
    }
}