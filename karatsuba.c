#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/myInt.h"

void karastuba_rec(BigInt a, BigInt b, BigInt* c, u64* buffer) {
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
    assert(c->cap >= c->len);
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

    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
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

    karastuba_rec(x_y, z_w, &xw_yz, buffer);

    memset(x_y.ptr, 0, x_y.len * 8); // cleaning memory after usage
    memset(z_w.ptr, 0, z_w.len * 8); // cleaning memory after usage

    BigInt yw = {
        .ptr = c->ptr,  // avoid malloc
        .len = 0,
        .cap = y.len + w.len,
    };
    karastuba_rec(y, w, &yw, buffer);
    BigInt xz = {
        .ptr = c->ptr + mid * 2,  // avoid malloc
        .len = 0,
        .cap = x.len + z.len,
    };
    karastuba_rec(x, z, &xz, buffer);

    bigint_sub_eq(&xw_yz, yw);
    bigint_sub_eq(&xw_yz, xz);

    BigInt fake_num = {
        .ptr = &c->ptr[mid],
        .len = c->cap - mid,
        .cap = c->cap - mid,
    };
    bigint_sum_eq_uncheked(&fake_num, xw_yz);

    memset(xw_yz.ptr, 0, xw_yz.len * 8); // cleaning memory after usage: is this useless?

    for (u64 i = c->len - 1;; i--) {
        if (c->ptr[i] != 0) {
            c->len = i + 1;
            break;
        } else if (i == 0) {
            c->len = 0;
            break;
        }
    }

    return;
}

BigInt karastuba(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.cap + b.cap);
    u64 buffer_size = 4, x = (a.cap > b.cap ? a.cap : b.cap);
    while (x > 2) buffer_size += (x = (x / 2 + 1)) * 2;
    u64* buffer = calloc(buffer_size, 8);
    karastuba_rec(a, b, &c, buffer);
    free(buffer);
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
