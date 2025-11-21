# Parallel Image Histogram Equalization
Implementation of a parallel image-processing pipeline using **MPI** and **OpenMP**.  
The program processes a 24-bit BMP image by applying:

1. **Median Filter (N×N)**  
2. **Grayscale Conversion** using weighted RGB combination  
3. **Histogram Equalization**  

Two parallel versions are implemented:
- **MPI (Message Passing Interface)**
- **OpenMP (Shared-Memory Parallelism)**

Both versions are compared in terms of **speedup** and **efficiency**.

---

## Features

### Sequential Implementation
- Applies an N×N median filter  
- Converts filtered image to grayscale using:   Gray = 0.299R + 0.587G + 0.114B


- Performs histogram equalization  
- Saves processed output as BMP  

### MPI Implementation
Parallelizes:
- Median filtering  
- Grayscale conversion  
- Histogram equalization  

The program receives:
- Mask size  
- Number of processes  

Outputs:
- Execution time  
- Speedup  
- Efficiency  

### OpenMP Implementation
Parallelizes the same three steps using shared memory.

Parameters:
- Mask size  
- Number of threads  

Outputs:
- Execution time  
- Speedup  
- Efficiency  

---

## Project Structure

```
.
├── bin/              # Compiled executables
├── include/          # Headers (.h)
├── src/              # Source code (.c)
├── scripts/          # Test scripts
├── Makefile          # Build system
└── README.md         # Documentation
```

## Compilation

```bash
make
```

This compiles three versions in the `bin/` directory:
- `bin/sequential` - sequential version
- `bin/mpi_version` - MPI version
- `bin/openmp_version` - OpenMP version

## Usage

### Sequential Version
```bash
./bin/sequential <input_image.bmp> <output_image.bmp> <mask_size>
```

### MPI Version
```bash
mpirun -np <num_procs> ./bin/mpi_version <input_image.bmp> <output_image.bmp> <mask_size>
```

### OpenMP Version
```bash
export OMP_NUM_THREADS=<num_threads>
./bin/openmp_version <input_image.bmp> <output_image.bmp> <mask_size>
```

### Test Script
```bash
./scripts/test.sh <input_image.bmp> <mask_size>
```

The script runs all versions with different numbers of processes/threads and calculates speedup and efficiency.

## Example

```bash
# Compile
make

# Run sequential version
./bin/sequential input.bmp output_seq.bmp 5

# Run MPI version with 4 processes
mpirun -np 4 ./bin/mpi_version input.bmp output_mpi.bmp 5

# Run OpenMP version with 4 threads
export OMP_NUM_THREADS=4
./bin/openmp_version input.bmp output_omp.bmp 5

# Run complete test
./scripts/test.sh input.bmp 5
```

## Requirements

- GCC with OpenMP support
- MPI (OpenMPI or MPICH)
- 24-bit BMP image

---
