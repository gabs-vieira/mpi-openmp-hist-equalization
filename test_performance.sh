#!/bin/bash

# Script para testar desempenho de todas as versões
# Uso: ./test_performance.sh <arquivo_entrada>

if [ $# -ne 1 ]; then
    echo "Uso: $0 <arquivo_entrada>"
    echo "Exemplo: $0 data/img.bmp"
    exit 1
fi

INPUT_FILE=$1
OUTPUT_DIR="output"
RESULTS_FILE="performance_results.txt"
CSV_FILE="performance_metrics.csv"

# Limpa arquivos anteriores
> $RESULTS_FILE
> $CSV_FILE

echo "=== Testes de Desempenho ===" >> $RESULTS_FILE
echo "Arquivo de entrada: $INPUT_FILE" >> $RESULTS_FILE
echo "Data: $(date)" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

# Cabeçalho CSV
echo "Modelo,Máscara,Processos/Threads,Tempo (s),Speedup,Eficiência" >> $CSV_FILE

# Array para armazenar tempos sequenciais (baseline)
declare -A sequential_times

echo "Testando versão sequencial..."
echo "=== VERSÃO SEQUENCIAL ===" >> $RESULTS_FILE

for mask in 3 5 7; do
    echo "  Máscara ${mask}x${mask}..."
    # Captura o tempo do output do programa (mais preciso)
    TIME=$(./bin/sequential $mask $INPUT_FILE 2>&1 | grep "Tempo total" | awk '{print $3}')
    if [ -z "$TIME" ] || [ "$TIME" = "" ]; then
        echo "Erro ao capturar tempo para máscara ${mask}x${mask}"
        TIME="0.0000"
    fi
    sequential_times[$mask]=$TIME
    echo "    Máscara ${mask}x${mask}: $TIME segundos" >> $RESULTS_FILE
    echo "Sequencial,${mask}x${mask},-,${TIME},1.00,1.00" >> $CSV_FILE
done

echo "" >> $RESULTS_FILE

# Testa versão MPI
echo "Testando versão MPI..."
echo "=== VERSÃO MPI ===" >> $RESULTS_FILE

for mask in 3 5 7; do
    echo "  Máscara ${mask}x${mask}..."
    echo "Máscara ${mask}x${mask}:" >> $RESULTS_FILE
    baseline_time=${sequential_times[$mask]}
    
    for procs in 1 2 3 4; do
        echo "    Processos $procs..."
        # Captura o tempo do output do programa MPI (mais preciso, não inclui overhead de inicialização)
        TIME=$(mpirun -np $procs ./bin/mpi_version $mask $INPUT_FILE 2>&1 | grep "Tempo total" | awk '{print $3}')
        
        if [ -z "$TIME" ] || [ "$TIME" = "" ]; then
            echo "      Erro ao capturar tempo para $procs processos"
            TIME="0.0000"
            speedup="0.0000"
            efficiency="0.0000"
        else
            # Calcula speedup (verifica se baseline_time é válido)
            if [ -n "$baseline_time" ] && [ "$baseline_time" != "0" ] && [ "$baseline_time" != "0.0000" ]; then
                speedup=$(echo "scale=4; $baseline_time / $TIME" | bc 2>/dev/null)
                if [ -z "$speedup" ]; then
                    speedup="0.0000"
                fi
                # Calcula eficiência
                efficiency=$(echo "scale=4; $speedup / $procs" | bc 2>/dev/null)
                if [ -z "$efficiency" ]; then
                    efficiency="0.0000"
                fi
            else
                speedup="0.0000"
                efficiency="0.0000"
            fi
        fi
        
        echo "      Processos $procs: $TIME segundos (Speedup: $speedup, Eficiência: $efficiency)" >> $RESULTS_FILE
        echo "MPI,${mask}x${mask},${procs},${TIME},${speedup},${efficiency}" >> $CSV_FILE
    done
    echo "" >> $RESULTS_FILE
done

# Testa versão OpenMP
echo "Testando versão OpenMP..."
echo "=== VERSÃO OPENMP ===" >> $RESULTS_FILE

for mask in 3 5 7; do
    echo "  Máscara ${mask}x${mask}..."
    echo "Máscara ${mask}x${mask}:" >> $RESULTS_FILE
    baseline_time=${sequential_times[$mask]}
    
    for threads in 1 2 3 4; do
        echo "    Threads $threads..."
        # Captura o tempo do output do programa (mais preciso)
        TIME=$(./bin/openmp_version $mask $threads $INPUT_FILE 2>&1 | grep "Tempo total" | awk '{print $3}')
        
        if [ -z "$TIME" ] || [ "$TIME" = "" ]; then
            echo "      Erro ao capturar tempo para $threads threads"
            TIME="0.0000"
            speedup="0.0000"
            efficiency="0.0000"
        else
            # Calcula speedup (verifica se baseline_time é válido)
            if [ -n "$baseline_time" ] && [ "$baseline_time" != "0" ] && [ "$baseline_time" != "0.0000" ]; then
                speedup=$(echo "scale=4; $baseline_time / $TIME" | bc 2>/dev/null)
                if [ -z "$speedup" ]; then
                    speedup="0.0000"
                fi
                # Calcula eficiência
                efficiency=$(echo "scale=4; $speedup / $threads" | bc 2>/dev/null)
                if [ -z "$efficiency" ]; then
                    efficiency="0.0000"
                fi
            else
                speedup="0.0000"
                efficiency="0.0000"
            fi
        fi
        
        echo "      Threads $threads: $TIME segundos (Speedup: $speedup, Eficiência: $efficiency)" >> $RESULTS_FILE
        echo "OpenMP,${mask}x${mask},${threads},${TIME},${speedup},${efficiency}" >> $CSV_FILE
    done
    echo "" >> $RESULTS_FILE
done

echo "=== RESUMO EM TABELA ===" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE
echo "Para visualização formatada, use: column -t -s',' $CSV_FILE" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE
echo "Ou visualize diretamente o arquivo CSV: $CSV_FILE" >> $RESULTS_FILE

echo "" >> $RESULTS_FILE
echo "=== TABELA RESUMIDA ===" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE
column -t -s',' $CSV_FILE >> $RESULTS_FILE

echo ""
echo "Resultados detalhados salvos em: $RESULTS_FILE"
echo "Resultados em CSV salvos em: $CSV_FILE"
echo ""
echo "=== RESUMO ===" 
cat $RESULTS_FILE
echo ""
echo "=== VISUALIZAÇÃO FORMATADA DO CSV ==="
column -t -s',' $CSV_FILE

