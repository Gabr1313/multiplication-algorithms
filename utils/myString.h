#pragma once

#include "myType.h"

#include <stdint.h>
#include <stdio.h>

typedef struct _string {
    u64 len;
    u64 cap;
    char *ptr;
} String;

String string_new(uint64_t cap);

void string_free(String s);

void string_push(String *s, char ch);

void string_shrink(String *s);

String string_read(FILE *stream);

void string_rev(String *s);
