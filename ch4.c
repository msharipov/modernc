#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

size_t
Find(const size_t parent[], size_t element) {

    while (parent[element] != SIZE_MAX) {
        element = parent[element];
    }

    return element;
}


void
FindReplace(size_t parent[], size_t element, const size_t new_value) {

    while (parent[element] != SIZE_MAX) {
        
        size_t original_value = element;
        element = parent[original_value];
        parent[original_value] = new_value;
    }

    parent[element] = new_value;
}


size_t
FindCompress(size_t parent[], size_t element) {
    
    size_t root = Find(parent, element);

    while (element != root) {

        size_t original_value = element;
        element = parent[element];
        parent[original_value] = root;
    }

    return root;
}


size_t
Union(size_t parent[], size_t A, size_t B) {
    
    size_t new_root = FindCompress(parent, A); 
    FindReplace(parent, B, new_root);
    
    return new_root;
}


int
main() {
    
    size_t PARENT[10] = {1, 2, SIZE_MAX, 1, 3, 4, SIZE_MAX, 8, 6, 8};

    Union(PARENT, 7, 3);
    printf("The root is: %zu\n", Find(PARENT, 5));
    exit(EXIT_SUCCESS);
}

