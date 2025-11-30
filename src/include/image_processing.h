#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "bmp.h"
#include <stdint.h>

// helper function for qsort
int compare_uint8(const void *a, const void *b);

// applies N×N median filter to image
void apply_median_filter(BMPImage *img, int mask_size);

// converts image to grayscale
void convert_to_grayscale(BMPImage *img);

// equalizes image histogram
void equalize_histogram(BMPImage *img);

#endif

