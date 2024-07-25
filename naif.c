#include <assert.h>
#include <sys/time.h>

#include "utils/myInt.h"
#include "utils/myString.h"

void bigint_sum_eq_shifted_unchecked(BigInt* c, BigInt* a, u64 shift) {
    if (!a->len) return;
    u64 offset = shift / 64;
    assert(offset + a->len + 1 <= c->cap);
    shift %= 64;
    u64* c_ptr = c->ptr + offset;
    shift %= 64;
    u64 carry = 0;
    for (u64 i = 0; i <= a->len; i++) {
        u64 res = c_ptr[i] + carry;
        if (i < a->len) res += (a->ptr[i] << shift);
        if (i && shift) res += a->ptr[i - 1] >> (64 - shift);
        carry = (res < c_ptr[i]) || (carry && c_ptr[i] == res);
        c_ptr[i] = res;
    }
    u64 i;
    for (i = a->len; carry; i++) {
        c_ptr[i]++;
        carry = (c_ptr[i] == 0);
    }
    u64 len2 = offset + (i - 1 > a->len + 1 ? i - 1 : a->len + 1);
    c->len = (len2 > c->len ? len2 : c->len);
}

BigInt mul2(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.len + b.len + 1);  // +1 to avoid OF
    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
    }
    for (u64 i = 0; i < b.len * 64; i++)
        if (bigint_is_set(b, i)) bigint_sum_eq_shifted_unchecked(&c, &a, i);
    return c;
}

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

    for (u64 i = c.len - 1;; i--) {
        if (c.ptr[i] != 0) {
            c.len = i + 1;
            break;
        } else if (i == 0) {
            c.len = 0;
            break;
        }
    }

    return c;
}
