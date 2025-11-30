#include "bmp.h"
#include <string.h>

// reads a BMP file
BMPImage* read_bmp(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir arquivo: %s\n", filename);
        return NULL;
    }

    // read BMP header (54 bytes)
    uint8_t header[54];
    if (fread(header, 1, 54, file) != 54) {
        printf("Erro ao ler header BMP\n");
        fclose(file);
        return NULL;
    }

    // check if it's a BMP (first 2 bytes should be "BM")
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Arquivo não é um BMP válido\n");
        fclose(file);
        return NULL;
    }

    // get width and height from header
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bits_per_pixel = *(short*)&header[28];

    if (bits_per_pixel != 24) {
        printf("Apenas BMP 24 bits são suportados\n");
        fclose(file);
        return NULL;
    }

    // calculate row size with padding (must be multiple of 4)
    int row_size = ((width * 3 + 3) / 4) * 4;
    int data_size = row_size * height;

    // allocate memory for image
    BMPImage *img = (BMPImage*)malloc(sizeof(BMPImage));
    img->width = width;
    img->height = height;
    img->data = (uint8_t*)malloc(data_size);

    // read image data
    if (fread(img->data, 1, data_size, file) != (size_t)data_size) {
        printf("Erro ao ler dados da imagem\n");
        free(img->data);
        free(img);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return img;
}

// writes BMP to file
void write_bmp(const char *filename, BMPImage *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao criar arquivo: %s\n", filename);
        return;
    }

    int row_size = ((img->width * 3 + 3) / 4) * 4;
    int data_size = row_size * img->height;
    int file_size = 54 + data_size;

    // create BMP header
    uint8_t header[54] = {0};
    header[0] = 'B';
    header[1] = 'M';
    *(int*)&header[2] = file_size;
    *(int*)&header[10] = 54;  // data offset
    *(int*)&header[14] = 40;  // header size
    *(int*)&header[18] = img->width;
    *(int*)&header[22] = img->height;
    *(short*)&header[26] = 1;  // planes
    *(short*)&header[28] = 24; // bits per pixel
    *(int*)&header[34] = data_size;

    // write header
    fwrite(header, 1, 54, file);

    // write image data
    fwrite(img->data, 1, data_size, file);

    fclose(file);
}

// free image memory
void free_bmp(BMPImage *img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

