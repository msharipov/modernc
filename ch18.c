/*
*   TODO:
*       - implement parallel merge sort
*       - generalize for 2^k threads
*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define LEN 1000000


void
fill_rand(const size_t n, double arr[static n], const int* const seed) {

    if (!seed) {

        srand(time(NULL));

    } else {
        
        srand(*seed);
    }
    
    for (size_t i = 0; i < n; i++) {

        arr[i] = (1.0*rand())/RAND_MAX;
    }
}


int
compare_double(const void* a, const void* b) {

    if (a > b) {

        return 1;

    } else if (a < b) {

        return -1;
    }

    return 0;
}


int
gen_mergesort(const size_t len, const size_t size, void* arr,
              int(*comp)(const void*, const void*)) {
    
    if (len == 1) {

        return 0;
    }

    const size_t len_left = len/2;
    const size_t len_right = (len + 1)/2;
    uint8_t* left = arr;
    uint8_t* right = (uint8_t*)arr + size*len_left;

    if (gen_mergesort(len_left, size, left, comp) ||
        gen_mergesort(len_right, size, right, comp)) {

        return 1;
    }
    
    uint8_t* new_arr = calloc(len, size);
    if (!new_arr) {

        return 1;
    }

    size_t left_merged = 0;
    size_t right_merged = 0;
    while (left_merged + right_merged != len) {

        if (left_merged == len_left) {

            memcpy(&new_arr[size*(left_merged + right_merged)], 
                   &right[size*right_merged],
                   size*(len_right - right_merged));
            break;
        }

        if (right_merged == len_right) {

            memcpy(&new_arr[size*(left_merged + right_merged)], 
                   &left[size*left_merged],
                   size*(len_left - left_merged));
            break;
        }

        if (comp(&left[size*left_merged], &right[size*right_merged]) < 0) {

            memcpy(&new_arr[size*(left_merged + right_merged)], 
                   &left[size*left_merged], size);

            left_merged++;

        } else {

            memcpy(&new_arr[size*(left_merged + right_merged)], 
                   &right[size*right_merged], size);

            right_merged++;
        }
    }

    memcpy(arr, new_arr, size*len);

    free(new_arr);
    return 0;
}


bool
is_sorted(const size_t len, const size_t size, void* arr,
              int(*comp)(const void*, const void*)) {
    
    for (size_t i = 0; i < len - 1; i++) {
        
        if (comp((uint8_t*)arr + size*i, 
                 (uint8_t*)arr + size*(i + 1)) > 0) {

            return false;
        }
    }
    return true;
}


int
main(int argc, char* argv[static argc]) {
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s K\n", argv[0]);
        return EXIT_FAILURE;
    }

    double* numbers = calloc(LEN, sizeof(double));
    if (!numbers) {
        fprintf(stderr, "Memory allocation failed!\n");
        return EXIT_FAILURE;
    }

    int seed = 7345;
    fill_rand(LEN, numbers, &seed);

    gen_mergesort(LEN, sizeof(double), numbers, compare_double);
    bool sorted = is_sorted(LEN, sizeof(double), numbers, compare_double);
    printf("Array sorted %s\n", (sorted) ? "correctly" : "incorrectly");
    free(numbers);
    return EXIT_SUCCESS;
}
