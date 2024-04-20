/*
*   TODO
*   - compare the speed of sorting algorithms from ch1
*
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>


void
print_array(const size_t n, const double arr[static n]) {

    for (size_t i = 0; i < n; i++) {

        printf("[%zu]: %f\n", i, arr[i]);
    }
}


bool
is_sorted(const size_t n, const double arr[static n]) {

    for (size_t i = 0; i < n - 1; i++) {

        if (arr[i] > arr[i + 1]) {

            return false;
        }
    }

    return true;
}


void
swap(double* const arr, const size_t a, const size_t b) {

    const double temp = arr[a];
    arr[a] = arr[b];
    arr[b] = temp;
}


void
quick_sort(const size_t len, double arr[static len]) {
    
    if (len < 3) {

        if (len == 2 && arr[0] > arr[1]) {

            swap(arr, 0, 1);
        }
        return;
    }

    // Hoare's scheme
    size_t i = 1;
    size_t j = len - 1;
    double P = arr[0];

    for (;;) {

        while (arr[i] < P && i < len) {

            i++;
        }

        while (arr[j] > P && j >= i) {

            j--;
        }

        if (i < j) {

            swap(arr, i, j);

        } else {

            swap(arr, 0, j);
            quick_sort(j, arr);
            quick_sort(len - i, &arr[i]);
            return;
        }
    }
}


void
merge_sort(const size_t len, double arr[static len]) {

    if (len < 3) {

        if (len == 2 && arr[0] > arr[1]) {

            swap(arr, 0, 1);
        }

        return;
    }

    size_t middle = len / 2 ;
    merge_sort(middle, arr);
    merge_sort(len - middle, &arr[middle]);

    double * temp = calloc(len, sizeof(double));

    if (!temp) {

        printf("Memory error!\n");
        exit(1);
    }

    size_t left = 0;
    size_t right = middle;
    size_t i = 0;

    while (left < middle && right < len) {

        if (arr[left] < arr[right]) {

            temp[i] = arr[left];
            left++;

        } else {

            temp[i] = arr[right];
            right++;
        }

        i++;
    }

    while (left < middle) {

        temp[i] = arr[left];
        left++;
        i++;
    }

    while (right < len) {

        temp[i] = arr[right];
        right++;
        i++;
    }

    memcpy(arr, temp, sizeof(double)*len);
    free(temp);
}


void
time_sort(double* list, const size_t list_len) {

    struct timespec start;
    struct timespec finish;
    intmax_t sec;
    intmax_t nsec;

    timespec_get(&start, TIME_UTC);
    quick_sort(list_len, list);
    timespec_get(&finish, TIME_UTC);
    sec = (intmax_t)finish.tv_sec - (intmax_t)start.tv_sec;
    nsec = finish.tv_nsec - start.tv_nsec;
    if (nsec < 0) {

        sec--;
        nsec += 1000000000;
    }
    printf("Quick sort: %jd.%09ld s\n", sec, nsec);
    
    timespec_get(&start, TIME_UTC);
    merge_sort(list_len, &list[list_len]);
    timespec_get(&finish, TIME_UTC);
    sec = (intmax_t)finish.tv_sec - (intmax_t)start.tv_sec;
    nsec = finish.tv_nsec - start.tv_nsec;
    if (nsec < 0) {

        sec--;
        nsec += 1000000000;
    }
    printf("Merge sort: %jd.%09ld s\n", sec, nsec);
}


int
main() {
    
    const size_t SHORT_LIST_LEN = 1000;
    const size_t MIDDLE_LIST_LEN = 20000;
    const size_t LONG_LIST_LEN = 400000;

    printf("Allocating memory for test data...\n");

    double* short_list = malloc(2 * SHORT_LIST_LEN * sizeof(double));
    if (!short_list) {
        
        printf("Allocation failed for the short list!\n");
        return EXIT_FAILURE;
    }
    
    double* mid_list = malloc(2 * MIDDLE_LIST_LEN * sizeof(double));
    if (!mid_list) {

        printf("Allocation failed for the medium list!\n");
        goto fail_mid; 
    }

    double* long_list = malloc(2 * LONG_LIST_LEN * sizeof(double));
    if (!long_list) {

        printf("Allocation failed for the long list!\n");
        goto fail_long; 
    }

    printf("Generating test data...\n");
    
    time_t rand_seed = time(NULL);
    if (rand_seed == (time_t)(-1)) {
        
        printf("Timing failure!\n");
        goto fail_all;
    }

    srand((intmax_t)rand_seed % RAND_MAX);
    
    for (size_t i = 0; i < SHORT_LIST_LEN; i++) {

        short_list[i] = (double)rand() / RAND_MAX;
        short_list[i + SHORT_LIST_LEN] = short_list[i];
    }

    for (size_t i = 0; i < MIDDLE_LIST_LEN; i++) {

        mid_list[i] = (double)rand() / RAND_MAX;
        mid_list[i + MIDDLE_LIST_LEN] = mid_list[i];
    }

    for (size_t i = 0; i < LONG_LIST_LEN; i++) {

        long_list[i] = (double)rand() / RAND_MAX;
        long_list[i + LONG_LIST_LEN] = long_list[i];
    }

    printf("\nShort list length = %zu\n", SHORT_LIST_LEN);
    time_sort(short_list, SHORT_LIST_LEN);

    printf("\nMedium list length = %zu\n", MIDDLE_LIST_LEN);
    time_sort(mid_list, MIDDLE_LIST_LEN);

    printf("\nLong list length = %zu\n", LONG_LIST_LEN);
    time_sort(long_list, LONG_LIST_LEN);

    free(short_list);
    free(mid_list);
    free(long_list);
    return EXIT_SUCCESS;

fail_all:
    free(long_list);

fail_long:
    free(mid_list);

fail_mid:
    free(short_list);

    return EXIT_FAILURE;
}
