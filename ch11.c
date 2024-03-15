/*
*   TODO:
*   - partition greyscale image into regions
*   - region merging
*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define IM_MAX_HEIGHT 400
#define IM_MAX_WIDTH 400
#define IM_NAME_LEN 255
#define IM_DEPTH 255
#define HEADER_BUF_LEN 100
#define IM_READ_BUFFER_LEN 100

typedef struct Image_Header Image_Header;
struct Image_Header {
    int16_t format;
    size_t height;
    size_t width;
};

typedef struct Region_Data Region_Data;
struct Region_Data {
    size_t count;
    uint64_t sum;
};


int
read_netpbm_header(Image_Header* const head, FILE* im) {
    
    char buffer[HEADER_BUF_LEN] = {0};

    if (!fgets(buffer, HEADER_BUF_LEN, im) || feof(im)) {

        return 1;
    }
    int16_t file_format = buffer[1] - '0';
    if (file_format != 5) {

        return 1;
    }

    if (!fgets(buffer, HEADER_BUF_LEN, im) || feof(im)) {

        return 1;
    }
    char* end = buffer;
    size_t file_width = strtoull(buffer, &end, 10);
    size_t file_height = strtoull(end, &end, 10);
    if (file_width > IM_MAX_WIDTH || file_height > IM_MAX_HEIGHT) {

        return 1;
    }
    
    if (!fgets(buffer, HEADER_BUF_LEN, im) || feof(im)) {

        return 1;
    }
    unsigned long file_depth = strtoul(buffer, &end, 10);
    if (file_depth != IM_DEPTH) {

        return 1;
    }
    
    head->format = file_format;
    head->width = file_width;
    head->height = file_height;

    return 0;
}


int
read_pgm_image(uint8_t data[], const size_t height, const size_t width,
               FILE* im) {
    
    if (feof(im)) {

        return 1;
    }

    size_t bytes_target = width * height;
    size_t bytes_tail = bytes_target % IM_READ_BUFFER_LEN;
    bytes_target -= bytes_tail;

    for (size_t bytes_read = 0; bytes_read < bytes_target; 
         bytes_read += IM_READ_BUFFER_LEN) {
        
        
    }
    return 0;
}


int
main (int argc, char* argv[]) {
    
    if (argc != 2) {
        
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return EXIT_FAILURE;
    } 
    
    char im_name[IM_NAME_LEN + 1] = {0};
    strncpy(im_name, argv[1], IM_NAME_LEN);
    FILE* im = fopen(im_name, "r");
    if (!im) {

        fprintf(stderr, "Failed to open file %s\n", im_name);
        return EXIT_FAILURE;
    }

    Image_Header head = {0};
    if (read_netpbm_header(&head, im)) {

        perror("Incompatible file header.\n");
        goto fail_close;
    }

    fpos_t image_data;
    if (fgetpos(im, &image_data)) {

        perror("Failed reading the position in file.\n");
        goto fail_close;
    }
    fflush(im);
    im = freopen(im_name, "rb", im);
    if (!im) {

        perror("Failed reopening the file for binary input.\n");
        return EXIT_FAILURE;
    }
    if (fsetpos(im, &image_data)) {
        
        perror("Failed to restore position in the file.\n");
        goto fail_close;
    }

    uint8_t* image = (uint8_t*)malloc(sizeof(uint8_t) * 
                                      head.width * head.height);
    if (!image) {

        perror("Failed to allocate memory.\n");
        goto fail_close;
    }

    size_t* region = (size_t*)malloc(sizeof(size_t) *
                                     head.width * head.height);
    if (!region) {

        perror("Failed to allocate memory.\n");
        goto fail_image;
    }

    Region_Data* reg_data = (Region_Data*)malloc(sizeof(Region_Data) *
                                                 head.width * head.height);
    if (!reg_data) {

        perror("Failed to allocate memory.\n");
        goto fail_region;
    }

    if (read_pgm_image(image, head.height, head.width, im)) {

        perror("Failed to read the image data.\n");
        goto fail_region;
    }


    
    free(reg_data);
    free(region);
    free(image);
    fclose(im);
    return EXIT_SUCCESS;

fail_region:
    free(region);

fail_image:
    free(image);

fail_close:
    fclose(im);
    return EXIT_FAILURE;
}

