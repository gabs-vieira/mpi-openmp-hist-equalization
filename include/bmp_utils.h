#ifndef BMP_UTILS_H
#define BMP_UTILS_H

#include <stdint.h>

typedef struct {
    uint8_t r, g, b;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel *data;
} Image;

Image* read_bmp(const char *filename);
int write_bmp(const char *filename, Image *img);
void free_image(Image *img);

#endif

