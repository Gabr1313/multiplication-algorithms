#include "myString.h"
#include "myType.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

String string_new(u64 cap) {
    String s;
    s.cap = cap;
    s.ptr = realloc(NULL, sizeof(*s.ptr) * cap);
    if (!s.ptr) assert(0);
    s.len = 0;
    return s;
}

void string_free(String s) {
    free(s.ptr);
}

void string_push(String *s, char ch) {
    if (s->len == s->cap) {
        s->ptr = realloc(s->ptr, sizeof(*s->ptr) * (s->cap *= 2));
        if (!s->ptr) assert(0);
    }
    s->ptr[s->len++] = ch;
}

void string_shrink(String *s) {
    s->cap = s->len;
    s->ptr = realloc(s->ptr, sizeof(*s->ptr) * s->cap);
    if (!s->ptr) assert(0);
}

String string_read(FILE *stream) {
    int ch;
    String s = string_new(64);
    while (EOF != (ch = getc(stream)) && !isspace(ch)) string_push(&s, ch);
    string_push(&s, '\0');
    string_shrink(&s);
    return s;
}

void string_rev(String *s) {
    for (u64 i = 0; i < s->len - 2 - i; i++) {  // '\0' at the end
        char tmp = s->ptr[i];
        s->ptr[i] = s->ptr[s->len - 2 - i];
        s->ptr[s->len - 2 - i] = tmp;
    }
}
