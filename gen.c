#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils/myType.h"

int main(int argc, char* argv[]) {
    assert(argc == 3);
    u64 n1 = strtoul(argv[1], NULL, 10);
    srand(time(NULL));
    if (n1 > 1) n1 += rand() % (n1 / 2) - (n1 / 4);
    fprintf(stderr, "%lu\n", n1);
    u64 n2 = strtoul(argv[2], NULL, 10);
    if (n2 > 1) n2 += rand() % (n2 / 2) - (n2 / 4);
    fprintf(stderr, "%lu\n", n2);
    for (int i = 0; i < n1; i++) printf("%d", rand() % 10);
    printf("\n");
    for (int i = 0; i < n2; i++) printf("%d", rand() % 10);
}
