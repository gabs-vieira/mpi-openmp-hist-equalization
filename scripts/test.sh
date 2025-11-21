#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_image.bmp> <mask_size>"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$PROJECT_DIR/bin"

INPUT=$1
MASK=$2
OUTPUT_SEQ="output_sequential.bmp"
OUTPUT_MPI="output_mpi.bmp"
OUTPUT_OMP="output_openmp.bmp"

cd "$PROJECT_DIR"

echo "=== Performance Test ==="
echo "Image: $INPUT"
echo "Mask: ${MASK}x${MASK}"
echo ""

# Sequential version
echo "--- Sequential Version ---"
if [ ! -f "$BIN_DIR/sequential" ]; then
    echo "Compiling sequential version..."
    make "$BIN_DIR/sequential"
fi
TIME_SEQ=$("$BIN_DIR/sequential" "$INPUT" "$OUTPUT_SEQ" "$MASK" 2>&1 | grep "Sequential time" | awk '{print $3}')
echo "Sequential time: ${TIME_SEQ}s"
echo ""

# MPI version
echo "--- MPI Version ---"
if [ ! -f "$BIN_DIR/mpi_version" ]; then
    echo "Compiling MPI version..."
    make "$BIN_DIR/mpi_version"
fi

echo "Processes | Time (s) | Speedup | Efficiency"
echo "----------|----------|--------|------------"
for procs in 1 2 3 4; do
    TIME_MPI=$(mpirun -np $procs "$BIN_DIR/mpi_version" "$INPUT" "$OUTPUT_MPI" "$MASK" 2>&1 | grep "MPI time" | awk '{print $4}')
    if [ -n "$TIME_MPI" ]; then
        SPEEDUP=$(echo "scale=2; $TIME_SEQ / $TIME_MPI" | bc)
        EFFICIENCY=$(echo "scale=2; $SPEEDUP / $procs" | bc)
        printf "    %d     |  %7s  |  %5s  |   %5s\n" "$procs" "$TIME_MPI" "$SPEEDUP" "$EFFICIENCY"
    fi
done
echo ""

# OpenMP version
echo "--- OpenMP Version ---"
if [ ! -f "$BIN_DIR/openmp_version" ]; then
    echo "Compiling OpenMP version..."
    make "$BIN_DIR/openmp_version"
fi

echo "Threads | Time (s) | Speedup | Efficiency"
echo "--------|----------|--------|------------"
for threads in 1 2 3 4; do
    export OMP_NUM_THREADS=$threads
    TIME_OMP=$("$BIN_DIR/openmp_version" "$INPUT" "$OUTPUT_OMP" "$MASK" 2>&1 | grep "OpenMP time" | awk '{print $4}')
    if [ -n "$TIME_OMP" ]; then
        SPEEDUP=$(echo "scale=2; $TIME_SEQ / $TIME_OMP" | bc)
        EFFICIENCY=$(echo "scale=2; $SPEEDUP / $threads" | bc)
        printf "   %d    |  %7s  |  %5s  |   %5s\n" "$threads" "$TIME_OMP" "$SPEEDUP" "$EFFICIENCY"
    fi
done

echo ""
echo "=== Test completed ==="
