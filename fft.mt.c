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
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "utils/myInt.h"

#define NUM_THREADS 8

typedef complex double cpx;

typedef struct _Args {
    u64 len_or_stop;
    cpx wlen;
} Args;

typedef struct _ArgsB {
    u64 i, n;
    cpx* a;
    Args* args;
    sem_t *sem_a, *sem_b;
} ArgsB;

void* fft_thread(void* args_b) {
    u64 n        = (*(ArgsB*)args_b).n;
    u64 i_0      = (*(ArgsB*)args_b).i;
    cpx* a       = (*(ArgsB*)args_b).a;
    Args* args   = (*(ArgsB*)args_b).args;
    sem_t* sem_a = (*(ArgsB*)args_b).sem_a;
    sem_t* sem_b = (*(ArgsB*)args_b).sem_b;
    while (1) {
        sem_wait(sem_a);
        u64 len = args->len_or_stop;
        if (len == 0) break;
        cpx wlen  = args->wlen;
        u64 delta = len * NUM_THREADS;
        for (u64 i = i_0 * len; i < n; i += delta) {
            cpx w = 1;
            for (u64 j = 0; j < len / 2; j++) {
                cpx u              = a[i + j];
                cpx v              = a[i + j + len / 2] * w;
                a[i + j]           = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
        sem_post(sem_b);
    }
    return NULL;
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

    pthread_t threads[NUM_THREADS];
    sem_t sems_a[NUM_THREADS], sems_b[NUM_THREADS];
    Args args;
    ArgsB args_b[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) sem_init(&sems_a[i], 0, 0), sem_init(&sems_b[i], 0, 0);
    for (int i = 0; i < NUM_THREADS; i++)
        args_b[i] = (ArgsB){
            .args  = &args,
            .sem_a = &sems_a[i],
            .sem_b = &sems_b[i],
            .i     = i,
            .a     = a,
            .n     = n,
        };
    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&threads[i], 0, fft_thread, &args_b[i]);
    for (u64 len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cpx wlen   = cos(ang) + I * sin(ang);
        args       = (Args){
                  .len_or_stop = len,
                  .wlen        = wlen,
        };
        for (u64 i = 0; i < NUM_THREADS; i++) sem_post(&sems_a[i]);
        for (int i = 0; i < NUM_THREADS; i++) sem_wait(&sems_b[i]);
    }
    args.len_or_stop = 0;  // stop
    for (int i = 0; i < NUM_THREADS; i++) sem_post(&sems_a[i]);
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);

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
