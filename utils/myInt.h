#pragma once

#include "myType.h"
#include "myString.h"

typedef struct _BigInt {
    u64 cap, len; // cap and len should be multiplied by 64
    u64 *ptr;
    // bits are used as follow:
    // - the least significant is in the rightmost bit of the first element
    // - the most significant is in the leftmost bit of the last element
} BigInt;

void bigint_free(BigInt n);
BigInt bigint_new(u64 u64s);
BigInt bigint_clone(BigInt n);
void bigint_shrink(BigInt *n);
void bigint_set(BigInt *n, u64 pos, u64 val);
u64 bigint_is_set(BigInt n, u64 pos);
void bigint_sub_eq(BigInt *a, BigInt b);
void bigint_sum_eq_uncheked(BigInt *a, BigInt b);
void bigint_sum(BigInt a, BigInt b, BigInt *c);
String bigint_to_string(BigInt n);
String bigint_to_string_hex(BigInt n);
String bigint_to_string_hex_dbg(BigInt n);
BigInt bigint_read(FILE *stream);
BigInt bigint_read_hex(FILE *stream);
void bigint_print(FILE *stream, BigInt x);
void bigint_print_hex(FILE *stream, BigInt x);
