#include "bmp_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_res;
    int32_t y_res;
    uint32_t colors;
    uint32_t important_colors;
} BMPInfoHeader;
#pragma pack(pop)

Image* read_bmp(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    BMPHeader header;
    BMPInfoHeader info;

    if (fread(&header, sizeof(BMPHeader), 1, file) != 1) {
        fclose(file);
        return NULL;
    }

    if (fread(&info, sizeof(BMPInfoHeader), 1, file) != 1) {
        fclose(file);
        return NULL;
    }

    if (header.type != 0x4D42 || info.bits != 24) {
        fprintf(stderr, "Invalid BMP format (must be 24 bits)\n");
        fclose(file);
        return NULL;
    }

    Image *img = (Image*)malloc(sizeof(Image));
    img->width = info.width;
    img->height = abs(info.height);
    img->data = (Pixel*)malloc(img->width * img->height * sizeof(Pixel));

    int row_size = ((img->width * 3 + 3) / 4) * 4;
    uint8_t *row = (uint8_t*)malloc(row_size);

    fseek(file, header.offset, SEEK_SET);

    for (int y = img->height - 1; y >= 0; y--) {
        if (fread(row, 1, row_size, file) != row_size) {
            free(row);
            free_image(img);
            fclose(file);
            return NULL;
        }
        for (int x = 0; x < img->width; x++) {
            int idx = y * img->width + x;
            img->data[idx].b = row[x * 3];
            img->data[idx].g = row[x * 3 + 1];
            img->data[idx].r = row[x * 3 + 2];
        }
    }

    free(row);
    fclose(file);
    return img;
}

int write_bmp(const char *filename, Image *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error creating file: %s\n", filename);
        return 0;
    }

    int row_size = ((img->width * 3 + 3) / 4) * 4;
    int image_size = row_size * img->height;

    BMPHeader header;
    header.type = 0x4D42;
    header.size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + image_size;
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader info;
    info.size = sizeof(BMPInfoHeader);
    info.width = img->width;
    info.height = img->height;
    info.planes = 1;
    info.bits = 24;
    info.compression = 0;
    info.image_size = image_size;
    info.x_res = 0;
    info.y_res = 0;
    info.colors = 0;
    info.important_colors = 0;

    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&info, sizeof(BMPInfoHeader), 1, file);

    uint8_t *row = (uint8_t*)calloc(row_size, 1);

    for (int y = img->height - 1; y >= 0; y--) {
        memset(row, 0, row_size);
        for (int x = 0; x < img->width; x++) {
            int idx = y * img->width + x;
            row[x * 3] = img->data[idx].b;
            row[x * 3 + 1] = img->data[idx].g;
            row[x * 3 + 2] = img->data[idx].r;
        }
        fwrite(row, 1, row_size, file);
    }

    free(row);
    fclose(file);
    return 1;
}

void free_image(Image *img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

