#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include "bmp.h"
#include "image_processing.h"

// Função auxiliar para comparar valores
int compare_uint8_mpi(const void *a, const void *b) {
    uint8_t va = *(uint8_t*)a;
    uint8_t vb = *(uint8_t*)b;
    return (va < vb) ? -1 : (va > vb) ? 1 : 0;
}

// Função para aplicar filtro mediana em uma região da imagem
void apply_median_filter_region(BMPImage *img, int mask_size, int start_y, int end_y) {
    int width = img->width;
    int height = img->height;
    int row_size = ((width * 3 + 3) / 4) * 4;
    int half = mask_size / 2;

    uint8_t *original = (uint8_t*)malloc(row_size * height);
    for (int i = 0; i < row_size * height; i++) {
        original[i] = img->data[i];
    }

    uint8_t *mask_values = (uint8_t*)malloc(mask_size * mask_size * sizeof(uint8_t));

    for (int y = start_y; y < end_y; y++) {
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

                qsort(mask_values, count, sizeof(uint8_t), compare_uint8_mpi);

                uint8_t median = mask_values[count / 2];
                int idx = y * row_size + x * 3 + channel;
                img->data[idx] = median;
            }
        }
    }

    free(mask_values);
    free(original);
}

// Função para converter para cinza em uma região
void convert_to_grayscale_region(BMPImage *img, int start_y, int end_y) {
    int width = img->width;
    int row_size = ((width * 3 + 3) / 4) * 4;

    for (int y = start_y; y < end_y; y++) {
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
}

// Função para equalizar histograma em uma região (coleta dados)
void collect_histogram_region(BMPImage *img, int *histogram, int start_y, int end_y) {
    int width = img->width;
    int row_size = ((width * 3 + 3) / 4) * 4;

    for (int y = start_y; y < end_y; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];
            histogram[gray]++;
        }
    }
}

// Função para aplicar equalização em uma região
void apply_equalization_region(BMPImage *img, int *cumulative, int total_pixels, int start_y, int end_y) {
    int width = img->width;
    int row_size = ((width * 3 + 3) / 4) * 4;

    for (int y = start_y; y < end_y; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * row_size + x * 3;
            uint8_t gray = img->data[idx];
            uint8_t new_value = (uint8_t)((cumulative[gray] * 255.0) / total_pixels);
            img->data[idx] = new_value;
            img->data[idx + 1] = new_value;
            img->data[idx + 2] = new_value;
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) {
            printf("Uso: mpirun -np <num_processos> %s <tamanho_mascara> <arquivo_entrada>\n", argv[0]);
            printf("Exemplo: mpirun -np 4 %s 3 data/img.bmp\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int mask_size = atoi(argv[1]);
    if (mask_size % 2 == 0 || mask_size < 3) {
        if (rank == 0) {
            printf("Tamanho da máscara deve ser ímpar e >= 3\n");
        }
        MPI_Finalize();
        return 1;
    }

    const char *input_file = argv[2];
    
    // Gera nome do arquivo de saída com tamanho da máscara
    char output_file[256];
    if (rank == 0) {
        snprintf(output_file, sizeof(output_file), "output/mpi_%d_output.bmp", mask_size);
    }

    BMPImage *img = NULL;
    double start_time = 0.0, end_time;

    // Processo 0 lê a imagem
    if (rank == 0) {
        printf("Lendo imagem: %s\n", input_file);
        img = read_bmp(input_file);
        if (!img) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("Imagem carregada: %dx%d\n", img->width, img->height);
        printf("Matriz de %d\n", mask_size);
        printf("Processando com %d processos...\n", size);
        start_time = MPI_Wtime();
    }

    // Envia dimensões da imagem para todos os processos
    int width, height, row_size, data_size;
    if (rank == 0) {
        width = img->width;
        height = img->height;
        row_size = ((width * 3 + 3) / 4) * 4;
        data_size = row_size * height;
    }

    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&row_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Todos os processos alocam memória
    if (rank != 0) {
        img = (BMPImage*)malloc(sizeof(BMPImage));
        img->width = width;
        img->height = height;
        img->data = (uint8_t*)malloc(data_size);
    }

    // Distribui a imagem para todos os processos
    MPI_Bcast(img->data, data_size, MPI_BYTE, 0, MPI_COMM_WORLD);

    // Calcula região de cada processo
    int rows_per_process = height / size;
    int start_y = rank * rows_per_process;
    int end_y = (rank == size - 1) ? height : (rank + 1) * rows_per_process;

    // ETAPA 1: Filtro mediana
    // Cada processo processa sua região (com overlap mínimo para bordas)
    int half = mask_size / 2;
    int local_start = (start_y > half) ? start_y - half : 0;
    int local_end = (end_y < height - half) ? end_y + half : height;

    apply_median_filter_region(img, mask_size, local_start, local_end);

    // Coleta apenas as partes processadas (sem overlap desnecessário)
    if (rank == 0) {
        // Processo 0 já tem sua parte, recebe das outras
        for (int i = 1; i < size; i++) {
            int other_start = i * rows_per_process;
            int other_end = (i == size - 1) ? height : (i + 1) * rows_per_process;
            int offset = other_start * row_size;
            int recv_size = (other_end - other_start) * row_size;
            MPI_Recv(img->data + offset, recv_size, MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        // Outros processos enviam apenas sua parte
        int offset = start_y * row_size;
        int send_size = (end_y - start_y) * row_size;
        MPI_Send(img->data + offset, send_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

    // Broadcast apenas se necessário para próxima etapa (mas não precisamos, vamos processar localmente)
    // Removido broadcast desnecessário

    // ETAPA 2: Conversão para cinza
    // Broadcast da imagem filtrada para todos os processos (todos devem chamar Bcast)
    MPI_Bcast(img->data, data_size, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    convert_to_grayscale_region(img, start_y, end_y);

    // Coleta resultados
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            int other_start = i * rows_per_process;
            int other_end = (i == size - 1) ? height : (i + 1) * rows_per_process;
            int offset = other_start * row_size;
            int recv_size = (other_end - other_start) * row_size;
            MPI_Recv(img->data + offset, recv_size, MPI_BYTE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        int offset = start_y * row_size;
        int send_size = (end_y - start_y) * row_size;
        MPI_Send(img->data + offset, send_size, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }

    // ETAPA 3: Equalização de histograma
    // Broadcast da imagem em cinza para todos os processos (todos devem chamar Bcast)
    MPI_Bcast(img->data, data_size, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    int local_histogram[256] = {0};
    collect_histogram_region(img, local_histogram, start_y, end_y);

    // Soma histogramas de todos os processos
    int global_histogram[256];
    MPI_Allreduce(local_histogram, global_histogram, 256, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Calcula histograma cumulativo (todos os processos calculam o mesmo)
    int cumulative[256];
    cumulative[0] = global_histogram[0];
    for (int i = 1; i < 256; i++) {
        cumulative[i] = cumulative[i - 1] + global_histogram[i];
    }

    int total_pixels = width * height;
    apply_equalization_region(img, cumulative, total_pixels, start_y, end_y);

    // Coleta resultados finais
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            int other_start = i * rows_per_process;
            int other_end = (i == size - 1) ? height : (i + 1) * rows_per_process;
            int offset = other_start * row_size;
            int recv_size = (other_end - other_start) * row_size;
            MPI_Recv(img->data + offset, recv_size, MPI_BYTE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        end_time = MPI_Wtime();
        double time_spent = end_time - start_time;

        printf("Salvando imagem: %s\n", output_file);
        write_bmp(output_file, img);
        printf("Tempo total: %.4f segundos\n", time_spent);
    } else {
        int offset = start_y * row_size;
        int send_size = (end_y - start_y) * row_size;
        MPI_Send(img->data + offset, send_size, MPI_BYTE, 0, 2, MPI_COMM_WORLD);
    }

    free_bmp(img);
    MPI_Finalize();

    return 0;
}

