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
