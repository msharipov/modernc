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

enum RangeMode {
    ADD = 0,
    SUBTRACT = 1,
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

void regex_free(RegexPattern* regex);

IndexRange regex_first_match(const size_t buf_len, const char buf[],
                             const RegexPattern* regex, const size_t start);


bool
is_valid_char(char c) {

    return (c >= ' ' && c <= '~');
}


void
regex_fill_class(char* data, const char A, const char B,
                 enum RangeMode mode) {

    char low = (A > B) ? B : A;
    char high = (A > B) ? A : B;

    for (char c = low; c <= high; c++) {
        
        size_t c_index = c - ' ';
        switch (mode){
            
            case ADD:
                data[c_index] = 1;
                break;

            case SUBTRACT:
                data[c_index] = 0;
                break;

            default:
                break;
        }
    }
}


// Returns non-zero if range can't be parsed
int
regex_parse_range(char* data, const char* str, enum RangeMode mode,
                  const bool escaped) {
    
    if (!str) {

        return 1;
    }

    while (str[0] && str[0] >= ' ' && str[0] <= '~') {

        switch (str[0]) {
        
            case '-':
                if (!escaped) {

                    return regex_parse_range(data, &str[1], SUBTRACT, false);
                }
                break;
                
            case '\\':
                if (!escaped) {

                    return regex_parse_range(data, &str[1], mode, true);
                }
                break;

            case ']':
                if(!escaped) {

                    return 0;
                }
        }

        if (str[1] && str[1] == '-') {

            if (str[2] && is_valid_char(str[2])) {

                if (str[2] == '\\') {
                    
                    if (str[3] && is_valid_char(str[3])) {

                        regex_fill_class(data, str[0], str[2], mode);
                        
                    } else {

                        return 1;
                    }

                } else if (is_valid_char(str[2])) {

                    regex_fill_class(data, str[0], str[2], mode);

                } else {

                    return 1;
                }

            } else {

                return 1;
            }
            
        } else {
            
            size_t c_index = str[0] - ' ';
            switch (mode) {

                case ADD:
                    data[c_index] = 1;

                case SUBTRACT:
                    data[c_index] = 0;
            }
        }
    }

    return 1;
}


RegexPattern*
regex_parse(const char* const str) {

    if (str[0] < ' ' || str[0] > '~' || str[0] == ']') {

        return (void*)0;
    }

    RegexPattern* regex = calloc(1, sizeof(RegexPattern));
    if (!regex) {

        return (void*)0;
    }
    
    size_t cur = 0;
    if (str[0] == '\\') {
        
        char next_c = str[1];
        if (next_c < ' ' || next_c > '~') {
            
            regex_free(regex);
            return (void*)0;
        }

        regex->type = EXACT;
        regex->data[0] = next_c;
        regex->len = 2;
        return regex;
 
    } else {

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

            case '[':
                if (regex_parse_range(regex->data, &str[1], ADD, false)) {
                    
                    free(regex);
                    return (void*)0;
                }
                regex->type = RANGE;
                return regex;
                

            default:
                regex->type = EXACT;
        }
    }

    for (;;) {
        
        char c = str[++cur];
        if (c < ' ' || c > '~' || c == '*' || c == '?' || c == '\\' ||
            c == '[') {

            break;
        }
    }
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


// Returns the length of matching string or 0 if doesn't match
size_t
regex_matches(const char buf[], const RegexPattern* const regex) {

    size_t len = strlen(regex->data);
    switch (regex->type) {
                
        case EXACT:
            if (strncmp(buf, regex->data, len)) {

                return 0;

            } else {
                
                return len;
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
    
    fputs(&buffer[old_pos], stdout);
}


int
main(int argc, char* argv[]) {

    if (argc != 2 && argc != 3) {

        fprintf(stderr, "Usage: %s PATTERN [REPLACEMENT]\n", argv[0]);
        return EXIT_FAILURE;
    }

    bool replace = (argc == 3);

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
