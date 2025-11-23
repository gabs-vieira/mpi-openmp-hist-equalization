#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Estrutura para armazenar uma imagem BMP
typedef struct {
    int width;
    int height;
    uint8_t *data;  // Dados da imagem (BGR)
} BMPImage;

// Lê uma imagem BMP do arquivo
BMPImage* read_bmp(const char *filename);

// Escreve uma imagem BMP no arquivo
void write_bmp(const char *filename, BMPImage *img);

// Libera a memória da imagem
void free_bmp(BMPImage *img);

#endif

