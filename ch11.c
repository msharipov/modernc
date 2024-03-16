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

#define IM_MAX_HEIGHT 2000
#define IM_MAX_WIDTH 2000
#define IM_NAME_LEN 255
#define IM_DEPTH 255
#define HEADER_BUF_LEN 100
#define NUM_STR_LEN 20

typedef struct RGB_Pixel RGB_Pixel;
struct RGB_Pixel {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

typedef union Pixel Pixel;
union Pixel {
    RGB_Pixel rgb;
    uint8_t gray;
};

typedef struct Pixel_Sum Pixel_Sum;
struct Pixel_Sum {
    uint64_t R;
    uint64_t G;
    uint64_t B;
    uint64_t gray;
};

typedef struct Image_Header Image_Header;
struct Image_Header {
    int16_t format;
    size_t height;
    size_t width;
};

typedef struct Region_Data Region_Data;
struct Region_Data {
    size_t count;
    Pixel_Sum sum;
    Pixel mean;
};


int
read_netpbm_header(Image_Header* const head, FILE* im) {
    
    char buffer[HEADER_BUF_LEN] = {0};

    if (!fgets(buffer, HEADER_BUF_LEN, im) || feof(im)) {

        return 1;
    }
    int16_t file_format = buffer[1] - '0';
    if (file_format != 5 && file_format != 6) {

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
read_pgm_image(Pixel data[], const size_t height, const size_t width,
               FILE* im) {
    
    if (feof(im)) {

        return 1;
    }

    size_t bytes_target = width * height;
    uint8_t* temp = (uint8_t*)malloc(sizeof(uint8_t) * bytes_target);

    if (!temp) {

        return 1;
    }
    
    if (bytes_target != fread(temp, sizeof(uint8_t), bytes_target, im)) {
        
        free(temp);
        return 1;
    }

    for (size_t i = 0; i < bytes_target; i++) {

        data[i].gray = temp[i];
    }

    free(temp);
    return 0;
}


int
read_ppm_image(Pixel data[], const size_t height, const size_t width,
               FILE* im) {

    if (feof(im)) {

        return 1;
    }

    size_t bytes_target = 3 * width * height;
    uint8_t* temp = (uint8_t*)malloc(sizeof(uint8_t) * bytes_target);

    if (!temp) {

        return 1;
    }
    
    if (bytes_target != fread(temp, sizeof(uint8_t), bytes_target, im)) {
        
        free(temp);
        return 1;
    }
    
    const size_t pixels = width * height;
    for (size_t i = 0; i < pixels; i++) {

        data[i].rgb = (RGB_Pixel) {temp[3*i], temp[3*i + 1], temp[3*i + 2]};
    }

    free(temp);
    return 0;
}


int
write_netpbm_image(FILE* out, const Image_Header* head, const Pixel data[]) {
    
    size_t total = head->height * head->width;

    if (head->format == 5) { 

        for (size_t i = 0; i < total; i++) {

            if (fputc(data[i].gray, out) == EOF) {
                
                return 1;
            }
        }

    } else if (head->format == 6) {

        for (size_t i = 0; i < total; i++) {
                
            if (fputc(data[i].rgb.R, out) == EOF ||
                fputc(data[i].rgb.G, out) == EOF ||
                fputc(data[i].rgb.B, out) == EOF) {

                return 1;
            }
        }
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


Pixel
merge_regions_pgm(size_t region[], Region_Data reg_data[], const size_t x,
                  const size_t y) {
    
    const size_t x_reg = get_reg(region, x);
    const size_t y_reg = get_reg(region, y);
    if (x_reg == y_reg) {

        return reg_data[x_reg].mean;
    }

    Region_Data* y_data = &reg_data[y_reg];
    Region_Data* x_data = &reg_data[x_reg];

    x_data->sum.gray += y_data->sum.gray;
    x_data->count += y_data->count;
    x_data->mean.gray = x_data->sum.gray / x_data->count;

    region[y_reg] = x_reg;

    return x_data->mean;
}


Pixel
merge_regions_ppm(size_t region[], Region_Data reg_data[], const size_t x,
                  const size_t y) {
    
    const size_t x_reg = get_reg(region, x);
    const size_t y_reg = get_reg(region, y);
    if (x_reg == y_reg) {

        return reg_data[x_reg].mean;
    }

    Region_Data* y_data = &reg_data[y_reg];
    Region_Data* x_data = &reg_data[x_reg];

    x_data->sum.R += y_data->sum.R;
    x_data->sum.G += y_data->sum.G;
    x_data->sum.B += y_data->sum.B;
    x_data->count += y_data->count;
    x_data->mean.rgb.R = x_data->sum.R / x_data->count;
    x_data->mean.rgb.G = x_data->sum.G / x_data->count;
    x_data->mean.rgb.B = x_data->sum.B / x_data->count;

    region[y_reg] = x_reg;

    return x_data->mean;
}


void
segment_pgm(size_t region[], Region_Data reg_data[], const Pixel image[],
            const Image_Header* const head, const unsigned long tolerance) {
    
    const size_t rows = head->height;
    const size_t cols = head->width;
    const size_t image_size = rows * cols;
    
    for (size_t i = 0; i < image_size; i++) {
        
        region[i] = i;
        reg_data[i] = (Region_Data) {
            .count = 1,
            .sum = (Pixel_Sum) {.gray = image[i].gray},
            .mean = image[i]
        };
    }

    for (size_t row = 0; row < rows;) {
        
        bool done = true;
        for (size_t col = 0; col < cols; col++) {
            
            size_t i = row * cols + col;
            Pixel mean = reg_data[get_reg(region, i)].mean;

            if (row && !same_reg(region, i, i - cols)) {

                size_t top = i - cols;
                Pixel top_mean = reg_data[get_reg(region, top)].mean;
                size_t diff = (mean.gray > top_mean.gray) ? 
                    mean.gray - top_mean.gray : top_mean.gray - mean.gray;
                if (diff <= tolerance) {
                    
                    done = false;
                    mean = merge_regions_pgm(region, reg_data, top, i);
                }
            }

            if (col && !same_reg(region, i, i - 1)) {
                
                size_t left = i - 1;
                Pixel left_mean = reg_data[get_reg(region, left)].mean;
                size_t diff = (mean.gray > left_mean.gray) ?
                    mean.gray - left_mean.gray : left_mean.gray - mean.gray;
                if (diff <= tolerance) {

                    done = false;
                    mean = merge_regions_pgm(region, reg_data, left, i);
                }
            }
        }

        if (done) {
            
            row++;
        }
    }
}


size_t
RGB_dist(const RGB_Pixel* const a, const RGB_Pixel* const b) {

    size_t total = 0;
    total += (a->R > b->R) ? a->R - b->R : b->R - a->R;
    total += (a->G > b->G) ? a->G - b->G : b->G - a->G;
    total += (a->B > b->B) ? a->B - b->B : b->B - a->B;

    return total;
}


void
segment_ppm(size_t region[], Region_Data reg_data[], const Pixel image[],
            const Image_Header* const head, const unsigned long tolerance) {
    
    const size_t rows = head->height;
    const size_t cols = head->width;
    const size_t image_size = rows * cols;
    
    for (size_t i = 0; i < image_size; i++) {
        
        region[i] = i;
        reg_data[i] = (Region_Data) {
            .count = 1,
            .sum = (Pixel_Sum) {
                .R = image[i].rgb.R,
                .G = image[i].rgb.G,
                .B = image[i].rgb.B
            },
            .mean = image[i]
        };
    }

    for (size_t row = 0; row < rows;) {
        
        bool done = true;
        for (size_t col = 0; col < cols; col++) {
            
            size_t i = row * cols + col;
            Pixel mean = reg_data[get_reg(region, i)].mean;

            if (row && !same_reg(region, i, i - cols)) {

                size_t top = i - cols;
                Pixel top_mean = reg_data[get_reg(region, top)].mean;
                size_t diff = RGB_dist(&mean.rgb, &top_mean.rgb); 
                if (diff <= tolerance) {
                    
                    done = false;
                    mean = merge_regions_ppm(region, reg_data, top, i);
                }
            }

            if (col && !same_reg(region, i, i - 1)) {
                
                size_t left = i - 1;
                Pixel left_mean = reg_data[get_reg(region, left)].mean;
                size_t diff = RGB_dist(&mean.rgb, &left_mean.rgb);
                if (diff <= tolerance) {

                    done = false;
                    mean = merge_regions_ppm(region, reg_data, left, i);
                }
            }
        }

        if (done) {
            
            row++;
        }
    }
}


void
flatten(Pixel image[], const Image_Header* const head, size_t region[],
        const Region_Data reg_data[]) {

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
    
    printf("Reading the header...\n");

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
    
    printf("Loading the source image...\n");

    Pixel* image = (Pixel*)malloc(sizeof(Pixel) * head.width * head.height);
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

    FILE* out = (void*)0;

    switch (head.format) {
        
        case 5:

            if (read_pgm_image(image, head.height, head.width, in)) {

                fprintf(stderr, "Failed to read the image data.\n");
                goto fail_reg_data;
            }

            out = fopen("pictures/out.pgm", "wb");
            break;

        case 6:
            
            if (read_ppm_image(image, head.height, head.width, in)) {

                fprintf(stderr, "Failed to read the image data.\n");
                goto fail_reg_data;
            }

            out = fopen("pictures/out.ppm", "wb");
            break;

        default:
            
            fprintf(stderr, "Cannot read this netpbm format.\n");
            goto fail_reg_data;
            
    }

    if (!out) {

        fprintf(stderr, "Failed to open the output file.\n");
        goto fail_reg_data;
    }

    printf("Applying POSTERIZE filter...\n");
    
    switch (head.format) {
    
        case 5:
            segment_pgm(region, reg_data, image, &head, tolerance);
            break;

        case 6:
            segment_ppm(region, reg_data, image, &head, tolerance);
            break;
    }

    flatten(image, &head, region, reg_data);
    
    printf("Saving the result...\n");

    if (write_netpbm_header(out, &head)) {

        fprintf(stderr, "Failed to write the image header to the output.\n");
        goto fail_out;
    }

    if (write_netpbm_image(out, &head, image)) {

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

