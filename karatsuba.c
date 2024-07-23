#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils/myInt.h"
#include "utils/myString.h"

void karastuba_rec(BigNum a, BigNum b, BigNum* c, u64* mem_pool) {
    // `*c` so I avoid memory allocations. I use it as a buffer
    // mem_pool is other memory that i need
    assert(c->cap >= a.cap + b.cap);
    if (a.cap == 0 || b.cap == 0) {
        c->cap = 0;  // @todo i don't like this
        return;
    }

    if (a.cap <= 64 && b.cap <= 64) {
        c->ptr[0] = (*a.ptr & UINT32_MAX) * (*b.ptr & UINT32_MAX);
        c->ptr[1] = (*a.ptr >> 32) * (*b.ptr >> 32);
        u64 tmp = (*a.ptr & UINT32_MAX) * (*b.ptr >> 32) + (*b.ptr & UINT32_MAX) * (*a.ptr >> 32);
        u64 prev = c->ptr[0];
        c->ptr[0] += ((tmp & UINT32_MAX) << 32);
        u64 carry = c->ptr[0] < prev;
        c->ptr[1] += carry + (tmp >> 32);
        return;
    }

    u64 mid = 0;
    for (int i = 7; i < 60; i++) {  // mid_cap >= 64
        if ((1ull << i) >= a.cap && (1ull << i) >= b.cap) {
            mid = (1ull << (i - 1));
            break;
        }
    }
    assert(mid);

    // a = x y
    // b = z w
    // a * b =
    //          x    y *
    //          z    w =
    // -----------------
    //         xw   yz +
    //    xz   yz      =
    // -----------------
    // (xz << 2*mid) + (xw+yz << mid) + yz
    BigNum x, y, z, w;
    x.ptr = &a.ptr[mid / 64];
    x.cap = (a.cap > mid ? a.cap - mid : 0);
    y.ptr = &a.ptr[0];
    y.cap = (a.cap > mid ? mid : a.cap);
    z.ptr = &b.ptr[mid / 64];
    z.cap = (b.cap > mid ? b.cap - mid : 0);
    w.ptr = &b.ptr[0];
    w.cap = (b.cap > mid ? mid : b.cap);

    BigNum h1, h2;
    h1.cap = a.cap;
    h1.ptr = c->ptr;  // avoid malloc
    h2.cap = b.cap;
    h2.ptr = c->ptr + h1.cap / 64;  // avoid malloc
    bignum_sum(x, y, &h1);
    bignum_sum(z, w, &h2);

    BigNum xw_yz;
    xw_yz.ptr = mem_pool; // avoid malloc 2
    xw_yz.cap = h1.cap + h2.cap;
    assert(xw_yz.cap % 64 == 0);
    u64* next_pool = mem_pool + (h1.cap + h2.cap) / 64;
    karastuba_rec(h1, h2, &xw_yz, next_pool);

    BigNum xz, yw;
    yw.cap = y.cap + w.cap;
    yw.ptr = c->ptr;  // avoid malloc
    xz.cap = x.cap + z.cap;
    xz.ptr = c->ptr + mid / 32;  // avoid malloc

    karastuba_rec(x, z, &xz, next_pool);
    karastuba_rec(y, w, &yw, next_pool);
    bignum_sub_eq(&xw_yz, xz);
    bignum_sub_eq(&xw_yz, yw);

    u64 mid_2 = (mid * 2 < c->cap ? mid * 2 : c->cap);
    if (yw.cap < mid_2) memset(c->ptr + yw.cap / 64, 0, (mid_2 - yw.cap) / 8);
    if (xz.cap < c->cap - mid_2) memset(c->ptr + mid / 32, 0, (c->cap - mid_2 - xz.cap) / 8);

    BigNum tmp;
    tmp.cap = c->cap - mid;
    tmp.ptr = &c->ptr[mid / 64];
    bignum_sum_eq_uncheked(&tmp, xw_yz);

    return;
}

BigNum karastuba(BigNum a, BigNum b) {
    BigNum c = bignum_new(a.cap + b.cap);
    assert(c.cap % 64 == 0);
    u64 acap = 1, bcap = 1;
    u64 sz = c.cap;
    while (acap != a.cap && bcap != b.cap) {
        sz += acap + bcap;
        acap = (acap * 2 < a.cap ? acap * 2 : a.cap);
        bcap = (bcap * 2 < a.cap ? bcap * 2 : a.cap);
    }
    u64* mem_pool = malloc(sz / 8); // (c.cap / 8 * 6) is a good upper bound
    karastuba_rec(a, b, &c, mem_pool);
    free(mem_pool);
    return c;
}

int main() {
    BigNum a = bignum_read(stdin);
    BigNum b = bignum_read(stdin);
    BigNum c = karastuba(a, b);

    bignum_print(stdout, a);
    fprintf(stdout, "\n*\n");
    bignum_print(stdout, b);
    fprintf(stdout, "\n=\n");
    bignum_print(stdout, c);
    fprintf(stdout, "\n");

    bignum_free(a);
    bignum_free(b);
    bignum_free(c);
    return 0;
}
