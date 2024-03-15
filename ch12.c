/* 
*   TODO
*   - implement linked list of strings = DONE
*   - split blob = DONE
*   - join consecutive blobs = DONE
*   - turn text into blobs = DONE
*   - rearrange into lines = DONE
*   - print blobs = DONE
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOB_SIZE 50

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

    strncpy(blob->content, src, BLOB_SIZE);
    blob->content[BLOB_SIZE] = '\0';
    
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
remove_node(list_node* node) {

    if (node->prev) {

        if (node->next) {

            node->prev->next = node->next;
            node->next->prev = node->prev;

        } else {

            node->prev->next = (void*)0;

        }

    } else if (node->next) {

        node->next->prev = (void*)0;

    }

    free(node);
}


void
free_list(list_node* head) {

    if (head->prev) {

        head->prev->next = 0;
    }

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


list_node*
load_text(const char* src, const size_t len) {
    
    size_t read = 0;

    if (len <= BLOB_SIZE) {

        return create_node(src, len);
    }

    list_node* const head = create_node(src, BLOB_SIZE);
    read += BLOB_SIZE;
    list_node* tail = head;

    while (len - read && src[read]) {
        
        size_t attach = (len - read > BLOB_SIZE) ? BLOB_SIZE : len - read;
        tail = attach_node(tail, &src[read], attach);
        read += attach;
        if (!tail) {
            
            free_list(head);
            return (void*)0;
        }
    }

    return head;
}


list_node*
condense(list_node* const first) {

    list_node* second = first->next;
    
    if (!second) {
        
        return (void*)0;
    }
    
    size_t len1 = strlen(first->content);
    size_t len2 = strlen(second->content);
    size_t free_space = BLOB_SIZE - len1;
    
    strncpy(&(first->content)[len1], second->content, free_space);

    if (len2 <= free_space) {
        
        remove_node(second);

    } else {

        char* dest = second->content;
        char* src = &dest[free_space];
        while ((*dest++ = *src++));
    }

    return first;
}


bool
node_contains(const list_node* const node, const char c) {

    return strchr(node->content, c);
}


list_node*
split_node(list_node* const node, const char divider) {

    char* divider_pos = strchr(node->content, divider);
    if (!divider_pos) {

        return (void*)0;
    }

    if (divider_pos == &(node->content)[BLOB_SIZE - 1]) {

        *divider_pos = '\0';
        return node;
    }

    if (divider_pos == node->content) {

        char* dest = divider_pos;
        char* src = dest + 1;
        while((*dest++ = *src++));
        return node;
    }
    
    list_node* new_node = create_node(divider_pos + 1, strlen(divider_pos + 1));
    if (!new_node) {

        return (void*)0;
    }
    *divider_pos = '\0';

    if (node->next) {

        node->next->prev = new_node;
        new_node->next = node->next;
    }

    node->next = new_node;
    new_node->prev = node;

    return node;
}


void
arrange_into_lines(list_node* blobs) {

    while (blobs->next) {

        condense(blobs);
        if (node_contains(blobs, '\n')) {
            
            split_node(blobs, '\n');
            blobs = blobs->next;
        }
    }
} 


int
main() {
    
    char limerick[300] =
        "There was a young lady named Bright,\n"
        "Whose speed was far faster than light.\n"
        "She set out one day\n"
        "In a relative way\n"
        "And returned on the previous night.\n";
    
    list_node* blobs = load_text(limerick, 300);
    if (!blobs) {

        fprintf(stderr, "Failed to load text!\n");
        return EXIT_FAILURE;
    }
    
    print_as_lines(blobs);
    arrange_into_lines(blobs);
    print_as_lines(blobs);    

    free_list(blobs);
    return EXIT_SUCCESS;
}
