/*
*   TODO:
*   - image input/output = DONE
*   - partition greyscale image into regions = DONE
*   - region merging = DONE
*   - flatten image = DONE
*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#define IM_MAX_HEIGHT 1000
#define IM_MAX_WIDTH 1000
#define IM_NAME_LEN 255
#define IM_DEPTH 255
#define HEADER_BUF_LEN 100
#define IM_READ_BLOCK_LEN 100
#define NUM_STR_LEN 20

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
    uint64_t mean;
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
write_netpbm_header(FILE* im, Image_Header* head) {
    
    if (head->format < 1 || head->format > 6) {

        return 1;
    }
    
    // 11 = 3 for 1st row + 4 for 2nd row + 4 for 3rd row
    if (11 > fprintf(im, "P%" PRId16 "\n%zu %zu\n255\n",
                     head->format, head->width, head->height)) {

        return 1;
    }

    return 0;
}


int
read_pgm_image(uint8_t data[], const size_t height, const size_t width,
               FILE* im) {
    
    if (feof(im)) {

        return 1;
    }

    size_t bytes_target = width * height;
    const size_t tail_bytes = bytes_target % IM_READ_BLOCK_LEN;
    bytes_target -= tail_bytes;

    for (size_t bytes_read = 0; bytes_read < bytes_target; 
         bytes_read += IM_READ_BLOCK_LEN) {
        
        if (IM_READ_BLOCK_LEN != fread(&data[bytes_read], sizeof(uint8_t),
                                       IM_READ_BLOCK_LEN, im)) {
            return 1;
        }
    }

    if (tail_bytes != fread(&data[bytes_target], sizeof(uint8_t),
                            tail_bytes, im)) {

        return 1;
    }

    return 0;
}


int
write_pgm_image(FILE* out, const Image_Header* head, const uint8_t data[]) {
    
    size_t total = head->height * head->width;
    if (total > fwrite(data, sizeof(uint8_t), total, out)) {

        return 1;
    }
    
    fflush(out);
    return 0;
}


size_t
get_reg(size_t region[], size_t i) {
    
    size_t parent = region[i];
    if (region[parent] == parent) {
        
        return parent;
    }

    region[i] = get_reg(region, parent);

    return region[i];
}


bool
same_reg(size_t region[], const size_t x, const size_t y) {

    return get_reg(region, x) == get_reg(region, y);
}


size_t
merge_regions(size_t region[], Region_Data reg_data[], const size_t x,
              const size_t y) {
    
    const size_t x_reg = get_reg(region, x);
    const size_t y_reg = get_reg(region, y);
    if (x_reg == y_reg) {

        return reg_data[x_reg].mean;
    }

    Region_Data* y_data = &reg_data[y_reg];
    Region_Data* x_data = &reg_data[x_reg];

    x_data->sum += y_data->sum;
    x_data->count += y_data->count;
    x_data->mean = x_data->sum / x_data->count;

    region[y_reg] = x_reg;

    return x_data->mean;
}


void
segment_pgm(size_t region[], Region_Data reg_data[], const uint8_t image[],
            const Image_Header* const head, const unsigned long tolerance) {
    
    const size_t rows = head->height;
    const size_t cols = head->width;
    const size_t IMAGE_SIZE = rows * cols;
    
    for (size_t i = 0; i < IMAGE_SIZE; i++) {
        
        region[i] = i;
        reg_data[i] = (Region_Data) {
            .count = 1,
            .sum = image[i],
            .mean = image[i]
        };
    }

    for (size_t row = 0; row < rows;) {
        
        bool done = true;
        for (size_t col = 0; col < cols; col++) {
            
            size_t i = row * cols + col;
            size_t mean = reg_data[get_reg(region, i)].mean;

            if (row && !same_reg(region, i, i - cols)) {

                size_t top = i - cols;
                size_t top_mean = reg_data[get_reg(region, top)].mean;
                size_t diff = (mean > top_mean) ? mean - top_mean
                                                : top_mean - mean;
                if (diff <= tolerance) {
                    
                    done = false;
                    mean = merge_regions(region, reg_data, top, i);
                }
            }

            if (col && !same_reg(region, i, i - 1)) {
                
                size_t left = i - 1;
                size_t left_mean = reg_data[get_reg(region, left)].mean;
                size_t diff = (mean > left_mean) ? mean - left_mean
                                                 : left_mean - mean;
                if (diff <= tolerance) {

                    done = false;
                    mean = merge_regions(region, reg_data, left, i);
                }
            }
        }

        if (done) {
            
            row++;
        }
    }
}


void
flatten_pgm(uint8_t image[], const Image_Header* const head,
            size_t region[], const Region_Data reg_data[]) {

    for (size_t i = 0; i < head->width * head->height; i++) {

        image[i] = reg_data[get_reg(region, i)].mean;
    }
}


int
main (int argc, char* argv[]) {
    
    if (argc != 3) {
        
        fprintf(stderr, "Usage: %s FILENAME TOL\n", argv[0]);
        return EXIT_FAILURE;
    }

    unsigned long tolerance = 0;
    char tol_str[NUM_STR_LEN + 1] = {0};
    strncpy(tol_str, argv[2], NUM_STR_LEN);
    char* end = NULL;
    tolerance = strtoul(argv[2], &end, 10);
    if (tolerance < 0 || tolerance > IM_DEPTH) {

        fprintf(stderr, "Invalid tolerance!\n");
        return EXIT_FAILURE;
    }
    
    char im_name[IM_NAME_LEN + 1] = {0};
    strncpy(im_name, argv[1], IM_NAME_LEN);
    FILE* in = fopen(im_name, "r");
    if (!in) {

        fprintf(stderr, "Failed to open file %s\n", im_name);
        return EXIT_FAILURE;
    }

    Image_Header head = {0};
    if (read_netpbm_header(&head, in)) {

        fprintf(stderr, "Incompatible file header.\n");
        goto fail_close;
    }

    fpos_t image_data;
    if (fgetpos(in, &image_data)) {

        fprintf(stderr, "Failed reading the position in file.\n");
        goto fail_close;
    }
    in = freopen(im_name, "rb", in);
    if (!in) {

        fprintf(stderr, "Failed reopening the file for binary input.\n");
        return EXIT_FAILURE;
    }
    if (fsetpos(in, &image_data)) {
        
        fprintf(stderr, "Failed to restore position in the file.\n");
        goto fail_close;
    }

    uint8_t* image = (uint8_t*)malloc(sizeof(uint8_t) * 
                                      head.width * head.height);
    if (!image) {

        fprintf(stderr, "Failed to allocate memory.\n");
        goto fail_close;
    }

    size_t* region = (size_t*)malloc(sizeof(size_t) *
                                     head.width * head.height);
    if (!region) {

        fprintf(stderr, "Failed to allocate memory.\n");
        goto fail_image;
    }

    Region_Data* reg_data = (Region_Data*)malloc(sizeof(Region_Data) *
                                                 head.width * head.height);
    if (!reg_data) {

        fprintf(stderr, "Failed to allocate memory.\n");
        goto fail_region;
    }

    if (read_pgm_image(image, head.height, head.width, in)) {

        fprintf(stderr, "Failed to read the image data.\n");
        goto fail_reg_data;
    }
    
    FILE* out = fopen("pictures/out.pgm", "wb");
    if (!out) {

        fprintf(stderr, "Failed to open the output file.\n");
        goto fail_reg_data;
    }

    segment_pgm(region, reg_data, image, &head, tolerance);
    flatten_pgm(image, &head, region, reg_data);
        
    if (write_netpbm_header(out, &head)) {

        fprintf(stderr, "Failed to write the image header to the output.\n");
        goto fail_out;
    }

    if (write_pgm_image(out, &head, image)) {

        fprintf(stderr, "Failed to write the image data to the output.\n");
        goto fail_out;
    }

    fflush(out);
    fclose(out);
    free(reg_data);
    free(region);
    free(image);
    fclose(in);
    return EXIT_SUCCESS;

    fail_out:
    fflush(out);
    fclose(out);

    fail_reg_data:
    free(reg_data);

    fail_region:
    free(region);

    fail_image:
    free(image);

    fail_close:
    fclose(in);
    return EXIT_FAILURE;
}

