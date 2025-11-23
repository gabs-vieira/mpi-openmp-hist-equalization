#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "bmp.h"
#include "image_processing.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <tamanho_mascara> <num_threads> <arquivo_entrada>\n", argv[0]);
        printf("Exemplo: %s 3 4 data/img.bmp\n", argv[0]);
        return 1;
    }

    int mask_size = atoi(argv[1]);
    if (mask_size % 2 == 0 || mask_size < 3) {
        printf("Tamanho da máscara deve ser ímpar e >= 3\n");
        return 1;
    }

    int num_threads = atoi(argv[2]);
    if (num_threads < 1) {
        printf("Número de threads deve ser >= 1\n");
        return 1;
    }

    omp_set_num_threads(num_threads);

    const char *input_file = argv[3];
    
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "output/openmp_%d_output.bmp", mask_size);

    printf("Lendo imagem: %s\n", input_file);
    BMPImage *img = read_bmp(input_file);
    if (!img) {
        return 1;
    }

    printf("Imagem carregada: %dx%d\n", img->width, img->height);
    printf("Matriz de %d\n", mask_size);
    printf("Processando com %d threads...\n", num_threads);

    double start_time = omp_get_wtime();

    // ETAPA 1: filtro mediana
    printf("Aplicando filtro mediana %dx%d...\n", mask_size, mask_size);
    int width = img->width;
    int height = img->height;
    int row_size = ((width * 3 + 3) / 4) * 4;
    int half = mask_size / 2;

    uint8_t *original = (uint8_t*)malloc(row_size * height);
    #pragma omp parallel for
    for (int i = 0; i < row_size * height; i++) {
        original[i] = img->data[i];
    }

    #pragma omp parallel
    {
        uint8_t *mask_values = (uint8_t*)malloc(mask_size * mask_size * sizeof(uint8_t));

        #pragma omp for
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int channel = 0; channel < 3; channel++) {
                    int count = 0;

                    for (int dy = -half; dy <= half; dy++) {
                        for (int dx = -half; dx <= half; dx++) {
                            int ny = y + dy;
                            int nx = x + dx;

                            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                                int idx = ny * row_size + nx * 3 + channel;
                                mask_values[count++] = original[idx];
                            }
                        }
                    }

                    qsort(mask_values, count, sizeof(uint8_t), compare_uint8);
                    uint8_t median = mask_values[count / 2];
                    int idx = y * row_size + x * 3 + channel;
                    img->data[idx] = median;
                }
            }
        }

        free(mask_values);
    }

    free(original);

    // ETAPA 2: conversão para tons de cinza
    printf("Convertendo para tons de cinza...\n");
    #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t B = img->data[idx];
            uint8_t G = img->data[idx + 1];
            uint8_t R = img->data[idx + 2];
            uint8_t gray = (uint8_t)(0.299 * R + 0.587 * G + 0.114 * B);
            img->data[idx] = gray;
            img->data[idx + 1] = gray;
            img->data[idx + 2] = gray;
        }
    }

    // ETAPA 3: equalização de histograma
    printf("Equalizando histograma...\n");
    int histogram[256] = {0};

    // calcula histograma
    #pragma omp parallel
    {
        int local_histogram[256] = {0};

        #pragma omp for
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * row_size + x * 3;
                uint8_t gray = img->data[idx];
                local_histogram[gray]++;
            }
        }

        // soma histogramas locais
        #pragma omp critical
        {
            for (int i = 0; i < 256; i++) {
                histogram[i] += local_histogram[i];
            }
        }
    }

    // calcula histograma cumulativo
    int cumulative[256];
    cumulative[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cumulative[i] = cumulative[i - 1] + histogram[i];
    }

    int total_pixels = width * height;

    // aplica equalização
    #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];
            uint8_t new_value = (uint8_t)((cumulative[gray] * 255.0) / total_pixels);
            img->data[idx] = new_value;
            img->data[idx + 1] = new_value;
            img->data[idx + 2] = new_value;
        }
    }

    double end_time = omp_get_wtime();
    double time_spent = end_time - start_time;

    printf("Salvando imagem: %s\n", output_file);
    write_bmp(output_file, img);

    printf("Tempo total: %.4f segundos\n", time_spent);
    free_bmp(img);

    return 0;
}

