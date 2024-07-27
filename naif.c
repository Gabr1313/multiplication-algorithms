#include <assert.h>
#include <sys/time.h>

#include "utils/myInt.h"
#include "utils/myString.h"

void bigint_sum_eq_mul_u32(BigInt* c, BigInt a, u32 b, u64 u32_shift) {
    u64 carry = 0;
    u64 to_mul = b;
    for (u64 i = 0; i < a.len * 2; i++) {
        u64 n = ((u32*)a.ptr)[i] * to_mul;
        u32 res = ((u32*)c->ptr)[i + u32_shift] + (n & UINT32_MAX);
        carry += (res < ((u32*)c->ptr)[i + u32_shift]) ||
                 (res == ((u32*)c->ptr)[i + u32_shift] && carry);
        ((u32*)c->ptr)[i + u32_shift] = res;
        if (i + u32_shift + 1 < c->cap * 2) {  // carry can be 2, but it is ok
            res = ((u32*)c->ptr)[i + u32_shift + 1] + (n >> 32) + carry;
            carry = (res < ((u32*)c->ptr)[i + u32_shift + 1]) ||
                    (res == ((u32*)c->ptr)[i + u32_shift + 1] && carry);
            ((u32*)c->ptr)[i + u32_shift + 1] = res;
        } else assert((n >> 32) + carry == 0);
    }
}

BigInt mul(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.len + b.len);
    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
    }
    for (u64 i = 0; i < b.len * 2; i++) bigint_sum_eq_mul_u32(&c, a, ((u32*)b.ptr)[i], i);
    bigint_clean(&c);
    return c;
}
