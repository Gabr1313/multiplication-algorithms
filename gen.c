#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils/myType.h"

int main(int argc, char* argv[]) {
    assert(argc == 3);
    srand(time(NULL));

    u64 n1 = strtoul(argv[1], NULL, 10);
    if (n1 > 1) n1 += rand() % (n1 / 2) - (n1 / 4);
    else n1 = 1;

    u64 n2 = strtoul(argv[2], NULL, 10);
    if (n2 > 1) n2 += rand() % (n2 / 2) - (n2 / 4);
    else n2 = 1;

    fprintf(stderr, "%lu\n%lu\n", n1, n2);

    for (u64 i = 0; i < n1 / 4; i++) fprintf(stdout, "%x", rand() & 0xf);
    if (n1 % 4) fprintf(stdout, "%x", rand() & ((1 << (n1 % 4)) - 1));
    fprintf(stdout, "\n");

    for (u64 i = 0; i < n2 / 4; i++) fprintf(stdout, "%x", rand() & 0xf);
    if (n2 % 4) fprintf(stdout, "%x", rand() & ((1 << (n2 % 4)) - 1));
}
