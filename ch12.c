/* 
*   TODO
*   - implement linked list of strings = DONE
*   - split blob
*   - join consecutive blobs
*   - turn text into blobs
*   - rearrange into lines
*   - print blobs = DONE
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOB_SIZE 100

typedef struct list_node list_node;
struct list_node {
    list_node* prev;
    list_node* next;
    char content[BLOB_SIZE + 1];
};


list_node*
create_node(const char* const src, const size_t len) {
    
    list_node* blob = (list_node*)malloc(sizeof(list_node));
    if (!blob || len > BLOB_SIZE) {

        return (void*)0;
    }

    blob->prev = (void*)0;
    blob->next = (void*)0;

    strncpy(blob->content, src, len);
    
    return blob;
}


list_node*
attach_node(list_node* const parent, const char* const src, const size_t len) {
    
    list_node* new_node = create_node(src, len);
    if (!new_node) {

        return (void*)0;
    }

    parent->next = new_node;
    new_node->prev = parent;

    return new_node;
}


list_node*
find_head(list_node* current) {
    
    while (current->prev) {

        current = current->prev;
    }

    return current;
}


list_node*
find_tail(list_node* current) {

    while (current->next) {

        current = current->next;
    }

    return current;
}


void
free_list(list_node* head) {

    while (head->next) {

        head = head->next;
        free(head->prev);
    }
    free(head);
}


void
print_as_lines(const list_node* const start) {
    
    const list_node* current = start;
    while (current) {

        printf("%s\n", current->content);
        current = current->next;
    }
}


int
main() {
    

    return EXIT_SUCCESS;
}
