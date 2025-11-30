#include "image_processing.h"
#include <string.h>
#include <stdlib.h>

// helper function for qsort
int compare_uint8(const void *a, const void *b) {
    uint8_t val_a = *(uint8_t*)a;
    uint8_t val_b = *(uint8_t*)b;
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}


void apply_median_filter(BMPImage *img, int mask_size) {
    int width = img->width;
    int height = img->height;
    int row_size = ((width * 3 + 3) / 4) * 4;
    int half = mask_size / 2;

    // make a copy of original image
    uint8_t *original = (uint8_t*)malloc(row_size * height);
    memcpy(original, img->data, row_size * height);

    // array to store mask values
    uint8_t *mask_values = (uint8_t*)malloc(mask_size * mask_size * sizeof(uint8_t));

    // process each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // for each channel (B, G, R)
            for (int channel = 0; channel < 3; channel++) {
                int count = 0;

                // collect values from mask area
                for (int dy = -half; dy <= half; dy++) {
                    for (int dx = -half; dx <= half; dx++) {
                        int ny = y + dy;
                        int nx = x + dx;

                        // check bounds
                        if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                            int idx = ny * row_size + nx * 3 + channel;
                            mask_values[count++] = original[idx];
                        }
                    }
                }

                // sort and get median
                qsort(mask_values, count, sizeof(uint8_t), compare_uint8);
                uint8_t median = mask_values[count / 2];

                // apply median value
                int idx = y * row_size + x * 3 + channel;
                img->data[idx] = median;
            }
        }
    }

    free(mask_values);
    free(original);
}


void convert_to_grayscale(BMPImage *img) {
    int width = img->width;
    int height = img->height;
    int row_size = ((width * 3 + 3) / 4) * 4;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;

            uint8_t B = img->data[idx];
            uint8_t G = img->data[idx + 1];
            uint8_t R = img->data[idx + 2];

            // standard grayscale formula
            uint8_t gray = (uint8_t)(0.299 * R + 0.587 * G + 0.114 * B);

            img->data[idx] = gray;     // B
            img->data[idx + 1] = gray; // G
            img->data[idx + 2] = gray; // R
        }
    }
}

void equalize_histogram(BMPImage *img) {
    int width = img->width;
    int height = img->height;
    int row_size = ((width * 3 + 3) / 4) * 4;

    // calculate histogram (using one channel since it's grayscale)
    int histogram[256] = {0};
    int total_pixels = width * height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];
            histogram[gray]++;
        }
    }

    // calculate cumulative histogram
    int cumulative[256];
    cumulative[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cumulative[i] = cumulative[i - 1] + histogram[i];
    }

    // apply equalization
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];

            // calculate new equalized value
            uint8_t new_value = (uint8_t)((cumulative[gray] * 255.0) / total_pixels);

            // apply to all three channels
            img->data[idx] = new_value;
            img->data[idx + 1] = new_value;
            img->data[idx + 2] = new_value;
        }
    }
}

