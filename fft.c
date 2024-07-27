#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>

#include "utils/myInt.h"

#define PI 3.141592653589793238462643383279502884
#define PRC 6 // precision: 2 to 6

typedef complex double cpx;

void fft(cpx* a, u64 a_size, u64 invert) {
    u64 n = a_size;

    for (u64 i = 1, j = 0; i < n; i++) {
        u64 bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;

        if (i < j) {
            cpx tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
        }
    }

    for (u64 len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cpx wlen = cos(ang) + I * sin(ang);
        for (u64 i = 0; i < n; i += len) {
            cpx w = 1;
            for (u64 j = 0; j < len / 2; j++) {
                cpx u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert)
        for (u64 i = 0; i < a_size; i++) a[i] /= n;
}

BigInt mul(BigInt a, BigInt b) {
    u64 n;
    for (n = 1 << PRC; (n >> PRC) < a.len + b.len; n <<= 1);

    u64 mask = ((1ull << (1 << (6 - PRC))) - 1);
    cpx* fa = malloc(sizeof(cpx) * n);  // * 2 because i use u32
    cpx* fb = malloc(sizeof(cpx) * n);  // * 2 because i use u32
    for (u64 i = 0; i < n; i++) {
        u64 shf = ((i & ((1ull << PRC) - 1)) << (6 - PRC));
        if (i < a.len << PRC) fa[i] = (a.ptr[i >> PRC] >> shf) & mask;
        else fa[i] = 0;
        if (i < b.len << PRC) fb[i] = (b.ptr[i >> PRC] >> shf) & mask;
        else fb[i] = 0;
    }

    fft(fa, n, 0);
    fft(fb, n, 0);
    for (u64 i = 0; i < n; i++) fa[i] *= fb[i];
    fft(fa, n, 1);

    BigInt c = bigint_new(a.len + b.len);
    c.len = c.cap;

    u64 carry = 0;
    for (u64 i = 0; i < c.len << PRC; i++) {
        u64 res = (u64)round(creal(fa[i])) + carry;
        carry = res >> (1 << (6 - PRC));
        u64 shf = ((i & ((1ull << PRC) - 1)) << (6 - PRC));
        c.ptr[i >> PRC] += (res & mask) << shf;
    }

    free(fa);
    free(fb);

    bigint_clean(&c);
    return c;
}
