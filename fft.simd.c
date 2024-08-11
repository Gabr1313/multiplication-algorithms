// precision: 2 to 6: higher precision
// 1 point in precision => time *= 2
// 1 point in precision => memory *= 2
// 4: is good          up to   1'000'000 binary digits
// 4: is good          up to  10'000'000 binary digits
// 5: is good at least up to 100'000'000 binary digits
// 6: too much memory => killed
// there also exists `complex long double`, in case they are needed
// i can sacrifice some performance for memory, but only up to half
#include <math.h>
#define PRC 5
#define PI 3.141592653589793238462643383279502884

#include <assert.h>
#include <complex.h>
#include <immintrin.h>
#include <stdlib.h>

#include "utils/myInt.h"

typedef complex double cpx;

// faster on smaller input, slower on bigger ones, don't know why!
// maybe cache misses?

// (a + i*b) * (c + i*d) = (e + i*f)
void cpx_mul(double a, double b, double c, double d, double* e, double* f) {
    *e = a * c - b * d;
    *f = a * d + b * c;
}

// (a + i*b) * (c + i*d) = (e + i*f)
void cpx_mul_128(__m128d a, __m128d b, __m128d c, __m128d d, __m128d* e, __m128d* f) {
    __m128d ac = _mm_mul_pd(a, c);
    __m128d bd = _mm_mul_pd(b, d);
    __m128d ad = _mm_mul_pd(a, d);
    __m128d bc = _mm_mul_pd(b, c);
    *e         = _mm_sub_pd(ac, bd);
    *f         = _mm_add_pd(ad, bc);
}

// (a + i*b) * (c + i*d) = (e + i*f)
void cpx_mul_256(__m256d a, __m256d b, __m256d c, __m256d d, __m256d* e, __m256d* f) {
    __m256d ac = _mm256_mul_pd(a, c);
    __m256d bd = _mm256_mul_pd(b, d);
    __m256d ad = _mm256_mul_pd(a, d);
    __m256d bc = _mm256_mul_pd(b, c);
    *e         = _mm256_sub_pd(ac, bd);
    *f         = _mm256_add_pd(ad, bc);
}

// (a + i*b) * (c + i*d) = (e + i*f)
void cpx_mul_512(__m512d a, __m512d b, __m512d c, __m512d d, __m512d* e, __m512d* f) {
    __m512d ac = _mm512_mul_pd(a, c);
    __m512d bd = _mm512_mul_pd(b, d);
    __m512d ad = _mm512_mul_pd(a, d);
    __m512d bc = _mm512_mul_pd(b, c);
    *e         = _mm512_sub_pd(ac, bd);
    *f         = _mm512_add_pd(ad, bc);
}

void fft(double* a_r, double* a_i, u64 a_size, u64 invert) {
    u64 n = a_size;

    for (u64 i = 1, j = 0; i < n; i++) {
        u64 bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            double tmp = a_r[i];
            a_r[i]     = a_r[j];
            a_r[j]     = tmp;
            tmp        = a_i[i];
            a_i[i]     = a_i[j];
            a_i[j]     = tmp;
        }
    }

    for (u64 len = 2; len <= n; len <<= 1) {
        double ang      = 2 * PI / len * (invert ? -1 : 1);
        double step1[2] = {cos(ang), sin(ang)};
        if (len == 2) {
            for (u64 i = 0; i < n; i += len) {
                double u_r       = a_r[i];
                double u_i       = a_i[i];
                double v_r       = a_r[i + 1];
                double v_i       = a_i[i + 1];
                a_r[i]           = u_r + v_r;
                a_i[i]           = u_i + v_i;
                a_r[i + len / 2] = u_r - v_r;
                a_i[i + len / 2] = u_i - v_i;
            }
            /* for (u64 i = 0; i < n; i += len * 8) { // can't believe this is not any faster!
                __m512d u_r    = _mm512_loadu_pd(&a_r[i]);
                u_r            = _mm512_mask_loadu_pd(u_r, 0b10101010, &a_r[i + 7]);
                __m512d v_r    = _mm512_loadu_pd(&a_r[i + 1]);
                v_r            = _mm512_mask_loadu_pd(v_r, 0b10101010, &a_r[i + 8]);
                __m512d sum_r  = _mm512_add_pd(u_r, v_r);
                __m512d diff_r = _mm512_sub_pd(u_r, v_r);
                _mm512_mask_storeu_pd(&a_r[i], 0b01010101, sum_r);
                _mm512_mask_storeu_pd(&a_r[i + 7], 0b10101010, sum_r);
                _mm512_mask_storeu_pd(&a_r[i + 1], 0b01010101, diff_r);
                _mm512_mask_storeu_pd(&a_r[i + 8], 0b10101010, diff_r);
                __m512d u_i    = _mm512_loadu_pd(&a_i[i]);
                u_i            = _mm512_mask_loadu_pd(u_i, 0b10101010, &a_i[i + 7]);
                __m512d v_i    = _mm512_loadu_pd(&a_i[i + 1]);
                v_i            = _mm512_mask_loadu_pd(v_i, 0b10101010, &a_i[i + 8]);
                __m512d sum_i  = _mm512_add_pd(u_i, v_i);
                __m512d diff_i = _mm512_sub_pd(u_i, v_i);
                _mm512_mask_storeu_pd(&a_i[i], 0b01010101, sum_i);
                _mm512_mask_storeu_pd(&a_i[i + 7], 0b10101010, sum_i);
                _mm512_mask_storeu_pd(&a_i[i + 1], 0b01010101, diff_i);
                _mm512_mask_storeu_pd(&a_i[i + 8], 0b10101010, diff_i);
            } */
        } else if (len == 4) {
            double tmp[2][3];
            tmp[0][0] = 1, tmp[1][0] = 0, tmp[0][1] = step1[0], tmp[1][1] = step1[1];
            cpx_mul(tmp[0][1], tmp[1][1], tmp[0][1], tmp[1][1], &tmp[0][2], &tmp[1][2]);
            __m128d step2[2]  = {_mm_set1_pd(tmp[0][2]), _mm_set1_pd(tmp[1][2])};
            __m128d w2_pre[2] = {_mm_loadu_pd(tmp[0]), _mm_loadu_pd(tmp[1])};
            for (u64 i = 0; i < n; i += len) {
                __m128d w2[2] = {w2_pre[0], w2_pre[1]};
                for (u64 j = 0; j < len / 2; j += 2) {
                    __m128d u_r, u_i, v_r, v_i;
                    u_r = _mm_loadu_pd(&a_r[i + j]);
                    u_i = _mm_loadu_pd(&a_i[i + j]);
                    v_r = _mm_loadu_pd(&a_r[i + j + len / 2]);
                    v_i = _mm_loadu_pd(&a_i[i + j + len / 2]);
                    cpx_mul_128(v_r, v_i, w2[0], w2[1], &v_r, &v_i);
                    _mm_storeu_pd(&a_r[i + j], _mm_add_pd(u_r, v_r));
                    _mm_storeu_pd(&a_i[i + j], _mm_add_pd(u_i, v_i));
                    _mm_storeu_pd(&a_r[i + j + len / 2], _mm_sub_pd(u_r, v_r));
                    _mm_storeu_pd(&a_i[i + j + len / 2], _mm_sub_pd(u_i, v_i));
                    cpx_mul_128(w2[0], w2[1], step2[0], step2[1], &w2[0], &w2[1]);
                }
            }
        } else if (len == 8) {
            double tmp[2][5];
            tmp[0][0] = 1, tmp[1][0] = 0, tmp[0][1] = step1[0], tmp[1][1] = step1[1];
            cpx_mul(tmp[0][1], tmp[1][1], tmp[0][1], tmp[1][1], &tmp[0][2], &tmp[1][2]);
            cpx_mul(tmp[0][2], tmp[1][2], tmp[0][1], tmp[1][1], &tmp[0][3], &tmp[1][3]);
            cpx_mul(tmp[0][2], tmp[1][2], tmp[0][2], tmp[1][2], &tmp[0][4], &tmp[1][4]);
            __m256d step4[2]  = {_mm256_set1_pd(tmp[0][4]), _mm256_set1_pd(tmp[1][4])};
            __m256d w4_pre[2] = {_mm256_loadu_pd(tmp[0]), _mm256_loadu_pd(tmp[1])};
            for (u64 i = 0; i < n; i += len) {
                __m256d w4[2] = {w4_pre[0], w4_pre[1]};
                for (u64 j = 0; j < len / 2; j += 4) {
                    __m256d u_r, u_i, v_r, v_i;
                    u_r = _mm256_loadu_pd(&a_r[i + j]);
                    u_i = _mm256_loadu_pd(&a_i[i + j]);
                    v_r = _mm256_loadu_pd(&a_r[i + j + len / 2]);
                    v_i = _mm256_loadu_pd(&a_i[i + j + len / 2]);
                    cpx_mul_256(v_r, v_i, w4[0], w4[1], &v_r, &v_i);
                    _mm256_storeu_pd(&a_r[i + j], _mm256_add_pd(u_r, v_r));
                    _mm256_storeu_pd(&a_i[i + j], _mm256_add_pd(u_i, v_i));
                    _mm256_storeu_pd(&a_r[i + j + len / 2], _mm256_sub_pd(u_r, v_r));
                    _mm256_storeu_pd(&a_i[i + j + len / 2], _mm256_sub_pd(u_i, v_i));
                    cpx_mul_256(w4[0], w4[1], step4[0], step4[1], &w4[0], &w4[1]);
                }
            }
        } else {
            double tmp[2][9];
            tmp[0][0] = 1, tmp[1][0] = 0, tmp[0][1] = step1[0], tmp[1][1] = step1[1];
            cpx_mul(tmp[0][1], tmp[1][1], tmp[0][1], tmp[1][1], &tmp[0][2], &tmp[1][2]);
            cpx_mul(tmp[0][2], tmp[1][2], tmp[0][1], tmp[1][1], &tmp[0][3], &tmp[1][3]);
            cpx_mul(tmp[0][2], tmp[1][2], tmp[0][2], tmp[1][2], &tmp[0][4], &tmp[1][4]);
            cpx_mul(tmp[0][4], tmp[1][4], tmp[0][1], tmp[1][1], &tmp[0][5], &tmp[1][5]);
            cpx_mul(tmp[0][4], tmp[1][4], tmp[0][2], tmp[1][2], &tmp[0][6], &tmp[1][6]);
            cpx_mul(tmp[0][4], tmp[1][4], tmp[0][3], tmp[1][3], &tmp[0][7], &tmp[1][7]);
            cpx_mul(tmp[0][4], tmp[1][4], tmp[0][4], tmp[1][4], &tmp[0][8], &tmp[1][8]);
            __m512d step8[2]  = {_mm512_set1_pd(tmp[0][8]), _mm512_set1_pd(tmp[1][8])};
            __m512d w8_pre[2] = {_mm512_loadu_pd(tmp[0]), _mm512_loadu_pd(tmp[1])};
            for (u64 i = 0; i < n; i += len) {
                __m512d w8[2] = {w8_pre[0], w8_pre[1]};
                for (u64 j = 0; j < len / 2; j += 8) {
                    __m512d u_r, u_i, v_r, v_i;
                    u_r = _mm512_loadu_pd(&a_r[i + j]);
                    u_i = _mm512_loadu_pd(&a_i[i + j]);
                    v_r = _mm512_loadu_pd(&a_r[i + j + len / 2]);
                    v_i = _mm512_loadu_pd(&a_i[i + j + len / 2]);
                    cpx_mul_512(v_r, v_i, w8[0], w8[1], &v_r, &v_i);
                    _mm512_storeu_pd(&a_r[i + j], _mm512_add_pd(u_r, v_r));
                    _mm512_storeu_pd(&a_i[i + j], _mm512_add_pd(u_i, v_i));
                    _mm512_storeu_pd(&a_r[i + j + len / 2], _mm512_sub_pd(u_r, v_r));
                    _mm512_storeu_pd(&a_i[i + j + len / 2], _mm512_sub_pd(u_i, v_i));
                    cpx_mul_512(w8[0], w8[1], step8[0], step8[1], &w8[0], &w8[1]);
                }
            }
        }
    }

    if (invert)
        for (u64 i = 0; i < a_size; i++) a_r[i] /= n, a_i[i] /= n;
}

BigInt mul(BigInt a, BigInt b) {
    u64 n;
    for (n = 1 << PRC; (n >> PRC) < a.len + b.len; n <<= 1);

    u64 mask     = ((1ull << (1 << (6 - PRC))) - 1);
    double* fa_r = malloc(sizeof(double) * n);
    double* fa_i = malloc(sizeof(double) * n);
    double* fb_r = malloc(sizeof(double) * n);
    double* fb_i = malloc(sizeof(double) * n);
    for (u64 i = 0; i < n; i++) {
        u64 shf = ((i & ((1ull << PRC) - 1)) << (6 - PRC));
        if (i < a.len << PRC) fa_r[i] = (a.ptr[i >> PRC] >> shf) & mask;
        else fa_r[i] = 0;
        if (i < b.len << PRC) fb_r[i] = (b.ptr[i >> PRC] >> shf) & mask;
        else fb_r[i] = 0;
        fa_i[i] = fb_i[i] = 0;
    }

    fft(fa_r, fa_i, n, 0);
    fft(fb_r, fb_i, n, 0);
    for (u64 i = 0; i < n; i++) cpx_mul(fa_r[i], fa_i[i], fb_r[i], fb_i[i], &fa_r[i], &fa_i[i]);
    fft(fa_r, fa_i, n, 1);

    BigInt c = bigint_new(a.len + b.len);
    c.len    = c.cap;

    u64 carry = 0;
    for (u64 i = 0; i < c.len << PRC; i++) {
        u64 res = (u64)round(fa_r[i]) + carry;
        carry   = res >> (1 << (6 - PRC));
        u64 shf = ((i & ((1ull << PRC) - 1)) << (6 - PRC));
        c.ptr[i >> PRC] += (res & mask) << shf;
    }

    free(fa_r);
    free(fa_i);
    free(fb_r);
    free(fb_i);

    bigint_clean(&c);
    return c;
}
