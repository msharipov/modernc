/* 
*   TODO:
*   - extend text processor to use wide characters = DONE
*   - add regex search/replace
*   - add regex grouping
*/

#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BLOB_SIZE 100
#define PATTERN_LEN 128
#define REPLACEMENT_LEN PATTERN_LEN
#define MAX_RANGES 100

enum RegexType {
    EXACT = 0,
    CLASS = 1,
};

enum LinesError {
    LINES_SUCCESS = 0,
    LINES_TOOLONG = 1,
    LINES_MEMFAIL = 2,
};

enum RegexEscape {
    UNESCAPED = 0,
    ESCAPED = 1,
};

typedef struct list_node list_node;
struct list_node {
    list_node* prev;
    list_node* next;
    wchar_t content[BLOB_SIZE + 1];
};

typedef struct IndexRange IndexRange;
struct IndexRange {
    size_t start;
    size_t len;
};

typedef struct WCharRange WCharRange;
struct WCharRange {
    wchar_t first;
    wchar_t last;
};

typedef struct RegexPattern RegexPattern;
struct RegexPattern {
    RegexPattern* next;
    enum RegexType type;
    size_t len; 
    wchar_t exact[BLOB_SIZE + 1];
    WCharRange included[MAX_RANGES];
    size_t included_len;
    WCharRange excluded[MAX_RANGES];
    size_t excluded_len;
};


void
highlight_ranges(const size_t buf_len, const wchar_t buffer[],
                 const IndexRange* ranges) {
    
    size_t old_pos = 0;

    for (; ranges->start < buf_len && ranges->len; ranges++) {

        for (; old_pos < ranges->start; old_pos++) {

            putwc(buffer[old_pos], stderr);
        }

        fwprintf(stderr, L"\x1b[7m");
        size_t range_end = ranges->start + ranges->len;
        for (; old_pos < range_end; old_pos++) {

            putwc(buffer[old_pos], stderr);
        }
        fwprintf(stderr, L"\x1b[0m");
    }
    
    fputws(&buffer[old_pos], stderr);
}


bool
regex_is_special(const wchar_t c) {

    switch(c) {

      case L']':
      case L'[':
      case L'}':
      case L'{':
      case L'?':
      case L'-':
      case L'\\':
      case L'^':
      case L'+':
        return true;

      default:
        return false;
    }
}


void
regex_print(const RegexPattern* regex) {

    while (regex) {

        fwprintf(stderr, L"Type: ");
        if (regex->type == EXACT) {
            
            fwprintf(stderr, L"EXACT, Pattern:\"%ls\"\n", regex->exact);
        
        } else {

            fwprintf(stderr, L"CLASS\n");
        }

        regex = regex->next;
    }
}


void
regex_free(RegexPattern* regex) {

    if (!regex) {
        
        return;
    } 

    while (regex->next) {

        RegexPattern* next = regex->next;
        free(regex);
        regex = next;
    }
    free(regex);
}


RegexPattern*
regex_parse(const wchar_t* const str, const wchar_t** end,
            enum RegexEscape esc) {
    
    const wchar_t c = str[0];
    if (iswcntrl(c)) {

        return (void*)0;
    }

    if (esc == UNESCAPED) switch (c) {

      case L']':
      case L'}':
      case L'{':
      case L'-':
      case L'^':
        return (void*)0;

      case L'\\':
        return regex_parse(&str[1], end, ESCAPED);

      case L'+':
        return regex_parse(&str[1], end, esc);
    }

    RegexPattern* regex = calloc(1, sizeof(RegexPattern));
    if (!regex) {

        return (void*)0;
    }

    // TODO: Handle ? and [] as classes
    if (c == L'?') {

        regex->type = CLASS;

    }
    
    regex->type = EXACT;
    size_t i = 0;
    size_t len = 0;
    for (; !iswcntrl(str[i]); i++) {
        
        if (esc == ESCAPED) {

            regex->exact[i] = str[i];
            len++;
            esc = UNESCAPED;
            continue;
        }

        if (str[i] == L'\\') {

            esc = ESCAPED;
            continue;
        }

        if (regex_is_special(str[i])) {

            break;
        }

        regex->exact[i] = str[i];
        len++;
    }

    *end = &str[i];
    regex->len = len;
    return regex;
}


RegexPattern*
regex_from_str(const wchar_t* str) {
    
    size_t len = wcslen(str);
    const wchar_t* end = str;
    RegexPattern* head = regex_parse(end, &end, UNESCAPED);
    if (!head) {

        return head;
    }
    
    RegexPattern* tail = head;
    while ((end - str) < len) {
    
        RegexPattern* next_reg = regex_parse(end, &end, UNESCAPED);
        if (!next_reg) {

            return head;
        }
        tail->next = next_reg;
        tail = next_reg;
    }

    return head;
}


size_t
regex_match(const size_t str_len, const wchar_t* str,
            const RegexPattern* regex, size_t pos) {
    
    size_t start = pos;
    while (regex) {

        if (pos >= str_len) {

            return 0;
        }
        
        if (regex->type == EXACT) {

            if (wcsncmp(&str[pos], regex->exact, regex->len)) {
            
                return 0;
            
            } else {

                pos += regex->len;
            }
        }

        // TODO: implement class matching
        regex = regex->next;
    }

    return pos - start;
}


IndexRange
regex_next_in_str(const size_t str_len, const wchar_t* str,
                   const RegexPattern* regex, size_t start) {
    
    IndexRange span = {0};

    if (!regex) {

        return span;
    }

    for (; start < str_len; start++) {

        size_t match_len = regex_match(str_len, str, regex, start);
        if (match_len) {

            span.start = start;
            span.len = match_len;
            return span;
        }
    }

    return span;
}


void
regex_search_str(const wchar_t* str, const RegexPattern* regex,
                 IndexRange* const matches, const size_t max_matches) {

    size_t count = 0;
    size_t str_len = wcslen(str);
    size_t pos = 0;

    while (pos < str_len && count < max_matches) {
    
        IndexRange match = regex_next_in_str(str_len, str, regex, pos);

        matches[count].len = match.len;
        matches[count].start = match.start;
        if (!match.len) {

            return;
        }
    
        pos = match.len + match.start;
        count++;
    } 
}


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

    if (parent) {
        
        parent->next = new_node;
    }
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
    if (!head) {

        return (void*)0;
    }

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
load_from_file(FILE* input) {
    
    wchar_t buffer[BLOB_SIZE + 1] = {0};
    wchar_t c = 0;
    size_t read = 0;
    list_node* tail = NULL;

    while ((c = fgetwc(input)) != EOF) {

        buffer[read] = c;
        read++;
        if (read == BLOB_SIZE) {

            read = 0;
            list_node* old_tail = tail;
            tail = attach_node(old_tail, buffer, BLOB_SIZE);
            if (!tail) {

                free_list(find_head(old_tail));
                return (void*)0;
            }
        }
    }
    
    tail = attach_node(tail, buffer, read);

    return find_head(tail);
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


enum LinesError
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


void
search_text(const list_node* text, const RegexPattern* regex) {
    
    IndexRange matches[BLOB_SIZE + 1] = {0};
    for (; text; text = text->next) {
    
        regex_search_str(text->content, regex, matches, BLOB_SIZE);
        highlight_ranges(wcslen(text->content), text->content, matches);
        fwprintf(stderr, L"\n");
    }
}


int
main(int argc, char* argv[]) {

    if (argc != 2 && argc != 3) {
        
        fprintf(stderr, "Missing a search regex!\n");
        return EXIT_FAILURE;
    }
    
    setlocale(LC_ALL, "");

    list_node* blobs = load_from_file(stdin);
    if (!blobs) {

        fprintf(stderr, "Failed to load text!\n");
        return EXIT_FAILURE;
    }
    
    wchar_t regex_str[BLOB_SIZE + 1] = {0};
    char regex_raw[4*(BLOB_SIZE + 1)] = {0};
    strncpy(regex_raw, argv[1], 4*(BLOB_SIZE + 1));
    mbstowcs(regex_str, regex_raw, BLOB_SIZE);
    RegexPattern* regex = regex_from_str(regex_str);
    if (!regex) {

        fprintf(stderr, "Could not parse the search regex!\n");
        goto FAIL;
    }
    regex_print(regex);
    
    switch (arrange_into_lines(blobs)) {
      
      case LINES_SUCCESS:
        break;

      case LINES_TOOLONG:
        fprintf(stderr, "One of the lines is too long "
                        "(>%d characters)!\n", BLOB_SIZE);
        goto FAIL_REGEX;

      case LINES_MEMFAIL:
        fprintf(stderr, "Memory allocation failure.\n");
        goto FAIL_REGEX;
    }
    
    fwprintf(stderr, L"\n");
    if (argc == 2) {

        search_text(blobs, regex);
    }
    
    regex_free(regex);
    free_list(blobs);
    return EXIT_SUCCESS;
  
  FAIL_REGEX:
    regex_free(regex);

  FAIL:
    free_list(blobs);
    return EXIT_FAILURE;
}
