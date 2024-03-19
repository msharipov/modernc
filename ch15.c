/* 
 *   TODO:
 *   - search for a word = DONE
 *   - replace a word
 *   - regex matching = DONE
 *   - regex search in a string = DONE
 *   - query-replace with regex
 *   - extend query-replace with grouping
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_LEN 200
#define FILENAME_LEN 256
#define PATTERN_LEN 64
#define REGEX_DATA_LEN 95

enum RegexType {
    EXACT = 0,
    RANGE = 1,
    ANY_CHAR = 2,
    ANY_SPAN = 3,
};

typedef struct RegexPattern RegexPattern;
struct RegexPattern {
    RegexPattern* next;
    enum RegexType type;
    size_t len;
    char data[REGEX_DATA_LEN + 1];
    char range_start;
    char range_end;
};

typedef struct IndexRange IndexRange;
struct IndexRange {
    size_t start;
    size_t len;
};


RegexPattern*
regex_parse(const char* const str) {

    if (str[0] < ' ' || str[0] > '~') {

        return (void*)0;
    }

    RegexPattern* regex = calloc(1, sizeof(RegexPattern));
    if (!regex) {

        return (void*)0;
    }
    
    size_t cur = 0;
    switch (str[0]) {

        case '*':
            while (str[++cur] == '*');
            regex->type = ANY_SPAN;
            regex->len = cur;
            return regex;

        case '?':
            regex->type = ANY_CHAR;
            regex->len = 1;
            return regex;

        default:
            regex->type = EXACT;
    }

    for (; str[cur] && str[cur] != '*' && str[cur] != '?'; cur++);
    strncpy(regex->data, str, cur);
    regex->data[cur] = 0;
    regex->len = cur;

    return regex;
}


RegexPattern*
regex_from_str(const char* str) {
    
    size_t len = strlen(str);
    size_t cur = 0;
    RegexPattern* head = regex_parse(str);
    if (!head) {

        return head;
    }
    
    cur += head->len;
    RegexPattern* tail = head;
    while (cur != len) {

        RegexPattern* next_reg = regex_parse(str + cur);
        if (!next_reg) {

            return head;
        }
        tail->next = next_reg;
        tail = next_reg;
        cur += tail->len;
    }

    return head;
}


void
regex_print(const RegexPattern* regex) {

    while (regex) {

        printf("Type: %d, Length: %zu Data: %s\n",
               regex->type, regex->len, regex->data);
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


IndexRange regex_first_match(const size_t buf_len, const char buf[],
                             const RegexPattern* regex, const size_t start);


// Returns the length of matching string or 0 if doesn't match
size_t
regex_matches(const char buf[], const RegexPattern* const regex) {

    switch (regex->type) {
                
        case EXACT:
            if (strncmp(buf, regex->data, regex->len)) {

                return 0;

            } else {
                
                return regex->len;
            }
            break;

        case ANY_CHAR:
            return (buf[0] >= ' ' && buf[0] <= '~');

        case ANY_SPAN:
            if (!regex->next) {

                return strlen(buf);
            }

            size_t buf_len = strlen(buf);
            IndexRange next_range =
                regex_first_match(buf_len, buf, regex->next, 0);

            if (next_range.start != buf_len) {
                
                return next_range.start;
            }
            return 0;

        default:
            return 0;
            break;
    }
}


IndexRange
regex_first_match(const size_t buf_len, const char buf[],
                  const RegexPattern* regex, const size_t start) {
    
    IndexRange range = {buf_len, 0};
    if (!regex) {
        
        return range;
    }
    
    for (size_t i = start; i < buf_len; i++) {
        
        size_t cur_start = i;
        const RegexPattern* current = regex;
        bool matched = true;
        while (current) {
            
            if (cur_start > buf_len) {

                return range;
            }
            
            size_t cur_len = regex_matches(&buf[cur_start], current);
            if (cur_len) {
                
                cur_start += cur_len;
                current = current->next;        
        
            } else {
                
                matched = false;
                break;
            }
        }

        if (matched) {
            
            range.start = i;
            range.len = cur_start - i;
            return range;
        }
    }

    return range;
}


void
highlight_regex(const size_t buf_len, const char buffer[],
                const RegexPattern* const regex, size_t line) {
    
    bool found_any = false;
    IndexRange match = regex_first_match(buf_len, buffer, regex, 0);

    size_t old_pos = 0;
    while (match.start < buf_len) {

        if (!found_any) {

            found_any = true;
            printf("%zu:", line);
        }

        for (; old_pos < match.start; old_pos++) {

            putc(buffer[old_pos], stdout);
        }
        printf("\x1b[7m");
        size_t match_end = match.start + match.len;
        for (; old_pos < match_end; old_pos++) {

            putc(buffer[old_pos], stdout);
        }
        printf("\x1b[0m");

        match = regex_first_match(buf_len, buffer, regex, old_pos);
    }
    
    if (!found_any) {

        return;
    }
    
    for (; old_pos < buf_len; old_pos++) {

        putc(buffer[old_pos], stdout);
    }
}


int
main(int argc, char* argv[]) {

    if (argc != 2 && argc != 4) {

        fprintf(stderr, "Usage: %s PATTERN [REPLACEMENT OUTPUT]\n", argv[0]);
        return EXIT_FAILURE;
    }

    bool replace = (argc == 4);

    char pattern[PATTERN_LEN + 1] = {0};
    strncpy(pattern, argv[1], PATTERN_LEN);
    RegexPattern* regex = regex_from_str(pattern);
    if (!regex) {

        fprintf(stderr, "Can't parse regular expression: %s\n", pattern);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_LEN] = {0};
    
    for (size_t line = 1; fgets(buffer, BUFFER_LEN, stdin); line++) {
        
        size_t current_len = strlen(buffer);
        if (current_len + 1 == BUFFER_LEN) {

            fprintf(stderr, "Error! Line %zu is too long.\n", line);
            return EXIT_FAILURE;
        }

        if (!replace) {

            highlight_regex(current_len, buffer, regex, line);
        }
    }
    
    regex_free(regex);
    return EXIT_SUCCESS;
}
