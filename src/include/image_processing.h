#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "bmp.h"
#include <stdint.h>

// Função auxiliar para comparar valores (usada no qsort)
int compare_uint8(const void *a, const void *b);

// Aplica filtro mediana N×N na imagem
void apply_median_filter(BMPImage *img, int mask_size);

// Converte imagem para tons de cinza
void convert_to_grayscale(BMPImage *img);

// Equaliza o histograma da imagem
void equalize_histogram(BMPImage *img);

#endif

