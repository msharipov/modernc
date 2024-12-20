// Computes pi to N digits using the Bailey-Borwein-Plouffe algorithm
// https://mathworld.wolfram.com/BBPFormula.html

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#define BBP_SUMMATION_LIMIT 6ll

typedef struct Powers Powers;
struct Powers {
    int64_t s1;
    int64_t s2;
    int64_t s3;
    int64_t s4;

};

typedef struct Sums Sums;
struct Sums {
    long double s1;
    long double s2;
    long double s3;
    long double s4;
};


int64_t
mod_power(int64_t base, int64_t exp, int64_t mod) {

    int64_t result = 1;

    for (int64_t k = 0; k < exp; k++) {

        result *= base;
        result %= mod;
    }

    return result;
}


long double
bbp_sum(int64_t n, int64_t plus_k) {

    long double result = 0.0;

    for (int64_t k = 0; k <= n; k++) {

        result += (long double)(mod_power(16, n - k, 8*k + plus_k))
                  / (8*k + plus_k);
    }

    for (int64_t k = n + 1; k < n + BBP_SUMMATION_LIMIT; k++) {
        
        result += powl(16, n - k) / (8*k + plus_k);
    }
    
    return result;
}


long double
bbp_long_sum(int64_t n, int64_t plus_k) {

    long double result = 0.0;

    for (int64_t k = 0; k <= n; k++) {

        result += (long double)(mod_power(16, n - k, 8*k + plus_k))
                   / (8*k + plus_k);
    }
    
    return result;
}


long double
bbp_short_sum(int64_t n, int64_t plus_k) {

    long double result = 0.0;
    
    for (int64_t k = n + 1; k < n + BBP_SUMMATION_LIMIT; k++) {
        
        result += powl(16, n - k) / (8*k + plus_k);
    }

    return result;
}


int8_t
hex_digit(const int64_t n) {
    
    long double frac_parts = 0.0;
    long double int_part;

    frac_parts += 4 * modfl(bbp_sum(n, 1), &int_part);
    frac_parts -= 2 * modfl(bbp_sum(n, 4), &int_part);
    frac_parts -= modfl(bbp_sum(n, 5), &int_part);
    frac_parts -= modfl(bbp_sum(n, 6), &int_part);

    frac_parts = modfl(frac_parts, &int_part);
    if (frac_parts < 0.0) {

        frac_parts = 1 + frac_parts;
    }

    return (int8_t)floorl(16.0 * frac_parts);
}


void
generate_power_series(const size_t n, Powers arr[static (n + 1)]) {

    for (int64_t k = n; k >= 0; k--) {

        Powers current = {mod_power(16, n - k, 8*k + 1),
                          mod_power(16, n - k, 8*k + 4),
                          mod_power(16, n - k, 8*k + 5),
                          mod_power(16, n - k, 8*k + 6)};

        arr[(size_t)k] = current;
    }
}


void
update_power_series(const size_t n, Powers arr[static (n + 1)]) {
    
    for (size_t k = 0; k <= n; k++) {
    
        arr[k].s1 = (16*arr[k].s1) % (8*k + 1); 
        arr[k].s2 = (16*arr[k].s2) % (8*k + 4);
        arr[k].s3 = (16*arr[k].s3) % (8*k + 5);
        arr[k].s4 = (16*arr[k].s4) % (8*k + 6);
    }
}


void
hex_sequence_imprecise(const int64_t start, const int64_t stop,
                       int8_t digits[static (stop - start + 1)]) {
    
    long double int_part;
    long double sum1 = bbp_long_sum(start - 1, 1);
    long double sum2 = bbp_long_sum(start - 1, 4);
    long double sum3 = bbp_long_sum(start - 1, 5);
    long double sum4 = bbp_long_sum(start - 1, 6);

    for (int64_t n = start; n <= stop; n++) {
        
        long double frac_parts = 0.0;
        
        sum1 = modfl(sum1 * 16, &int_part) + 1.0/(8*n + 1);
        sum2 = modfl(sum2 * 16, &int_part) + 1.0/(8*n + 4);
        sum3 = modfl(sum3 * 16, &int_part) + 1.0/(8*n + 5);
        sum4 = modfl(sum4 * 16, &int_part) + 1.0/(8*n + 6);

        frac_parts += 4 * modfl(sum1 + bbp_short_sum(n, 1), &int_part);
        frac_parts -= 2 * modfl(sum2 + bbp_short_sum(n, 4), &int_part);
        frac_parts -= modfl(sum3 + bbp_short_sum(n, 5), &int_part);
        frac_parts -= modfl(sum4 + bbp_short_sum(n, 6), &int_part);

        frac_parts = modfl(frac_parts, &int_part);
        if (frac_parts < 0.0) {

            frac_parts = 1 + frac_parts;
        }

        digits[n - start] = (int8_t)floorl(16.0 * frac_parts);
    }
}


void
hex_sequence(const int64_t start, const int64_t stop,
             int8_t digits[static (stop + 1)]) {
    
    long double int_part;
    Powers* powers = malloc(sizeof(Powers[stop + 1]));

    if (!powers) {
        
        printf("Failed to create power tables.\n");
        exit(EXIT_FAILURE);
    }
    
    const Powers ones = {1, 1, 1, 1};
    for (size_t i = 0; i < (size_t)stop + 1; i++) {

        powers[i] = ones;
    }

    generate_power_series(start, powers);

    for (int64_t n = start; n <= stop; n++) {
        
        long double frac_parts = 0.0;
        Sums sums = {0.0, 0.0, 0.0, 0.0};

        for (long k = 0; k <= n; k++) {

            sums.s1 += (long double)(powers[k].s1)/(8*k + 1);
            sums.s2 += (long double)(powers[k].s2)/(8*k + 4);
            sums.s3 += (long double)(powers[k].s3)/(8*k + 5);
            sums.s4 += (long double)(powers[k].s4)/(8*k + 6);
        }

        update_power_series(n, powers);

        frac_parts += 4 * modfl(sums.s1 + bbp_short_sum(n, 1), &int_part);
        frac_parts -= 2 * modfl(sums.s2 + bbp_short_sum(n, 4), &int_part);
        frac_parts -= modfl(sums.s3 + bbp_short_sum(n, 5), &int_part);
        frac_parts -= modfl(sums.s4 + bbp_short_sum(n, 6), &int_part);

        frac_parts = modfl(frac_parts, &int_part);
        if (frac_parts < 0.0) {

            frac_parts = 1 + frac_parts;
        }

        digits[n - start] = (int8_t)floorl(16.0 * frac_parts);
    }

    free(powers);
 }


int
main(int argc, char * argv[static argc]) {

    size_t start = 0;
    size_t stop = 0;
    char N_str[21] = {0};
    char ** end = NULL;
    
    switch (argc) {
        
        case 2:
            strncpy(N_str, argv[1], 20);
            stop = strtoll(N_str, end, 10) - 1;
            break;
    
        case 3:
            strncpy(N_str, argv[1], 20);
            start = strtoll(N_str, end, 10);
            strncpy(N_str, argv[2], 20);
            stop = strtoll(N_str, end, 10);
            break;

        default:
            printf("Usage: ./ch3 [START] STOP\n");
            return EXIT_FAILURE;
    }


    if (stop < start) {

        printf("Usage: ./ch3 [START] STOP\n");
        return EXIT_FAILURE;
    }
    
    int8_t * pi_digits = malloc(sizeof(int8_t[stop - start + 1]));
    if (!pi_digits) {
        printf("Memory error.");
        return EXIT_FAILURE;
    }
    
    if (argc == 3) {

        hex_sequence(start, stop, pi_digits);

    } else {

        for (size_t digit = 0; digit <= stop; digit++) {

            pi_digits[digit] = hex_digit(digit);
        }
    }

    size_t col_counter = 0;
    size_t col_offset = 0;
    if (!start) {

        printf("pi = 3.");
        col_counter = 6;
        col_offset = 6;
    }
 
    for (size_t digit = 0; digit < stop - start + 1; digit++) {

        printf("%x", pi_digits[digit]);
        col_counter++;
        
        if (col_counter == 65 + col_offset) {
            
            if (!start) {

                printf("\n       ");

            } else {

                printf("\n");
            }

            col_counter = col_offset;

        } else if (col_counter >= (10 + col_offset) 
                   && !((col_counter - 10 - col_offset) % 11)) {

            printf(" ");
            col_counter++;
        }
    }
    printf("\n");

    free(pi_digits);
    return EXIT_SUCCESS;
}

