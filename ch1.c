// Implements quicksort and mergesort

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void print_array(const double * const arr, const size_t n) {
    
    for (size_t i = 0; i < n; i++) {
        printf("[%zu]: %f\n", i, arr[i]);
    }
}


void fill_rand(double * const arr, const size_t n, const unsigned seed) {
    
    srand(seed);
    for (size_t i = 0; i < n; i++) {
        arr[i] = (double)(rand()) / RAND_MAX;
    }
}


bool is_sorted(const double * arr, const size_t n) {

    for (size_t i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }

    return true;
}


void swap(double * const arr, const size_t a, const size_t b) {

    const double temp = arr[a];
    arr[a] = arr[b];
    arr[b] = temp;
}


void quick_sort(double * const arr, const size_t len) {
    
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
            quick_sort(arr, j);
            quick_sort(&arr[i], len - i);
            return;
        }

    }

}


void merge_sort(double * const arr, const size_t len) {
    
    if (len < 3) {
        if (len == 2 && arr[0] > arr[1]) {
            swap(arr, 0, 1);
        }
        return;
    }
    
    size_t middle = len / 2 ;
    merge_sort(arr, middle);
    merge_sort(&arr[middle], len - middle);

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


int main(const int argc, const char * argv[]) {
    
    uint16_t seed = 1;
    
    if (argc == 2) {
        char seed_str[15];
        strncpy(seed_str, argv[1], 14);
        char * end = NULL;
        seed = strtoul(seed_str, &end, 10) % UINT16_MAX;
    }

    double arr[15] = {0};
    fill_rand(arr, 15, seed);
    quick_sort(arr, 15);
    print_array(arr, 15);
    if (is_sorted(arr, 15)) {
        printf("Sorted correctly!\n");
    } else {
        printf("Not sorted correctly!\n");
    }

    fill_rand(arr, 15, seed + 27);
    merge_sort(arr, 15);
    print_array(arr, 15);
    if (is_sorted(arr, 15)) {
        printf("Sorted correctly!\n");
    } else {
        printf("Not sorted correctly!\n");
    }

    return 0;
}
