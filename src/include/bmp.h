#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// structure to store a BMP image
typedef struct {
    int width;
    int height;
    uint8_t *data;  // image data (BGR format)
} BMPImage;

// reads a BMP file
BMPImage* read_bmp(const char *filename);

// writes BMP to file
void write_bmp(const char *filename, BMPImage *img);

// frees image memory
void free_bmp(BMPImage *img);

#endif

