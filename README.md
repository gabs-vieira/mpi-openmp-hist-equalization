# Image Processing - Sequential, MPI, and OpenMP

This project implements BMP image processing in three versions: sequential, MPI parallel, and OpenMP parallel.

## What it does

The program processes images in three steps:
1. **Median Filter**: Removes noise using an N×N filter (3×3, 5×5, 7×7, etc.)
2. **Grayscale Conversion**: Converts color images to grayscale
3. **Histogram Equalization**: Improves image contrast

## Build

```bash
make all
```

**Requirements:**
- GCC with C11 support
- OpenMP (usually included with GCC)
- MPI (OpenMPI or MPICH)

## Usage

### Sequential
```bash
./bin/sequential <mask_size> <input_file>
```

### MPI
```bash
mpirun -np <num_processes> ./bin/mpi_version <mask_size> <input_file>
```

### OpenMP
```bash
./bin/openmp_version <mask_size> <num_threads> <input_file>
```

**Examples:**
```bash
./bin/sequential 3 data/img.bmp
mpirun -np 4 ./bin/mpi_version 3 data/img.bmp
./bin/openmp_version 3 4 data/img.bmp
```

**Parameters:**
- `mask_size`: Filter size (must be odd: 3, 5, 7, etc.)
- `num_processes` (MPI): Number of MPI processes
- `num_threads` (OpenMP): Number of OpenMP threads
- `input_file`: Path to input BMP file

## Performance Testing

Run automated performance tests:
```bash
./test_performance.sh data/img.bmp
```

This tests all versions with different mask sizes and calculates speedup and efficiency metrics. Results are saved to `performance_results.txt` and `performance_metrics.csv`.

## Output

Processed images are saved in the `output/` directory:
- `sequential_<mask>_output.bmp`
- `mpi_<mask>_output.bmp`
- `openmp_<mask>_output.bmp`

## Clean

```bash
make clean
```

Removes compiled binaries and output images.

## Notes

- Supports 24-bit BMP images (uncompressed)
- Mask size must be odd (3, 5, 7, 9, etc.)
- Processing time is displayed at the end of execution
