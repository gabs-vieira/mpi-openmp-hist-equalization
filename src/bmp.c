#include "bmp.h"
#include <string.h>

// Lê uma imagem BMP do arquivo
BMPImage* read_bmp(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir arquivo: %s\n", filename);
        return NULL;
    }

    // Lê o header BMP (54 bytes)
    uint8_t header[54];
    if (fread(header, 1, 54, file) != 54) {
        printf("Erro ao ler header BMP\n");
        fclose(file);
        return NULL;
    }

    // Verifica se é BMP (primeiros 2 bytes devem ser "BM")
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Arquivo não é um BMP válido\n");
        fclose(file);
        return NULL;
    }

    // Extrai largura e altura (offset 18 e 22)
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bits_per_pixel = *(short*)&header[28];

    if (bits_per_pixel != 24) {
        printf("Apenas BMP 24 bits são suportados\n");
        fclose(file);
        return NULL;
    }

    // Calcula o tamanho dos dados da imagem
    int row_size = ((width * 3 + 3) / 4) * 4;  // Padding para múltiplo de 4
    int data_size = row_size * height;

    // Aloca memória para a imagem
    BMPImage *img = (BMPImage*)malloc(sizeof(BMPImage));
    img->width = width;
    img->height = height;
    img->data = (uint8_t*)malloc(data_size);

    // Lê os dados da imagem
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

// Escreve uma imagem BMP no arquivo
void write_bmp(const char *filename, BMPImage *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao criar arquivo: %s\n", filename);
        return;
    }

    int row_size = ((img->width * 3 + 3) / 4) * 4;
    int data_size = row_size * img->height;
    int file_size = 54 + data_size;

    // Cria o header BMP
    uint8_t header[54] = {0};
    header[0] = 'B';
    header[1] = 'M';
    *(int*)&header[2] = file_size;
    *(int*)&header[10] = 54;  // Offset dos dados
    *(int*)&header[14] = 40;  // Tamanho do header
    *(int*)&header[18] = img->width;
    *(int*)&header[22] = img->height;
    *(short*)&header[26] = 1;  // Planos
    *(short*)&header[28] = 24; // Bits por pixel
    *(int*)&header[34] = data_size;

    // Escreve o header
    fwrite(header, 1, 54, file);

    // Escreve os dados da imagem
    fwrite(img->data, 1, data_size, file);

    fclose(file);
}

// Libera a memória da imagem
void free_bmp(BMPImage *img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

