/*  Copyright 2024 msharipovr@gmail.com
 *
 *   TODO:
 *   - search for a word = DONE
 *   - replace a word
 *   - regex matching
 *   - regex search in a string
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

// Returns the starting index of the substring {word} in {buf}. If the word
// is not in the buffer, returns the length of the buffer instead.
size_t
find_word(const size_t buf_len, const char *const buf,
                 const char *const word, const size_t start) {

    size_t word_len = strlen(word);
    if (!word_len) {

        return buf_len;
    }

    for (size_t i = start; i + word_len < buf_len; i++) {

        if (strncmp(&buf[i], word, word_len)) {

            continue;

        } else {

            return i;
        }
    }

    return buf_len;
}

void
highlight_found(const size_t buf_len, const char buffer[],
                const char pattern[], size_t line) {
    
    bool found_any = false;
    size_t current_pos = find_word(buf_len, buffer, pattern, 0);
    size_t old_pos = 0;
    while (current_pos < buf_len) {
        
        if (!found_any) {

            found_any = true;
            printf("%zu:", line);
        }

        for (; old_pos < current_pos; old_pos++) {

            putc(buffer[old_pos], stdout);
        }
        printf("\x1b[33m%s\x1b[0m", pattern);

        current_pos += strlen(pattern);
        old_pos = current_pos;
        current_pos = find_word(buf_len, buffer, pattern, current_pos);
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

    char buffer[BUFFER_LEN] = {0};
    
    for (size_t line = 1; fgets(buffer, BUFFER_LEN, stdin); line++) {
        
        size_t current_len = strlen(buffer);
        if (current_len + 1 == BUFFER_LEN) {

            fprintf(stderr, "Error! Line %zu is too long.\n", line);
            return EXIT_FAILURE;
        }

        if (!replace) {

            highlight_found(current_len, buffer, pattern, line);
        }
    }

    return EXIT_SUCCESS;
}
