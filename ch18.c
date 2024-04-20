/*
*   TODO:
*       - implement parallel merge sort
*       - generalize for 2^k threads
*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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


int
main(int argc, char* argv[static argc]) {

    return EXIT_SUCCESS;
}
