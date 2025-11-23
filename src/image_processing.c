#include "image_processing.h"
#include <string.h>
#include <stdlib.h>

// Função auxiliar para comparar valores (usada no qsort)
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

    // Cria uma cópia da imagem original
    uint8_t *original = (uint8_t*)malloc(row_size * height);
    memcpy(original, img->data, row_size * height);

    // Array para armazenar valores da máscara
    uint8_t *mask_values = (uint8_t*)malloc(mask_size * mask_size * sizeof(uint8_t));

    // processa cada pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // para cada canal (B, G, R)
            for (int channel = 0; channel < 3; channel++) {
                int count = 0;

                // coleta valores da máscara
                for (int dy = -half; dy <= half; dy++) {
                    for (int dx = -half; dx <= half; dx++) {
                        int ny = y + dy;
                        int nx = x + dx;

                        // verifica limites
                        if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                            int idx = ny * row_size + nx * 3 + channel;
                            mask_values[count++] = original[idx];
                        }
                    }
                }

                // ordena e pega a mediana
                qsort(mask_values, count, sizeof(uint8_t), compare_uint8);
                uint8_t median = mask_values[count / 2];

                // aplica o valor mediano
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

    // calcula o histograma usando um so canal
    int histogram[256] = {0};
    int total_pixels = width * height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];
            histogram[gray]++;
        }
    }

    // calcula histograma cumulativo
    int cumulative[256];
    cumulative[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cumulative[i] = cumulative[i - 1] + histogram[i];
    }

    // aplica equalização
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];

            // calcula novo valor equalizado
            uint8_t new_value = (uint8_t)((cumulative[gray] * 255.0) / total_pixels);

            // aplica nos três canais
            img->data[idx] = new_value;
            img->data[idx + 1] = new_value;
            img->data[idx + 2] = new_value;
        }
    }
}

