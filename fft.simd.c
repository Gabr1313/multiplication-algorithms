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

typedef complex double cpx;

// (a + i*b) * (c + i*d) = (ca - db) + i*(da + cb)
static inline __m512d custom_mul(__m512d ab, __m512d cd) {
    static u64 x[8] = {1ull << 63, 0, 1ull << 63, 0, 1ull << 63, 0, 1ull << 63, 0};
    static u64 y[8] = {2, 0, 6, 4, 10, 8, 14, 12};   // * 2?
    static u64 z[8] = {0, 0, 4, 4, 8, 8, 12, 12};    // * 2?
    static u64 w[8] = {2, 2, 6, 6, 10, 10, 14, 14};  // * 2?
    __m512d dc      = _mm512_permutevar_pd(cd, _mm512_load_epi64(&y));
    __m512i mask    = _mm512_load_epi64(&x);
    __m512d _dc     = (__m512d)_mm512_xor_epi64((__m512i)dc, mask);  // hack to change sign
    __m512d aa      = _mm512_permutevar_pd(ab, _mm512_load_epi64(&z));
    __m512d bb      = _mm512_permutevar_pd(ab, _mm512_load_epi64(&w));
    __m512d cada    = _mm512_mul_pd(cd, aa);
    __m512d dbcb    = _mm512_mul_pd(_dc, bb);
    __m512d res     = _mm512_add_pd(cada, dbcb);
    return res;
}

void fft_pre(cpx* fa, u64 n) {
    for (u64 i = 1, j = 0; i < n; i++) {
        u64 bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            cpx tmp = fa[i];
            fa[i]   = fa[j];
            fa[j]   = tmp;
        }
    }
}

void fft_pre_2(cpx* fa, cpx* fb, u64 n) {
    for (u64 i = 1, j = 0; i < n; i++) {
        u64 bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            cpx tmp = fa[i];
            fa[i]   = fa[j];
            fa[j]   = tmp;
            tmp     = fb[i];
            fb[i]   = fb[j];
            fb[j]   = tmp;
        }
    }
}

void fft_post(cpx* a, u64 n, u64 invert) {
    for (u64 len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cpx wlen   = cos(ang) + I * sin(ang);
        if (len == 2) {
            for (u64 i = 0; i < n; i += 8) {
                __m512d u = _mm512_mask_loadu_pd(_mm512_loadu_pd(&a[i]), 0b11001100, &a[i + 3]);
                __m512d v = _mm512_mask_loadu_pd(_mm512_loadu_pd(&a[i + 1]), 0b11001100, &a[i + 4]);
                __m512d sum = _mm512_add_pd(u, v);
                _mm512_mask_storeu_pd(&a[i], 0b00110011, sum);
                _mm512_mask_storeu_pd(&a[i + 3], 0b11001100, sum);
                __m512d diff = _mm512_sub_pd(u, v);
                _mm512_mask_storeu_pd(&a[i + 1], 0b00110011, diff);
                _mm512_mask_storeu_pd(&a[i + 4], 0b11001100, diff);
            }
        } else if (len == 4) {
            for (u64 i = 0; i < n; i += 8) {
                a[i + 3] *= wlen;
                a[i + 7] *= wlen;
                __m512d u = _mm512_mask_loadu_pd(_mm512_loadu_pd(&a[i]), 0b11110000, &a[i + 2]);
                __m512d v = _mm512_mask_loadu_pd(_mm512_loadu_pd(&a[i + 2]), 0b11110000, &a[i + 4]);
                __m512d sum = _mm512_add_pd(u, v);
                _mm512_mask_storeu_pd(&a[i], 0b00001111, sum);
                _mm512_mask_storeu_pd(&a[i + 2], 0b11110000, sum);
                __m512d diff = _mm512_sub_pd(u, v);
                _mm512_mask_storeu_pd(&a[i + 2], 0b00001111, diff);
                _mm512_mask_storeu_pd(&a[i + 4], 0b11110000, diff);
            }
        } else {
            for (u64 i = 0; i < n; i += len) {
                cpx wlen4  = wlen * wlen * wlen * wlen;
                __m512d ww = _mm512_set_pd(cimag(wlen * wlen * wlen), creal(wlen * wlen * wlen),
                                           cimag(wlen * wlen), creal(wlen * wlen), cimag(wlen),
                                           creal(wlen), 0, 1);
                __m512d ll = _mm512_set_pd(cimag(wlen4), creal(wlen4), cimag(wlen4), creal(wlen4),
                                           cimag(wlen4), creal(wlen4), cimag(wlen4), creal(wlen4));
                for (u64 j = 0; j + 3 < len / 2; j += 4) {
                    __m512d uu = _mm512_loadu_pd((double*)&a[i + j]);
                    __m512d vv = custom_mul(_mm512_loadu_pd((double*)&a[i + j + len / 2]), ww);
                    _mm512_storeu_pd((double*)&a[i + j], _mm512_add_pd(uu, vv));
                    _mm512_storeu_pd((double*)&a[i + j + len / 2], _mm512_sub_pd(uu, vv));
                    ww = custom_mul(ww, ll);
                }
            }
        }
    }

    if (invert) {
        __m512d ww = _mm512_set1_pd((double)1 / n);
        for (u64 i = 0; i < n; i += 4) {
            __m512d aa = _mm512_loadu_pd((double*)&a[i]);
            _mm512_storeu_pd((double*)&a[i], _mm512_mul_pd(aa, ww));
        }
    }
}

BigInt mul(BigInt a, BigInt b) {
    u64 n;
    for (n = 1 << PRC; (n >> PRC) < a.len + b.len; n <<= 1);

    u64 mask = ((1ull << (1 << (6 - PRC))) - 1);
    cpx* fa  = malloc(sizeof(cpx) * n);  // * 2 because i use u32
    cpx* fb  = malloc(sizeof(cpx) * n);  // * 2 because i use u32
    for (u64 i = 0; i < n; i++) {        // @simd?
        u64 shf = ((i & ((1ull << PRC) - 1)) << (6 - PRC));
        if (i < a.len << PRC) fa[i] = (a.ptr[i >> PRC] >> shf) & mask;
        else fa[i] = 0;
        if (i < b.len << PRC) fb[i] = (b.ptr[i >> PRC] >> shf) & mask;
        else fb[i] = 0;
    }

    fft_pre_2(fa, fb, n);
    fft_post(fa, n, 0);
    fft_post(fb, n, 0);
    for (u64 i = 0; i < n; i++) fa[i] *= fb[i];
    fft_pre(fa, n);
    fft_post(fa, n, 1);

    BigInt c = bigint_new(a.len + b.len);
    c.len    = c.cap;

    u64 carry = 0;
    for (u64 i = 0; i < c.len << PRC; i++) {  // @simd?
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
