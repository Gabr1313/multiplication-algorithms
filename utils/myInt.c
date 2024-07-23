#include "myInt.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void bignum_free(BigNum n) {
    free(n.ptr);
}

BigNum bignum_clone(BigNum n) {
    BigNum m;
    m.cap = n.cap;
    assert(m.cap % 64 == 0);
    m.ptr = malloc(m.cap / 8);
    memcpy(m.ptr, n.ptr, m.cap / 8);
    return m;
}

String bignum_to_string(BigNum n) {
    n = bignum_clone(n);
    String s = string_new(n.cap >= 6 ? n.cap / 3 : 2);  // 3 < log2(10)
    int right_most = n.cap / 64;
    while (1) {
        u64 carry = 0;
        while (right_most > 0 && n.ptr[right_most - 1] == 0) right_most--;
        if (right_most <= 0) break;
        for (int i = right_most - 1; i >= 0; i--) {
            u64 x = (n.ptr[i] >> 32) + carry * (1ull << 32);
            carry = x % 10;
            n.ptr[i] &= (u64)UINT32_MAX;
            n.ptr[i] |= (x / 10) << 32;

            x = (n.ptr[i] & (u64)UINT32_MAX) + carry * (1ull << 32);
            carry = x % 10;
            n.ptr[i] &= ~(u64)UINT32_MAX;
            n.ptr[i] |= x / 10;
        }
        string_push(&s, carry + '0');
    }
    bignum_free(n);
    if (s.len == 0) string_push(&s, '0');
    string_push(&s, '\0');
    string_shrink(&s);
    string_rev(&s);
    return s;
}

BigNum bignum_new(u64 bits) {
    BigNum n = {
        .cap = (bits + 63) / 64 * 64,
        .ptr = malloc(n.cap / 8),
    };
    assert(n.ptr);
    return n;
}

void bignum_set(BigNum *n, u64 pos, u64 val) {
    assert(pos < n->cap);
    if (val) n->ptr[(pos) / 64] |= 1ull << ((pos) % 64);
    else n->ptr[(pos) / 64] &= ~(1ull << ((pos) % 64));
}

void bignum_shrink(BigNum *n, u64 cap) {
    if (cap % 64) n->ptr[(cap - 1) / 64] &= (UINT64_MAX >> (64 - cap % 64));
    cap = (cap + 63) / 64 * 64;
    if (cap != n->cap) {
        n->ptr = realloc(n->ptr, n->cap / 8);
        assert(n->ptr);
        n->cap = cap;
    }
}

BigNum bignum_read(FILE *stream) {
    String s = string_read(stream);
    for (u64 i = 0; i < s.len - 1; i++) assert(s.ptr[i] >= '0' && s.ptr[i] <= '9');
    BigNum n = bignum_new(s.len * 4);  // 4 > log2(10)
    u64 left_most = 0, pos = 0;
    while (left_most <= s.len - 2) {
        bignum_set(&n, pos++, (s.ptr[s.len - 2] - '0') % 2);
        s.ptr[s.len - 2] = (s.ptr[s.len - 2] - '0') / 2 + '0';
        for (u64 i = s.len - 3; i >= left_most && i != UINT64_MAX; i--) {
            s.ptr[i + 1] += 5 * ((s.ptr[i] - '0') % 2);
            s.ptr[i] = (s.ptr[i] - '0') / 2 + '0';
        }
        left_most += (s.ptr[left_most] == '0');
    }
    string_free(s);
    bignum_shrink(&n, pos);
    return n;
}

void bignum_cpy_increase_cap(BigNum a, u64 cap) {
    BigNum b = bignum_new(cap);
    memcpy(b.ptr, a.ptr, a.cap / 8);
    memset(b.ptr + a.cap / 64, 0, b.cap / 8 - a.cap / 8);
}

void bignum_sub_eq(const BigNum *const a, BigNum b) {
    assert(a->cap >= b.cap);
    u64 carry = 0;
    for (u64 i = 0; i < a->cap / 64; i++) {
        u64 res = a->ptr[i] - carry;
        if (i < b.cap / 64) res -= b.ptr[i];
        carry = res > a->ptr[i];
        a->ptr[i] = res;
    }
}

void bignum_sum_eq_uncheked(BigNum *a, BigNum b) {
    u64 carry = 0;
    for (u64 i = 0; i < a->cap / 64; i++) {
        u64 res = a->ptr[i] + carry;
        if (i < b.cap / 64) res += b.ptr[i];
        carry = res < a->ptr[i];
        a->ptr[i] = res;
    }
    assert(carry == 0);
}

void bignum_sum(BigNum a, BigNum b, BigNum *c) {
    if (a.cap < b.cap) {
        BigNum tmp = a;
        a = b;
        b = tmp;
    }
    assert(c->cap >= a.cap);
    int carry = 0;
    for (u64 i = 0; i < a.cap / 64; i++) {
        c->ptr[i] = a.ptr[i] + carry;
        carry = (c->ptr[i] == 0);
        if (i < b.cap / 64) {
            c->ptr[i] += b.ptr[i];
            carry |= c->ptr[i] < b.ptr[i];
        }
    }
    if (carry) {
        assert(c->cap >= a.cap + 64);
        c->cap = a.cap + 64;
        c->ptr[a.cap / 64] = 1ull;
    } else c->cap = a.cap;
}

void bignum_print(FILE *stream, BigNum x) {
    String s = bignum_to_string(x);
    fprintf(stream, "%s", s.ptr);
    string_free(s);
}
