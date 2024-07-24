#include <assert.h>
#include <stdio.h>

#include "utils/myInt.h"

void bigint_sum_eq_shifted_unchecked(BigInt* c, BigInt* a, u64 shift) {
    if (!a->len) return;
    u64 offset = shift / 64;
    assert(offset + a->len + 1 <= c->cap);
    shift %= 64;
    u64* c_ptr = c->ptr + offset;
    shift %= 64;
    u64 carry = 0;
    for (u64 i = 0; i <= a->len; i++) {
        u64 res = c_ptr[i] + carry;
        if (i < a->len) res += (a->ptr[i] << shift);
        if (i && shift) res += a->ptr[i - 1] >> (64 - shift);
        carry = (res < c_ptr[i]) || (carry && c_ptr[i] == res);
        c_ptr[i] = res;
    }
    u64 i;
    for (i = a->len; carry; i++) {
        c_ptr[i]++;
        carry = (c_ptr[i] == 0);
    }
    u64 len2 = offset + (i - 1 > a->len + 1 ? i - 1 : a->len + 1);
    c->len = (len2 > c->len ? len2 : c->len);
}

BigInt naif_mul(BigInt a, BigInt b) {
    BigInt c = bigint_new(a.len + b.len + 1);  // +1 to avoid OF
    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
    }
    for (u64 i = 0; i < b.len * 64; i++)
        if (bigint_is_set(b, i)) bigint_sum_eq_shifted_unchecked(&c, &a, i);
    return c;
}

int main() {
    BigInt a = bigint_read(stdin);
    BigInt b = bigint_read(stdin);
    BigInt c = naif_mul(a, b);

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
