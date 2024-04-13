/* 
*   TODO:
*   - display prime factors of a list of numbers = DONE
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INT_LENGTH 20

size_t
lowest_prime_factor(const size_t n) {

    for (size_t div = 2; div*div <= n; div++) {

        if (!(n % div)) {

            return div;
        }
    }

    return n;
}


int
main(int argc, char* argv[static argc]) {

    if (argc < 2) {

        printf("Usage: %s N1 [N2 N3 ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char N_str[MAX_INT_LENGTH + 1] = {0};

    for (size_t number = 1; number < argc; number++) {

        strncpy(N_str, argv[number], MAX_INT_LENGTH);
        size_t N = strtoull(N_str, 0, 10);
        
        if (N < 2) {

            printf("N must be greater than 1!\n");
            exit(EXIT_FAILURE);
        }

        printf("%zu:", N);

        while (N != 1) {

            size_t div = lowest_prime_factor(N);
            printf(" %zu", div);
            N /= div;
        }
        
        printf("\n");
    }

    return EXIT_SUCCESS;
}
