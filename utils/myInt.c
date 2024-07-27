#include "myInt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void bigint_free(BigInt n) {
    free(n.ptr);
}

BigInt bigint_new(u64 u64s) {
    BigInt n = {
        .cap = u64s,
        .len = n.cap,
        .ptr = calloc(n.cap, sizeof(u64)),
    };
    assert(n.ptr);
    return n;
}

BigInt bigint_clone(BigInt n) {
    BigInt m = {
        .cap = n.cap,
        .len = n.len,
        .ptr = malloc(m.cap * 8),
    };
    assert(m.ptr);
    memcpy(m.ptr, n.ptr, m.cap * 8);
    return m;
}

void bigint_shrink(BigInt *n) {
    if (n->len < n->cap) {
        n->ptr = realloc(n->ptr, n->len * 8);
        assert(n->ptr);
        n->cap = n->len;
    } else assert(n->len == n->cap);
}

void bigint_resize(BigInt *n, u64 cap) {
    if (n->cap == cap) return;
    n->ptr = realloc(n->ptr, n->len * 8);
    assert(n->ptr);
    if (n->cap > cap) memset(n->ptr + n->cap, 0, (cap - n->cap) * 8);
    n->cap = cap;
}

void bigint_set(BigInt *n, u64 pos, u64 val) {
    assert(pos < n->cap * 64);
    if (pos >= n->len * 64) n->len = (pos + 63) / 64;
    if (val) n->ptr[pos / 64] |= 1ull << (pos % 64);
    else n->ptr[pos / 64] &= ~(1ull << (pos % 64));
}

u64 bigint_is_set(BigInt n, u64 pos) {
    assert(pos < n.len * 64);
    return !!(n.ptr[pos / 64] & 1ull << (pos % 64));
}

void bigint_sub_eq(BigInt *a, BigInt b) {
    assert(a->len >= b.len);
    u64 carry = 0;
    for (u64 i = 0; i < a->len; i++) {
        u64 res = a->ptr[i] - carry - (i < b.len ? b.ptr[i] : 0);
        carry = (res > a->ptr[i]) || (carry && res == a->ptr[i]);
        a->ptr[i] = res;
    }
    assert(carry == 0);
}

void bigint_sum_eq_uncheked(BigInt *a, BigInt b) {
    u64 carry = 0;
    for (u64 i = 0; i < a->len; i++) {
        u64 res = a->ptr[i] + carry + (i < b.cap ? b.ptr[i] : 0);
        carry = res < a->ptr[i] || (carry && res == a->ptr[i]);
        a->ptr[i] = res;
    }
    assert(carry == 0);
}

void bigint_sum(BigInt a, BigInt b, BigInt *c) {
    if (a.len < b.len) {
        BigInt tmp = a;
        a = b;
        b = tmp;
    }
    assert(c->cap >= a.len);
    int carry = 0;
    for (u64 i = 0; i < a.len; i++) {
        u64 res = a.ptr[i] + carry + (i < b.len ? b.ptr[i] : 0);
        carry = res < a.ptr[i] || (carry && res == a.ptr[i]);
        c->ptr[i] = res;
    }
    if (carry) {
        assert(c->cap > a.len);
        c->len = a.len + 1;
        c->ptr[a.len] = 1ull;
    } else c->len = a.len;
}

void bigint_clean(BigInt *c) {
    for (u64 i = c->len - 1;; i--) {
        if (c->ptr[i] != 0) {
            c->len = i + 1;
            break;
        } else if (i == 0) {
            c->len = 0;
            break;
        }
    }
}

String bigint_to_string(BigInt n) {
    n = bigint_clone(n);
    String s = string_new(n.len ? n.len * 64 / 3 : 2);  // 3 < log2(10)
    int right_most = n.len;
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
    bigint_free(n);
    if (s.len == 0) string_push(&s, '0');
    string_push(&s, '\0');
    string_shrink(&s);
    string_rev(&s);
    return s;
}

String bigint_to_string_hex(BigInt n) {
    String s = string_new(n.len ? n.len * 16 + 1 : 2);  // 3 < log2(10)
    if (!n.len) {
        sprintf(s.ptr, "0");
        s.len = 1;
    } else {
        for (int i = n.len - 1; i >= 0; i--)
            sprintf(s.ptr + (n.len - 1 - i) * 16, "%016lx", n.ptr[i]);
        s.ptr[s.cap - 1] = '\0';
        s.len = s.cap - 1;
    }
    return s;
}

String bigint_to_string_hex_dbg(BigInt n) {
    String s = string_new(n.len ? n.len * 17 + 1 : 2);  // 3 < log2(10)
    if (!n.len) {
        sprintf(s.ptr, "0");
        s.len = 1;
    } else {
        for (int i = n.len - 1; i >= 0; i--)
            sprintf(s.ptr + (n.len - 1 - i) * 17, "%016lx ", n.ptr[i]);
        s.ptr[s.cap - 2] = '\0';
        s.len = s.cap - 2;
    }
    return s;
}

BigInt bigint_read(FILE *stream) {
    String s = string_read(stream);
    for (u64 i = 0; i < s.len - 1; i++) assert(s.ptr[i] >= '0' && s.ptr[i] <= '9');
    BigInt n = bigint_new((s.len * 4 + 63) / 64);  // 4 > log2(10)
    u64 left_most = 0, pos = 0;
    while (left_most <= s.len - 2) {
        bigint_set(&n, pos++, (s.ptr[s.len - 2] - '0') % 2);
        s.ptr[s.len - 2] = (s.ptr[s.len - 2] - '0') / 2 + '0';
        for (u64 i = s.len - 3; i >= left_most && i != UINT64_MAX; i--) {
            s.ptr[i + 1] += 5 * ((s.ptr[i] - '0') % 2);
            s.ptr[i] = (s.ptr[i] - '0') / 2 + '0';
        }
        left_most += (s.ptr[left_most] == '0');
    }
    string_free(s);
    n.len = (pos + 63) / 64;
    bigint_shrink(&n);
    return n;
}

BigInt bigint_read_hex(FILE *stream) {
    String s = string_read(stream);
    for (u64 i = 0; i < s.len - 1; i++)
        assert((s.ptr[i] >= '0' && s.ptr[i] <= '9') || (s.ptr[i] >= 'a' && s.ptr[i] <= 'f'));
    BigInt n = bigint_new((s.len + 15) / 16);
    n.len = n.cap;

    for (u64 i = 0; i < n.len - 1; i++) {
        sscanf(s.ptr + s.len - (i + 1) * 16, "%lx", &n.ptr[i]);
        s.ptr[s.len - (i + 1) * 16] = '\0';
    }
    sscanf(s.ptr, "%lx", &n.ptr[n.len - 1]);

    string_free(s);
    return n;
}

void bigint_print(FILE *stream, BigInt x) {
    String s = bigint_to_string(x);
    fprintf(stream, "%s", s.ptr);
    string_free(s);
}

void bigint_print_hex(FILE *stream, BigInt x) {
    String s = bigint_to_string_hex(x);
    fprintf(stream, "%s", s.ptr);
    string_free(s);
}
