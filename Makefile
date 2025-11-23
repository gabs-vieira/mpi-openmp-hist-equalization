CC = gcc
MPICC = mpicc
CFLAGS = -Wall -Wextra -O2 -std=c11 -Isrc/include
OPENMP_FLAGS = -fopenmp

# Diretórios
SRC_DIR = src
OUTPUT_DIR = output
BIN_DIR = bin

# Arquivos objeto
BMP_OBJ = $(BIN_DIR)/bmp.o
IMG_PROC_OBJ = $(BIN_DIR)/image_processing.o

# Executáveis
SEQUENTIAL = $(BIN_DIR)/sequential
MPI_VERSION = $(BIN_DIR)/mpi_version
OPENMP_VERSION = $(BIN_DIR)/openmp_version

.PHONY: all clean sequential mpi openmp

all: sequential mpi openmp

# Cria diretórios necessários
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Compila biblioteca BMP
$(BMP_OBJ): $(SRC_DIR)/bmp.c $(SRC_DIR)/include/bmp.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bmp.c -o $(BMP_OBJ)

# Compila processamento de imagem
$(IMG_PROC_OBJ): $(SRC_DIR)/image_processing.c $(SRC_DIR)/include/image_processing.h $(BMP_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/image_processing.c -o $(IMG_PROC_OBJ)

# Versão sequencial
sequential: $(SEQUENTIAL)

$(SEQUENTIAL): $(SRC_DIR)/sequential.c $(BMP_OBJ) $(IMG_PROC_OBJ) | $(BIN_DIR) $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/sequential.c $(BMP_OBJ) $(IMG_PROC_OBJ) -o $(SEQUENTIAL)

# Versão MPI
mpi: $(MPI_VERSION)

$(MPI_VERSION): $(SRC_DIR)/mpi_version.c $(BMP_OBJ) $(IMG_PROC_OBJ) | $(BIN_DIR) $(OUTPUT_DIR)
	$(MPICC) $(CFLAGS) $(SRC_DIR)/mpi_version.c $(BMP_OBJ) $(IMG_PROC_OBJ) -o $(MPI_VERSION)

# Versão OpenMP
openmp: $(OPENMP_VERSION)

$(OPENMP_VERSION): $(SRC_DIR)/openmp_version.c $(BMP_OBJ) $(IMG_PROC_OBJ) | $(BIN_DIR) $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) $(SRC_DIR)/openmp_version.c $(BMP_OBJ) $(IMG_PROC_OBJ) -o $(OPENMP_VERSION)

# Limpa arquivos compilados
clean:
	rm -rf $(BIN_DIR)
	rm -f $(OUTPUT_DIR)/*.bmp

# Testa todas as versões
test: all
	@echo "Testando versão sequencial..."
	./$(SEQUENTIAL) 3 data/img.bmp
	@echo "\nTestando versão MPI com 2 processos..."
	mpirun -np 2 ./$(MPI_VERSION) 3 data/img.bmp
	@echo "\nTestando versão OpenMP com 2 threads..."
	./$(OPENMP_VERSION) 3 2 data/img.bmp

