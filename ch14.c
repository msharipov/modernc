/*
*   TODO:
*   - custom comparison function = DONE
*   - generic merge sort = DONE
*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define NAME_LEN 20

typedef struct Person Person;
struct Person {
    
    char name[NAME_LEN + 1];
    uint8_t age;
};


void
Person_init(Person* const p, const char* name, const uint8_t age) {

    p->age = age;
    strncpy(p->name, name, NAME_LEN);
    p->name[NAME_LEN] = '\0';
}


void
Person_print(const Person* const p) {

    printf("%s, %" PRIu8 "\n", p->name, p->age);
}


int
compare_name(const void* a, const void* b) {

    const Person* A = a;
    const Person* B = b;
    return strncmp(A->name, B->name, NAME_LEN);
}


int
compare_age(const void* a, const void* b) {

    const Person* A = a;
    const Person* B = b;
    return A->age - B->age;
}


int
gen_mergesort(void* arr, const size_t len, const size_t size,
              int(*comp)(const void*, const void*)) {
    
    if (len == 1) {

        return 0;
    }

    const size_t len_left = len/2;
    const size_t len_right = (len + 1)/2;
    uint8_t* left = arr;
    uint8_t* right = (uint8_t*)arr + size*len_left;

    if (gen_mergesort(arr, len_left, size, comp) ||
        gen_mergesort(right, len_right, size, comp)) {

        return 1;
    }
    
    uint8_t* new_arr = (uint8_t*)malloc(len*size);
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
main() {
    
    Person users[8] = {0};
    Person_init(&users[0], "Eve", 40);
    Person_init(&users[1], "David", 32);
    Person_init(&users[2], "Alice", 31);
    Person_init(&users[3], "Bob", 25);
    Person_init(&users[4], "Cindy", 48);
    Person_init(&users[5], "George", 39);
    Person_init(&users[6], "Fiona", 22);
    Person_init(&users[7], "Herbert", 43);
    
    printf("Unsorted:\n");
    for(size_t i = 0; i < 8; i++) {

        Person_print(&users[i]);
    }
    
    if (gen_mergesort(users, 8, sizeof(Person), compare_name)) {
        
        fprintf(stderr, "Sorting by name failed!\n");
        return EXIT_FAILURE;
    }
    printf("\nSorted by name:\n");
    for(size_t i = 0; i < 8; i++) {

        Person_print(&users[i]);
    }
    
    if (gen_mergesort(users, 8, sizeof(Person), compare_age)) {

        fprintf(stderr, "Sorting by age failed!\n");
        return EXIT_FAILURE;
    }
    printf("\nSorted by age:\n");
    for(size_t i = 0; i < 8; i++) {

        Person_print(&users[i]);
    }

    return EXIT_SUCCESS;
}
