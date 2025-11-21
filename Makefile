CC = gcc
MPICC = mpicc
CFLAGS = -Wall -Wextra -O2 -std=c99
OPENMP_FLAGS = -fopenmp
INCLUDES = -Iinclude
SRC_DIR = src
BIN_DIR = bin

SRC_BMP = $(SRC_DIR)/bmp_utils.c
SRC_SEQ = $(SRC_DIR)/sequential.c
SRC_MPI = $(SRC_DIR)/mpi_version.c
SRC_OMP = $(SRC_DIR)/openmp_version.c

all: $(BIN_DIR)/sequential $(BIN_DIR)/mpi_version $(BIN_DIR)/openmp_version

$(BIN_DIR)/sequential: $(SRC_SEQ) $(SRC_BMP)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SRC_SEQ) $(SRC_BMP)

$(BIN_DIR)/mpi_version: $(SRC_MPI) $(SRC_BMP)
	@mkdir -p $(BIN_DIR)
	$(MPICC) $(CFLAGS) $(INCLUDES) -o $@ $(SRC_MPI) $(SRC_BMP)

$(BIN_DIR)/openmp_version: $(SRC_OMP) $(SRC_BMP)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) $(INCLUDES) -o $@ $(SRC_OMP) $(SRC_BMP)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
