# Processamento de Imagens BMP - Sequencial, MPI e OpenMP

Este projeto implementa processamento de imagens BMP 24 bits em três versões: sequencial, paralela com MPI e paralela com OpenMP. O objetivo é comparar o desempenho das diferentes abordagens de paralelização para processamento de imagens.

## Funcionalidades

O programa realiza três etapas de processamento na imagem:

1. **Filtro Mediana N×N**: Remove ruído da imagem aplicando um filtro mediana com máscara de tamanho configurável (3×3, 5×5, 7×7, etc.)
2. **Conversão para Tons de Cinza**: Converte a imagem colorida para escala de cinza usando a fórmula: `0.299R + 0.587G + 0.114B`
3. **Equalização de Histograma**: Melhora o contraste da imagem equalizando seu histograma

## Estrutura do Projeto

```
.
├── src/
│   ├── bmp.c                  # Leitura e escrita de arquivos BMP
│   ├── image_processing.c      # Funções de processamento de imagem
│   ├── sequential.c           # Versão sequencial
│   ├── mpi_version.c          # Versão paralela com MPI
│   ├── openmp_version.c       # Versão paralela com OpenMP
│   └── include/
│       ├── bmp.h              # Cabeçalho para manipulação BMP
│       └── image_processing.h # Cabeçalho para processamento
├── data/
│   └── img.bmp                # Imagem de entrada de exemplo
├── output/                     # Imagens processadas (geradas automaticamente)
├── bin/                        # Executáveis compilados
├── test_performance.sh         # Script para testes automatizados de desempenho
├── generate_graphs.py          # Script para gerar gráficos de performance
├── performance_results.txt     # Resultados detalhados dos testes
├── performance_metrics.csv     # Métricas em formato CSV
├── performance_graphs.html     # Gráficos interativos de speedup e eficiência
├── Makefile
└── README.md
```

## Compilação

### Pré-requisitos

- GCC com suporte a C11
- OpenMP (geralmente incluído no GCC)
- MPI (OpenMPI ou MPICH)

### Compilar todas as versões

```bash
make all
```

### Compilar versões individuais

```bash
make sequential   # Versão sequencial
make mpi          # Versão MPI
make openmp       # Versão OpenMP
```

## Uso

### Versão Sequencial

```bash
./bin/sequential <tamanho_mascara> <arquivo_entrada>
```

**Exemplo:**
```bash
./bin/sequential 3 data/img.bmp
```

### Versão MPI

```bash
mpirun -np <num_processos> ./bin/mpi_version <tamanho_mascara> <arquivo_entrada>
```

**Exemplo:**
```bash
mpirun -np 4 ./bin/mpi_version 3 data/img.bmp
```

### Versão OpenMP

```bash
./bin/openmp_version <tamanho_mascara> <num_threads> <arquivo_entrada>
```

**Exemplo:**
```bash
./bin/openmp_version 3 4 data/img.bmp
```

## Parâmetros

- **tamanho_mascara**: Tamanho da máscara do filtro mediana (deve ser ímpar: 3, 5, 7, etc.)
- **num_processos** (MPI): Número de processos MPI a serem utilizados
- **num_threads** (OpenMP): Número de threads OpenMP a serem utilizadas
- **arquivo_entrada**: Caminho para o arquivo BMP de entrada

## Testes de Desempenho

### Teste Automatizado (Recomendado)

O projeto inclui um script que executa automaticamente todos os testes e calcula as métricas de desempenho:

```bash
./test_performance.sh data/img.bmp
```

Este script:
- Testa a versão sequencial com máscaras 3×3, 5×5 e 7×7
- Testa a versão MPI com 1, 2, 3 e 4 processos para cada máscara
- Testa a versão OpenMP com 1, 2, 3 e 4 threads para cada máscara
- Calcula automaticamente speedup e eficiência
- Gera dois arquivos de saída:
  - `performance_results.txt`: Resultados detalhados em formato texto
  - `performance_metrics.csv`: Métricas em formato CSV para análise

**Exemplo de saída:**
```
=== RESUMO ===
=== VERSÃO SEQUENCIAL ===
    Máscara 3x3: 0.0472 segundos
    Máscara 5x5: 0.1852 segundos
    Máscara 7x7: 0.4292 segundos

=== VERSÃO MPI ===
Máscara 3x3:
      Processos 1: 0.1258 segundos (Speedup: .3751, Eficiência: .3751)
      Processos 2: 0.0564 segundos (Speedup: .8368, Eficiência: .4184)
      ...
```

### Teste Manual

Para testar manualmente versões específicas:

#### Teste Sequencial (baseline)

```bash
./bin/sequential 3 data/img.bmp
./bin/sequential 5 data/img.bmp
./bin/sequential 7 data/img.bmp
```

#### Teste MPI

```bash
mpirun -np 1 ./bin/mpi_version 3 data/img.bmp
mpirun -np 2 ./bin/mpi_version 3 data/img.bmp
mpirun -np 3 ./bin/mpi_version 3 data/img.bmp
mpirun -np 4 ./bin/mpi_version 3 data/img.bmp
```

Repita para máscaras 5×5 e 7×7.

#### Teste OpenMP

```bash
./bin/openmp_version 3 1 data/img.bmp
./bin/openmp_version 3 2 data/img.bmp
./bin/openmp_version 3 3 data/img.bmp
./bin/openmp_version 3 4 data/img.bmp
```

Repita para máscaras 5×5 e 7×7.

## Visualização de Resultados

### Gerar Gráficos Interativos

Após executar os testes de desempenho, você pode gerar gráficos interativos de speedup e eficiência:

```bash
python3 generate_graphs.py
```

Isso gera o arquivo `performance_graphs.html` com gráficos interativos usando Chart.js. Abra o arquivo no navegador para visualizar:
- Gráficos de Speedup para cada máscara (3×3, 5×5, 7×7)
- Gráficos de Eficiência para cada máscara
- Comparação entre MPI e OpenMP
- Linha de referência para speedup linear ideal

### Visualizar Dados CSV

Para visualizar os dados em formato tabular:

```bash
column -t -s',' performance_metrics.csv
```

Ou abra diretamente o arquivo `performance_metrics.csv` em uma planilha (Excel, LibreOffice Calc, etc.).

## Métricas de Desempenho

### Speedup

O speedup mede quanto mais rápido a versão paralela é em relação à sequencial:

```
Speedup = Tempo_Sequencial / Tempo_Paralelo
```

- **Speedup = 1.0**: Sem ganho de desempenho
- **Speedup > 1.0**: Ganho de desempenho (quanto maior, melhor)
- **Speedup < 1.0**: Overhead de paralelização maior que o ganho

### Eficiência

A eficiência mede o quão bem os recursos paralelos estão sendo utilizados:

```
Eficiência = Speedup / Número_de_Processos_ou_Threads
```

- **Eficiência = 1.0**: Uso perfeito dos recursos (100% de eficiência)
- **Eficiência < 1.0**: Indica overhead de comunicação/sincronização
- **Eficiência > 1.0**: Superlinear speedup (raro, geralmente devido a cache effects)

### Interpretação dos Resultados

- **Speedup ideal**: Linear com o número de processos/threads (ex: 4 processos = speedup 4.0)
- **Eficiência ideal**: Próxima de 1.0, indicando baixo overhead
- **MPI vs OpenMP**: 
  - OpenMP geralmente tem menor overhead para problemas com memória compartilhada
  - MPI é mais adequado para sistemas distribuídos
  - Para sistemas com memória compartilhada, OpenMP costuma ter melhor eficiência

## Arquivos de Saída

### Imagens Processadas

As imagens processadas são salvas automaticamente no diretório `output/` com o seguinte padrão de nomenclatura:
- `sequential_<mascara>_output.bmp`: Versão sequencial
- `mpi_<mascara>_output.bmp`: Versão MPI
- `openmp_<mascara>_output.bmp`: Versão OpenMP

### Arquivos de Performance

- **`performance_results.txt`**: Resultados detalhados em formato texto legível
- **`performance_metrics.csv`**: Métricas em formato CSV para análise em planilhas
- **`performance_graphs.html`**: Gráficos interativos (gerado pelo script `generate_graphs.py`)

## Limpeza

Para remover arquivos compilados e saídas:

```bash
make clean
```

Isso remove:
- Todos os executáveis em `bin/`
- Todas as imagens processadas em `output/`

Para limpar também os arquivos de performance:

```bash
rm -f performance_results.txt performance_metrics.csv performance_graphs.html
```

## Detalhes de Implementação

### Versão Sequencial

Processa a imagem sequencialmente, aplicando os três filtros em ordem:
1. Filtro mediana
2. Conversão para tons de cinza
3. Equalização de histograma

### Versão MPI

- Divide a imagem em fatias horizontais entre os processos
- Cada processo processa sua fatia localmente
- Usa comunicação coletiva (MPI_Allreduce) para sincronizar o histograma na equalização
- Overhead de comunicação pode impactar o desempenho com poucos processos

### Versão OpenMP

- Paraleliza os loops de processamento usando diretivas `#pragma omp parallel`
- Usa memória compartilhada, evitando comunicação explícita
- Geralmente mais eficiente que MPI para sistemas com memória compartilhada
- Menor overhead de sincronização

## Requisitos do Sistema

- **GCC**: Compilador C com suporte a C11
- **OpenMP**: Geralmente incluído no GCC (versão 4.2+)
- **MPI**: OpenMPI ou MPICH instalado
- **Python 3**: Para gerar gráficos (opcional, apenas para visualização)
- **bc**: Calculadora de linha de comando (para cálculos no script de testes)

### Instalação de Dependências (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential libopenmpi-dev openmpi-bin bc python3
```

### Instalação de Dependências (Fedora/RHEL)

```bash
sudo dnf install gcc openmpi-devel bc python3
```

## Notas Importantes

- O programa suporta apenas imagens BMP 24 bits (sem compressão)
- O tamanho da máscara deve ser ímpar (3, 5, 7, 9, etc.)
- As imagens processadas são salvas automaticamente no diretório `output/`
- O tempo de processamento é exibido ao final da execução de cada programa
- Para melhores resultados de performance, execute os testes em um sistema com carga mínima
- Os resultados podem variar dependendo do hardware e da carga do sistema

## Exemplo de Uso Completo

```bash
# 1. Compilar todas as versões
make all

# 2. Executar testes de desempenho
./test_performance.sh data/img.bmp

# 3. Gerar gráficos de visualização
python3 generate_graphs.py

# 4. Visualizar resultados
cat performance_results.txt
# Ou abra performance_graphs.html no navegador
```

## Licença

Este projeto é um trabalho acadêmico para estudo de programação paralela com MPI e OpenMP.

