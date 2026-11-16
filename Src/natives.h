// src/natives.h
#ifndef NATIVES_H
#define NATIVES_H

#include "value.h"

// تستدعي دالة مدمجة. إذا وجدت الدالة، تضع true في found وتعيد النتيجة.
Value native_call(const char* name, Value* args, int argc, bool* found);

#endif
