#include <assert.h>
#include <string.h>

#include "utils/myInt.h"

void karastuba_rec(BigInt a, BigInt b, BigInt* c) {
    // `*c` so I avoid memory allocations. I use it as a buffer
    assert(c->cap >= a.len + b.len);
    if (a.len == 0 || b.len == 0) {
        c->len = 0;
        return;
    }

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

    c->len = a.len + b.len;
    if (a.len <= 1 && b.len <= 1) {
        u64 x = *a.ptr >> 32;
        u64 y = *a.ptr & UINT32_MAX;
        u64 z = *b.ptr >> 32;
        u64 w = *b.ptr & UINT32_MAX;

        u64 xw_yz = x * w + y * z;
        u64 carry_2 = (x * w > xw_yz);
        c->ptr[0] = y * w;
        c->ptr[1] = x * z;

        u64 tmp = c->ptr[0];
        c->ptr[0] += xw_yz << 32;
        u64 carry = c->ptr[0] < tmp;
        c->ptr[1] += (xw_yz >> 32) + carry + (carry_2 << 32);
        return;
    }

    u64 mid = (a.len > b.len ? a.len / 2 : b.len / 2);

    BigInt x = {
        .ptr = &a.ptr[mid],
        .len = (a.len > mid ? a.len - mid : 0),
        .cap = (a.cap > mid ? a.cap - mid : 0),
    };
    BigInt y = {
        .ptr = &a.ptr[0],
        .len = (a.len > mid ? mid : a.len),
        .cap = (a.cap > mid ? mid : a.cap),
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
        .cap = (a.len < mid + 1 ? a.len : mid + 1),
    };
    BigInt z_w = {
        .ptr = c->ptr + x_y.cap,  // avoid malloc
        .len = 0,
        .cap = (b.len < mid + 1 ? b.len : mid + 1),
    };
    bigint_sum(x, y, &x_y);
    bigint_sum(z, w, &z_w);

    BigInt xw_yz = bigint_new(x_y.cap + z_w.cap);  // @todo can I avoid all this mallocs 
                                                   // (maybe doing a pool)?
    karastuba_rec(x_y, z_w, &xw_yz);

    BigInt yw = {
        .ptr = c->ptr,  // avoid malloc
        .len = 0,
        .cap = y.len + w.len,
    };
    BigInt xz = {
        .ptr = c->ptr + mid * 2,  // avoid malloc
        .len = 0,
        .cap = x.len + z.len,
    };

    karastuba_rec(x, z, &xz);
    karastuba_rec(y, w, &yw);

    bigint_sub_eq(&xw_yz, xz);
    bigint_sub_eq(&xw_yz, yw);

    if (yw.len < mid * 2) memset(c->ptr + yw.len, 0, (mid * 2 - yw.len) * 8);
    if (xz.len < c->len - mid * 2)
        memset(c->ptr + mid * 2 + xz.len, 0, (c->len - mid * 2 - xz.len) * 8);

    BigInt fake_num = {
        .ptr = &c->ptr[mid],
        .len = c->cap - mid,
        .cap = c->cap - mid,
    };
    bigint_sum_eq_uncheked(&fake_num, xw_yz);
    bigint_free(xw_yz);

    return;
}

BigInt karastuba(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.cap + b.cap);
    karastuba_rec(a, b, &c);
    return c;
}

int main() {
    BigInt a = bigint_read(stdin);
    BigInt b = bigint_read(stdin);
    BigInt c = karastuba(a, b);

    // bigint_print(stdout, a);
    // fprintf(stdout, "\n*\n");
    // bigint_print(stdout, b);
    // fprintf(stdout, "\n=\n");
    bigint_print(stdout, c);
    fprintf(stdout, "\n");

    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    return 0;
}
