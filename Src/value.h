// ═══════════════════════════════════════════════════════════════
// value.h — Runtime Values
// Copyright (c) 2025 Boutelis Mohamed El Amine
// Company: BMA SOFTWARE ALGERIA
// ═══════════════════════════════════════════════════════════════

#ifndef VALUE_H
#define VALUE_H

#include "bma.h"

typedef enum { V_INT, V_REAL, V_STRING, V_BOOL, V_NULL, V_ARRAY, V_INSTANCE } VType;

typedef struct Value Value;

typedef struct Field {
    char *name;
    Value *val;
} Field;

struct Value {
    VType type;
    union {
        long long i;
        double d;
        bool b;
        char *s;
        struct {
            Value *items;
            int count;
        } arr;
        struct {
            Field *fields;
            int count;
            char *class_name;   // ← NEW
        } inst;
    };
};

Value val_int(long long i);
Value val_real(double d);
Value val_str(char *s);
Value val_bool(bool b);
Value val_null(void);
Value val_array(Value *items, int count);
Value val_instance(Field *fields, int count, char *class_name);  // ← class_name added
void val_print(Value v);

#endif