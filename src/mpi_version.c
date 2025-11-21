#include "../include/bmp_utils.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int compare_uint8(const void *a, const void *b) {
    return (*(uint8_t*)a - *(uint8_t*)b);
}

void median_filter_parallel(Image *input, Image *output, int mask_size, int rank, int size) {
    int half = mask_size / 2;
    int rows_per_proc = input->height / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? input->height : (rank + 1) * rows_per_proc;
    int local_rows = end_row - start_row;
    
    uint8_t *window = (uint8_t*)malloc(mask_size * mask_size * sizeof(uint8_t));
    Pixel *local_result = (Pixel*)malloc(local_rows * input->width * sizeof(Pixel));
    
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < input->width; x++) {
            int count = 0;
            
            for (int dy = -half; dy <= half; dy++) {
                for (int dx = -half; dx <= half; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    
                    if (ny >= 0 && ny < input->height && nx >= 0 && nx < input->width) {
                        int idx = ny * input->width + nx;
                        window[count++] = input->data[idx].r;
                    }
                }
            }
            
            qsort(window, count, sizeof(uint8_t), compare_uint8);
            uint8_t median = window[count / 2];
            
            int local_idx = (y - start_row) * input->width + x;
            local_result[local_idx].r = median;
            local_result[local_idx].g = median;
            local_result[local_idx].b = median;
        }
    }
    
    free(window);
    
    int *recvcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        int rpp = input->height / size;
        int sr = i * rpp;
        int er = (i == size - 1) ? input->height : (i + 1) * rpp;
        recvcounts[i] = (er - sr) * input->width * 3;
        displs[i] = sr * input->width * 3;
    }
    
    MPI_Gatherv(local_result, local_rows * input->width * 3, MPI_BYTE,
                output->data, recvcounts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    free(recvcounts);
    free(displs);
    
    MPI_Bcast(output->data, input->width * input->height * 3, MPI_BYTE, 0, MPI_COMM_WORLD);
    free(local_result);
}

void to_grayscale_parallel(Image *input, Image *output, int rank, int size) {
    int rows_per_proc = input->height / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? input->height : (rank + 1) * rows_per_proc;
    int local_rows = end_row - start_row;
    
    Pixel *local_result = (Pixel*)malloc(local_rows * input->width * sizeof(Pixel));
    
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < input->width; x++) {
            int idx = y * input->width + x;
            uint8_t gray = (uint8_t)(0.299 * input->data[idx].r + 
                                      0.587 * input->data[idx].g + 
                                      0.114 * input->data[idx].b);
            int local_idx = (y - start_row) * input->width + x;
            local_result[local_idx].r = gray;
            local_result[local_idx].g = gray;
            local_result[local_idx].b = gray;
        }
    }
    
    int *recvcounts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        recvcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            int rpp = input->height / size;
            int sr = i * rpp;
            int er = (i == size - 1) ? input->height : (i + 1) * rpp;
            recvcounts[i] = (er - sr) * input->width * 3;
            displs[i] = sr * input->width * 3;
        }
    }
    
    MPI_Gatherv(local_result, local_rows * input->width * 3, MPI_BYTE,
                output->data, recvcounts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    MPI_Bcast(output->data, input->width * input->height * 3, MPI_BYTE, 0, MPI_COMM_WORLD);
    free(local_result);
}

void histogram_equalization_parallel(Image *img, int rank, int size) {
    int rows_per_proc = img->height / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? img->height : (rank + 1) * rows_per_proc;
    int local_rows = end_row - start_row;
    
    int *local_hist = (int*)calloc(256, sizeof(int));
    
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < img->width; x++) {
            int idx = y * img->width + x;
            local_hist[img->data[idx].r]++;
        }
    }
    
    int *global_hist = (int*)calloc(256, sizeof(int));
    MPI_Allreduce(local_hist, global_hist, 256, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    int *cdf = (int*)malloc(256 * sizeof(int));
    cdf[0] = global_hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + global_hist[i];
    }
    
    int min_cdf = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            min_cdf = cdf[i];
            break;
        }
    }
    
    int total_pixels = img->width * img->height;
    
    Pixel *local_result = (Pixel*)malloc(local_rows * img->width * sizeof(Pixel));
    
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < img->width; x++) {
            int idx = y * img->width + x;
            uint8_t old_val = img->data[idx].r;
            uint8_t new_val = (uint8_t)((cdf[old_val] - min_cdf) * 255.0 / (total_pixels - min_cdf));
            int local_idx = (y - start_row) * img->width + x;
            local_result[local_idx].r = new_val;
            local_result[local_idx].g = new_val;
            local_result[local_idx].b = new_val;
        }
    }
    
    int *recvcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        int rpp = img->height / size;
        int sr = i * rpp;
        int er = (i == size - 1) ? img->height : (i + 1) * rpp;
        recvcounts[i] = (er - sr) * img->width * 3;
        displs[i] = sr * img->width * 3;
    }
    
    MPI_Gatherv(local_result, local_rows * img->width * 3, MPI_BYTE,
                img->data, recvcounts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    free(recvcounts);
    free(displs);
    
    MPI_Bcast(img->data, img->width * img->height * 3, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    free(local_hist);
    free(global_hist);
    free(cdf);
    free(local_result);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 4) {
        if (rank == 0) {
            fprintf(stderr, "Usage: mpirun -np <num_procs> %s <input_image.bmp> <output_image.bmp> <mask_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    int mask_size = atoi(argv[3]);
    if (mask_size % 2 == 0 || mask_size < 3) {
        if (rank == 0) {
            fprintf(stderr, "Mask size must be odd and >= 3\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    double start_time = MPI_Wtime();
    
    Image *input = NULL;
    if (rank == 0) {
        input = read_bmp(argv[1]);
        if (!input) {
            MPI_Finalize();
            return 1;
        }
    }
    
    int width, height;
    if (rank == 0) {
        width = input->width;
        height = input->height;
    }
    
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank != 0) {
        input = (Image*)malloc(sizeof(Image));
        input->width = width;
        input->height = height;
        input->data = (Pixel*)malloc(input->width * input->height * sizeof(Pixel));
    }
    
    MPI_Bcast(input->data, width * height * 3, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    Image *filtered = (Image*)malloc(sizeof(Image));
    filtered->width = width;
    filtered->height = height;
    filtered->data = (Pixel*)malloc(filtered->width * filtered->height * sizeof(Pixel));
    
    Image *grayscale = (Image*)malloc(sizeof(Image));
    grayscale->width = width;
    grayscale->height = height;
    grayscale->data = (Pixel*)malloc(grayscale->width * grayscale->height * sizeof(Pixel));
    
    median_filter_parallel(input, filtered, mask_size, rank, size);
    to_grayscale_parallel(filtered, grayscale, rank, size);
    histogram_equalization_parallel(grayscale, rank, size);
    
    if (rank == 0) {
        if (!write_bmp(argv[2], grayscale)) {
            free_image(input);
            free_image(filtered);
            free_image(grayscale);
            MPI_Finalize();
            return 1;
        }
    }
    
    double end_time = MPI_Wtime();
    double time_spent = end_time - start_time;
    
    if (rank == 0) {
        printf("MPI time (%d processes): %.6f seconds\n", size, time_spent);
    }
    
    free_image(input);
    free_image(filtered);
    free_image(grayscale);
    
    MPI_Finalize();
    return 0;
}

