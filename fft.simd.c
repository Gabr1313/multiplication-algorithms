// precision: 2 to 6: higher precision
// 1 point in precision => time *= 2
// 1 point in precision => memory *= 2
// 4: is good          up to   1'000'000 binary digits
// 4: is good          up to  10'000'000 binary digits
// 5: is good at least up to 100'000'000 binary digits
// 6: too much memory => killed
// there also exists `complex long double`, in case they are needed
// i can sacrifice some performance for memory, but only up to half
#define PRC 5
#define PI 3.141592653589793238462643383279502884

#include <assert.h>
#include <complex.h>
#include <immintrin.h>
#include <math.h>
#include <stdlib.h>

#include "utils/myInt.h"

#pragma GCC target("avx512f")  // @todo remove

typedef complex double cpx;

// (a + i*b) * (c + i*d) = (ca - db) + i*(da + cb)
inline __m512d custom_mul(__m512d ab, __m512d cd) {  // too slow
    u64 sign_bit = (1ull << 63);
    __m512i mask = _mm512_set_epi64(0, sign_bit, 0, sign_bit, 0, sign_bit, 0, sign_bit);
    __m512d dc   = _mm512_permutevar_pd(cd, _mm512_set_epi64(12, 14, 8, 10, 4, 6, 0, 2));  // * 2?
    __m512d _dc  = (__m512d)_mm512_xor_epi64((__m512i)dc, mask);
    __m512d aa   = _mm512_permutevar_pd(ab, _mm512_set_epi64(12, 12, 8, 8, 4, 4, 0, 0));    // * 2?
    __m512d bb   = _mm512_permutevar_pd(ab, _mm512_set_epi64(14, 14, 10, 10, 6, 6, 2, 2));  // * 2?
    __m512d cada = _mm512_mul_pd(cd, aa);
    __m512d dbcb = _mm512_mul_pd(_dc, bb);
    __m512d res  = _mm512_add_pd(cada, dbcb);
    return res;
}

void fft(cpx* a, u64 a_size, u64 invert) {
    u64 n = a_size;

    for (u64 i = 1, j = 0; i < n; i++) {
        u64 bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            cpx tmp = a[i];
            a[i]    = a[j];
            a[j]    = tmp;
        }
    }

    for (u64 len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cpx wlen   = cos(ang) + I * sin(ang);
        for (u64 i = 0; i < n; i += len) {
            /* cpx w = 1;
            u64 j;
            __m512d ww = _mm512_set_pd(cimag(wlen * wlen * wlen), creal(wlen * wlen * wlen),
                                       cimag(wlen * wlen), creal(wlen * wlen), cimag(wlen),
                                       creal(wlen), 0, 1);
            cpx wlen4  = wlen * wlen * wlen * wlen;
            __m512d ll = _mm512_set_pd(cimag(wlen4), creal(wlen4), cimag(wlen4), creal(wlen4),
                                       cimag(wlen4), creal(wlen4), cimag(wlen4), creal(wlen4));

            for (j = 0; j + 3 < len / 2; j += 4) {
                __m512d uu = _mm512_loadu_pd((double*)&a[i + j]);
                __m512d vv = custom_mul(_mm512_loadu_pd((double*)&a[i + j + len / 2]), ww);
                _mm512_storeu_pd((double*)&a[i + j], _mm512_add_pd(uu, vv));
                _mm512_storeu_pd((double*)&a[i + j + len / 2], _mm512_sub_pd(uu, vv));
                ww = custom_mul(ww, ll);
            }
            w = cpowl(wlen, j);
            for (; j < len / 2; j++) {
                cpx u              = a[i + j];
                cpx v              = a[i + j + len / 2] * w;
                a[i + j]           = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            } */

            /* cpx w      = 1;
            __m512d ww = _mm512_set_pd(cimag(wlen * wlen * wlen), creal(wlen * wlen * wlen),
                                       cimag(wlen * wlen), creal(wlen * wlen), cimag(wlen),
                                       creal(wlen), 0, 1);
            cpx wlen4  = wlen * wlen * wlen * wlen;
            __m512d ll = _mm512_set_pd(cimag(wlen4), creal(wlen4), cimag(wlen4), creal(wlen4),
                                       cimag(wlen4), creal(wlen4), cimag(wlen4), creal(wlen4));
            u64 j;
            for (j = 0; j + 3 < len / 2; j += 4) {  // this is the slowest part!
                __m512d vv = _mm512_loadu_pd((double*)&a[i + j + len / 2]);
                _mm512_storeu_pd((double*)&a[i + j + len / 2], custom_mul(vv, ww));
                ww = custom_mul(ww, ll);
            }
            w = cpowl(wlen, j);
            for (; j < len / 2; j++) a[i + j + len / 2] *= w, w *= wlen;
            for (j = 0; j + 3 < len / 2; j += 4) {
                __m512d uu = _mm512_loadu_pd((double*)&a[i + j]);
                __m512d vv = _mm512_loadu_pd((double*)&a[i + j + len / 2]);
                _mm512_storeu_pd((double*)&a[i + j], _mm512_add_pd(uu, vv));
                _mm512_storeu_pd((double*)&a[i + j + len / 2], _mm512_sub_pd(uu, vv));
            }
            for (; j < len / 2; j++) {
                cpx u = a[i + j], v = a[i + j + len / 2];
                a[i + j]           = u + v;
                a[i + j + len / 2] = u - v;
            } */

            cpx w = 1;
            u64 j;
            // (a + i*b) * (c + i*d) = (ac - bd) + i*(ad + bc)
            for (j = 0; j + 3 < len / 2; j += 4) {
                a[i + j + 0 + len / 2] *= w, w *= wlen;
                a[i + j + 1 + len / 2] *= w, w *= wlen;
                a[i + j + 2 + len / 2] *= w, w *= wlen;
                a[i + j + 3 + len / 2] *= w, w *= wlen;
                __m512d uu = _mm512_loadu_pd((double*)&a[i + j]);
                __m512d vv = _mm512_loadu_pd((double*)&a[i + j + len / 2]);
                _mm512_storeu_pd((double*)&a[i + j], _mm512_add_pd(uu, vv));
                _mm512_storeu_pd((double*)&a[i + j + len / 2], _mm512_sub_pd(uu, vv));
            }
            for (; j < len / 2; j++) {
                a[i + j + len / 2] *= w, w *= wlen;
                cpx u              = a[i + j];
                cpx v              = a[i + j + len / 2];
                a[i + j]           = u + v;
                a[i + j + len / 2] = u - v;
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
    cpx* fa  = malloc(sizeof(cpx) * n);  // * 2 because i use u32
    cpx* fb  = malloc(sizeof(cpx) * n);  // * 2 because i use u32
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
    c.len    = c.cap;

    u64 carry = 0;
    for (u64 i = 0; i < c.len << PRC; i++) {
        u64 res = (u64)round(creal(fa[i])) + carry;
        carry   = res >> (1 << (6 - PRC));
        u64 shf = ((i & ((1ull << PRC) - 1)) << (6 - PRC));
        c.ptr[i >> PRC] += (res & mask) << shf;
    }

    free(fa);
    free(fb);

    bigint_clean(&c);
    return c;
}
