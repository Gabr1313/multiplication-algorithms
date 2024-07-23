#pragma once

#include "myType.h"
#include "myString.h"

typedef struct _bigNum {
    u64 cap;
    u64 *ptr;
    // bits are used as follow:
    // - the least significant is in the rightmost bit of the first element
    // - the most significant is in the leftmost bit of the last element
} BigNum;

void bignum_free(BigNum n);

BigNum bignum_clone(BigNum n);

String bignum_to_string(BigNum n);

void bignum_print(FILE* stream, BigNum x);

BigNum bignum_new(u64 bits);

// does not realloc
void bignum_set(BigNum *n, u64 pos, u64 val);

void bignum_shrink(BigNum *n, u64 cap);

BigNum bignum_read(FILE* stream);

void bignum_cpy_increase_cap(BigNum a, u64 cap);

void bignum_sub_eq(const BigNum *const a, BigNum b);

void bignum_sum_eq_uncheked(BigNum *a, BigNum b);

void bignum_sum(BigNum a, BigNum b, BigNum *c);
