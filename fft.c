#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>

#include "utils/myInt.h"

#define PI 3.141592653589793238462643383279502884
#define PRC 4  // precision: 2 to 6

typedef complex double cpx;

u32* pre_calc_pos(u64 n) {
    assert(n < 1ull << 32);  // how much memory would i ever need?
    u32* res = malloc(sizeof(*res) * n);
    for (u64 i = 1, j = 0; i < n; i++) {
        u32 bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        res[i] = j;
    }
    return res;
}

cpx* pre_calc_ang(u64 n) {  // @todo i could get almost 1/4 of the memory using symmetry
                            // i am usign 1st and 2nd quadrant, I could use only 1/2 of the 1st
    u64 cnt = 0;
    for (u64 len = 2; len <= n; len <<= 1) cnt += len / 2;
    cpx* res = malloc(sizeof(*res) * cnt);
    for (u64 len = 2, idx = 0; len <= n; len <<= 1) {
        double ang = 2 * PI / len;
        cpx z = cos(ang) + I * sin(ang);
        cpx w = 1;
        for (u64 j = 0; j < len / 2; j++, idx++) {
            res[idx] = w;
            fprintf(stderr, "(%f %f)", creal(w), cimag(w));
            w *= z;
        }
        fprintf(stderr, "\n");
    }

    /* for (u64 len = 2, idx = 0; len <= n; len <<= 1) {
        double ang = 2 * PI / len;
        cpx z = cos(ang) + I * sin(ang);
        cpx w = 1;
        for (u64 j = 0; j < len / 8; j++, idx++) {
            fprintf(stderr, "(%f %f)", creal(w), cimag(w));
            w *= z;
        }
        for (u64 j = len / 8; j < len / 4; j++, idx++) {
            fprintf(stderr, "(%f %f)", cimag(w), creal(w));
            w /= z;
        }
        w = I;
        for (u64 j = len / 4; j < len * 3 / 8; j++, idx++) {
            fprintf(stderr, "(%f %f)", -creal(w), cimag(w));
            w /= z;
        }
        for (u64 j = len * 3 / 8; j < len / 2; j++, idx++) {
            fprintf(stderr, "(%f %f)", -creal(w), cimag(w));
            w /= z;
        }
        fprintf(stderr, "\n");
    } */

    return res;
}

void fft(cpx* a, u64 n, u64 invert, u32* pos, cpx* ang) {
    for (u64 i = 1; i < n; i++) {
        if (i < pos[i]) {
            cpx tmp = a[i];
            a[i] = a[pos[i]];
            a[pos[i]] = tmp;
        }
    }

    if (invert)
        for (u64 len = 2, i = 0; len <= n; len <<= 1)
            for (u64 j = 0; j < len / 2; j++, i++) ang[i] = conj(ang[i]);

    cpx* ptr = ang;
    for (u64 len = 2; len <= n; len <<= 1) {
        for (u64 i = 0; i < n; i += len) {
            cpx* ptr2 = ptr;
            for (u64 j = 0; j < len / 2; j++, ptr2++) {
                cpx u = a[i + j], v = a[i + j + len / 2] * *ptr2;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
            }
        }
        ptr += len / 2;
    }

    if (invert)
        for (u64 len = 2, i = 0; len <= n; len <<= 1)
            for (u64 j = 0; j < len / 2; j++, i++) ang[i] = conj(ang[i]);

    if (invert)
        for (u64 i = 0; i < n; i++) a[i] /= n;
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

    u32* pos = pre_calc_pos(n);
    cpx* angu = pre_calc_ang(n);
    fft(fa, n, 0, pos, angu);
    fft(fb, n, 0, pos, angu);
    for (u64 i = 0; i < n; i++) fa[i] *= fb[i];
    fft(fa, n, 1, pos, angu);
    free(pos);
    free(angu);

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
