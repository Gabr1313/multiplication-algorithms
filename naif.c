#include <assert.h>
#include <sys/time.h>

#include "utils/myInt.h"
#include "utils/myString.h"

BigInt mul(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.len + b.len);
    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
    }
    for (u64 i = 0; i < b.len * 2; i++) bigint_sum_eq_mul_u32(&c, a, ((u32*)b.ptr)[i], i);
    bigint_clean(&c);
    return c;
}
