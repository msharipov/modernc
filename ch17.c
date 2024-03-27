/* 
*   TODO:
*   - extend text processor to use wide characters = DONE
*   - add regex search/replace
*   - add regex grouping
*/

#include <wchar.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BLOB_SIZE 100

enum Lines_Error {
    LINES_SUCCESS = 0,
    LINES_TOOLONG = 1,
    LINES_MEMFAIL = 2,
};

typedef struct list_node list_node;
struct list_node {
    list_node* prev;
    list_node* next;
    wchar_t content[BLOB_SIZE + 1];
};


list_node*
create_node(const wchar_t* const src, const size_t len) {
    
    list_node* blob = malloc(sizeof(list_node));
    if (!blob || len > BLOB_SIZE) {

        return (void*)0;
    }

    blob->prev = (void*)0;
    blob->next = (void*)0;

    wcsncpy(blob->content, src, len);
    blob->content[len] = L'\0';
    
    return blob;
}


list_node*
attach_node(list_node* const parent, const wchar_t* const src,
            const size_t len) {
    
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

        wprintf(L"%Ls\n", current->content);
        current = current->next;
    }
}


list_node*
load_text(const wchar_t* const src, const size_t len) {
    
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


size_t
condense(list_node* const first) {

    list_node* second = first->next;
    
    if (!second) {
        
        return 0;
    }
    
    size_t len1 = wcslen(first->content);
    size_t len2 = wcslen(second->content);
    size_t free_space = BLOB_SIZE - len1;
    
    if (!free_space) {

        return BLOB_SIZE;
    }

    wcsncpy(&(first->content)[len1], second->content, free_space);

    if (len2 <= free_space) {
        
        remove_node(second);

    } else {

        wchar_t* dest = second->content;
        wchar_t* src = &dest[free_space];
        while ((*dest++ = *src++));
    }

    return (free_space > len2) ? (len1 + len2) : BLOB_SIZE;
}


bool
node_contains(const list_node* const node, const wchar_t c) {

    return wcschr(node->content, c);
}


list_node*
split_node(list_node** const node_ptr, const wchar_t divider) {
    
    list_node* node = *node_ptr;

    if (!divider) {

        return node;
    }

    wchar_t* divider_pos = wcschr(node->content, divider);
    if (!divider_pos) {

        return node;
    }

    if (divider_pos == node->content) {

        if (!divider_pos[1]) {

            list_node* next = node->next;
            remove_node(node);
            *node_ptr = next;
            return (void*)0;
        }
        
        wchar_t* dest = divider_pos;
        wchar_t* src = dest + 1;
        while ((*dest++ = *src++));
        return node;
    }

    if (!divider_pos[1]) {
        
        *divider_pos = L'\0';
        return node;
    }
    
    list_node* new_node = create_node(divider_pos + 1,
                                      wcslen(divider_pos + 1));
    if (!new_node) {

        return (void*)0;
    }
    *divider_pos = L'\0';

    if (node->next) {

        node->next->prev = new_node;
        new_node->next = node->next;
    }

    node->next = new_node;
    new_node->prev = node;

    return node;
}


enum Lines_Error
arrange_into_lines(list_node* blob) {

    while (blob) {
        
        wchar_t* linebreak = wcschr(blob->content, L'\n');
        if (!linebreak) {

            if (!blob->next) {
                
                return LINES_SUCCESS;
            }
            
            const size_t len = wcslen(blob->content);
            if (len < BLOB_SIZE) {
                
                condense(blob);
                continue;
            }

            if (blob->next->content[0] == L'\n') {

                list_node* next = blob->next;
                if (!split_node(&next, L'\n') && next) {

                    return LINES_MEMFAIL;
                }

                blob = blob->next;
                continue;
            
            } else {

                return LINES_TOOLONG;
            }
        }
        
        if (!split_node(&blob, L'\n')) {

            return (blob) ? LINES_MEMFAIL : LINES_SUCCESS;
        }
        
        if (!wcschr(blob->content, L'\n')) {
            
            blob = blob->next;
        }
    }

    return LINES_SUCCESS;
}


int main() {
    
    setlocale(LC_ALL, "");
    wchar_t text[1000] =
        L"There was a young lady named Bright,\n"
        L"Whose speed was far faster than light.\n"
        L"She set out one day\n"
        L"In a relative way\n"
        L"And returned on the previous night.\n"
        L"\n"
        L"Я сижу в темноте. И она не хуже\n"
        L"В комнате, чем темнота снаружи.\n";

    list_node* blobs = load_text(text, wcslen(text));
    if (!blobs) {

        fprintf(stderr, "Failed to load text!\n");
        return EXIT_FAILURE;
    }
    
    print_as_lines(blobs);
    
    switch (arrange_into_lines(blobs)) {
      
      case LINES_SUCCESS:
        break;

      case LINES_TOOLONG:
        fprintf(stderr, "One of the lines is too long "
                        "(>%d characters)!\n", BLOB_SIZE);
        goto FAIL;

      case LINES_MEMFAIL:
        fprintf(stderr, "Memory allocation failure.\n");
        goto FAIL;
    }

    print_as_lines(blobs);    

    free_list(blobs);
    return EXIT_SUCCESS;

  FAIL:
    free_list(blobs);
    return EXIT_FAILURE;
}
