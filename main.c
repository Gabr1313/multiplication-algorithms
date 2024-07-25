#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "utils/myInt.h"
#include "utils/header.h"

int main() {
    // BigInt a = bigint_read(stdin);
    // BigInt b = bigint_read(stdin);

    BigInt a = bigint_read_hex(stdin);
    BigInt b = bigint_read_hex(stdin);

    struct timeval end, start;
    gettimeofday(&start, NULL);
    BigInt c = mul(a, b);
    gettimeofday(&end, NULL);
    f64 time = (end.tv_sec - start.tv_sec) + (f64)(end.tv_usec - start.tv_usec) / 1e6;
    fprintf(stderr, "%.3f\n", time);

    // bigint_print_hex(stdout, a);
    // fprintf(stdout, "\n*\n");
    // bigint_print_hex(stdout, b);
    // fprintf(stdout, "\n=\n");
    bigint_print_hex(stdout, c);
    fprintf(stdout, "\n");

    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    return 0;
}
