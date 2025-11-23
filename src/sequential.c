#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bmp.h"
#include "image_processing.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <tamanho_mascara> <arquivo_entrada>\n", argv[0]);
        printf("Exemplo: %s 3 data/img.bmp\n", argv[0]);
        return 1;
    }

    int mask_size = atoi(argv[1]);
    if (mask_size % 2 == 0 || mask_size < 3) {
        printf("Tamanho da máscara deve ser ímpar e >= 3\n");
        return 1;
    }

    const char *input_file = argv[2];
    
    // Gera nome do arquivo de saída com o tamanho da máscara
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "output/sequential_%d_output.bmp", mask_size);

    // Lê a imagem
    printf("Lendo imagem: %s\n", input_file);
    BMPImage *img = read_bmp(input_file);
    if (!img) {
        return 1;
    }

    printf("Imagem carregada: %dx%d\n", img->width, img->height);
    printf("Matriz de %d\n", mask_size);

    // Inicia medição de tempo
    clock_t start = clock();

    // Aplica filtro mediana
    printf("Aplicando filtro mediana %dx%d...\n", mask_size, mask_size);
    apply_median_filter(img, mask_size);

    // Converte para tons de cinza
    printf("Convertendo para tons de cinza...\n");
    convert_to_grayscale(img);

    // Equaliza histograma
    printf("Equalizando histograma...\n");
    equalize_histogram(img);

    // Finaliza medição de tempo
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Salva a imagem
    printf("Salvando imagem: %s\n", output_file);
    write_bmp(output_file, img);

    printf("Tempo total: %.4f segundos\n", time_spent);

    // Libera memória
    free_bmp(img);

    return 0;
}

