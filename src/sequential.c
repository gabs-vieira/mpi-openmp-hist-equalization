#include "../include/bmp_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int compare_uint8(const void *a, const void *b) {
    return (*(uint8_t*)a - *(uint8_t*)b);
}

void median_filter(Image *input, Image *output, int mask_size) {
    int half = mask_size / 2;
    uint8_t *window = (uint8_t*)malloc(mask_size * mask_size * sizeof(uint8_t));
    
    for (int y = 0; y < input->height; y++) {
        for (int x = 0; x < input->width; x++) {
            int count = 0;
            
            for (int dy = -half; dy <= half; dy++) {
                for (int dx = -half; dx <= half; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    
                    if (ny >= 0 && ny < input->height && nx >= 0 && nx < input->width) {
                        int idx = ny * input->width + nx;
                        window[count++] = input->data[idx].r;
                    }
                }
            }
            
            qsort(window, count, sizeof(uint8_t), compare_uint8);
            uint8_t median = window[count / 2];
            
            int idx = y * input->width + x;
            output->data[idx].r = median;
            output->data[idx].g = median;
            output->data[idx].b = median;
        }
    }
    
    free(window);
}

void to_grayscale(Image *input, Image *output) {
    for (int i = 0; i < input->width * input->height; i++) {
        uint8_t gray = (uint8_t)(0.299 * input->data[i].r + 
                                  0.587 * input->data[i].g + 
                                  0.114 * input->data[i].b);
        output->data[i].r = gray;
        output->data[i].g = gray;
        output->data[i].b = gray;
    }
}

void histogram_equalization(Image *img) {
    int *hist = (int*)calloc(256, sizeof(int));
    
    for (int i = 0; i < img->width * img->height; i++) {
        hist[img->data[i].r]++;
    }
    
    int *cdf = (int*)malloc(256 * sizeof(int));
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + hist[i];
    }
    
    int min_cdf = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            min_cdf = cdf[i];
            break;
        }
    }
    
    int total_pixels = img->width * img->height;
    for (int i = 0; i < img->width * img->height; i++) {
        uint8_t old_val = img->data[i].r;
        uint8_t new_val = (uint8_t)((cdf[old_val] - min_cdf) * 255.0 / (total_pixels - min_cdf));
        img->data[i].r = new_val;
        img->data[i].g = new_val;
        img->data[i].b = new_val;
    }
    
    free(hist);
    free(cdf);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_image.bmp> <output_image.bmp> <mask_size>\n", argv[0]);
        return 1;
    }
    
    int mask_size = atoi(argv[3]);
    if (mask_size % 2 == 0 || mask_size < 3) {
        fprintf(stderr, "Mask size must be odd and >= 3\n");
        return 1;
    }
    
    clock_t start = clock();
    
    Image *input = read_bmp(argv[1]);
    if (!input) {
        return 1;
    }
    
    Image *filtered = (Image*)malloc(sizeof(Image));
    filtered->width = input->width;
    filtered->height = input->height;
    filtered->data = (Pixel*)malloc(filtered->width * filtered->height * sizeof(Pixel));
    
    Image *grayscale = (Image*)malloc(sizeof(Image));
    grayscale->width = input->width;
    grayscale->height = input->height;
    grayscale->data = (Pixel*)malloc(grayscale->width * grayscale->height * sizeof(Pixel));
    
    median_filter(input, filtered, mask_size);
    to_grayscale(filtered, grayscale);
    histogram_equalization(grayscale);
    
    if (!write_bmp(argv[2], grayscale)) {
        free_image(input);
        free_image(filtered);
        free_image(grayscale);
        return 1;
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Sequential time: %.6f seconds\n", time_spent);
    
    free_image(input);
    free_image(filtered);
    free_image(grayscale);
    
    return 0;
}

