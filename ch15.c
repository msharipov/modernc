/*  Copyright 2024 msharipovr@gmail.com
 *
 *   TODO:
 *   - search for a word
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

#define MAIN_BUF_LEN 250
#define FILENAME_LEN 256
#define PATTERN_LEN 64

// Returns the starting index of the substring {word} in {buf}. If the word
// is not in the buffer, returns the length of the buffer instead.
size_t
find_word(const size_t buf_len, const char *const buf,
                 const char *const word, const size_t start) {

    size_t word_len = strlen(word);
    for (size_t i = 0; i + word_len < buf_len; i++) {

        if (strncmp(&buf[i], word, word_len)) {

            continue;

        } else {

            return i;
        }
    }

    return buf_len;
}


int
main(int argc, char* argv[]) {

    if (argc != 3) {

        fprintf(stderr, "Usage: %s PATTERN FILE\n", argv[0]);
        return EXIT_FAILURE;
    }

    char pattern[PATTERN_LEN + 1] = {0};
    strncpy(pattern, argv[1], PATTERN_LEN);

    char filename[FILENAME_LEN] = {0};
    strncpy(filename, argv[2], FILENAME_LEN);

    FILE* file = fopen(filename, "r");
    if (!file) {

        fprintf(stderr, "Could not open file %s\n", filename);
    }
    
    char* buffer = malloc(MAIN_BUF_LEN);
    if (!buffer) {
        
        fprintf(stderr, "Buffer memory allocation failed!\n");
        return(EXIT_FAILURE);
    }

    

    char sampletext[] = "";

    free(buffer);
    return EXIT_SUCCESS;
}
