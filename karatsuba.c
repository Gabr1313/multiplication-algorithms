#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils/myInt.h"

void karatsuba_rec(BigInt a, BigInt b, BigInt* c, u64* buffer) {
    // `*c` so I avoid memory allocations. I use it as a buffer

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

    assert(c->cap >= a.len + b.len);
    if (a.len == 0 || b.len == 0) {
        c->len = 0;
        return;
    }

    c->len = a.len + b.len;
    assert(c->cap >= c->len);

    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
    }

    if (a.len <= 32 && b.len <= 32) { // naif
        for (u64 i = 0; i < b.len * 2; i++) bigint_sum_eq_mul_u32(c, a, ((u32*)b.ptr)[i], i);
        bigint_clean(c);
        return;
    }

    u64 mid = a.len / 2;

    BigInt x = {
        .ptr = &a.ptr[mid],
        .len = a.len - mid,
        .cap = a.cap - mid,
    };
    BigInt y = {
        .ptr = &a.ptr[0],
        .len = mid,
        .cap = mid,
    };
    BigInt z = {
        .ptr = &b.ptr[mid],
        .len = (b.len > mid ? b.len - mid : 0),
        .cap = (b.cap > mid ? b.cap - mid : 0),
    };
    BigInt w = {
        .ptr = &b.ptr[0],
        .len = (b.len > mid ? mid : b.len),
        .cap = (b.cap > mid ? mid : b.cap),
    };

    BigInt x_y = {
        .ptr = c->ptr,  // avoid malloc
        .len = 0,
        .cap = mid + 1,
    };
    bigint_sum(x, y, &x_y);
    BigInt z_w = {
        .ptr = c->ptr + x_y.cap,  // avoid malloc
        .len = 0,
        .cap = (b.len < mid + 1 ? b.len : mid + 1),
    };
    bigint_sum(z, w, &z_w);

    BigInt xw_yz = {
        .ptr = buffer,  // avoid malloc
        .len = 0,
        .cap = x_y.cap + z_w.cap,
    };
    buffer += xw_yz.cap;

    karatsuba_rec(x_y, z_w, &xw_yz, buffer);

    memset(x_y.ptr, 0, x_y.len * 8);  // cleaning memory after usage
    memset(z_w.ptr, 0, z_w.len * 8);  // cleaning memory after usage

    BigInt yw = {
        .ptr = c->ptr,  // avoid malloc
        .len = 0,
        .cap = y.len + w.len,
    };
    karatsuba_rec(y, w, &yw, buffer);
    BigInt xz = {
        .ptr = c->ptr + mid * 2,  // avoid malloc
        .len = 0,
        .cap = x.len + z.len,
    };
    karatsuba_rec(x, z, &xz, buffer);

    bigint_sub_eq(&xw_yz, yw);
    bigint_sub_eq(&xw_yz, xz);

    BigInt fake_num = {
        .ptr = &c->ptr[mid],
        .len = c->cap - mid,
        .cap = c->cap - mid,
    };
    bigint_sum_eq_uncheked(&fake_num, xw_yz);

    memset(xw_yz.ptr, 0, xw_yz.len * 8);  // cleaning memory after usage: is this useless?

    bigint_clean(c);
    return;
}

BigInt mul(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.len + b.len);
    u64 buffer_size = 4, x = (a.len > b.len ? a.len : b.len);
    while (x > 2) buffer_size += (x = (x / 2 + 1)) * 2;
    u64* buffer = calloc(buffer_size, 8);
    karatsuba_rec(a, b, &c, buffer);
    free(buffer);
    return c;
}
