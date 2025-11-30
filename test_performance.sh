#!/bin/bash
# ./test_performance.sh <arquivo_entrada>

if [ $# -ne 1 ]; then
    echo "Uso: $0 <arquivo_entrada>"
    echo "Exemplo: $0 data/img.bmp"
    exit 1
fi

INPUT_FILE=$1
OUTPUT_DIR="output"
CSV_FILE="performance_metrics.csv"

# Limpa arquivo CSV anterior
> $CSV_FILE

# Cabeçalho CSV
echo "Modelo,Máscara,Tarefas,Tempo Médio (s)" >> $CSV_FILE

# Função para validar se um valor é um número de ponto flutuante
validate_float() {
    local value=$1
    # Regex para validar número de ponto flutuante (ex: 0.512345, 1.23, 0.5, 123.456, etc)
    # Aceita: números inteiros, números com ponto decimal
    if [[ $value =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        return 0  # válido
    else
        return 1  # inválido
    fi
}

# Função para executar um programa e capturar o tempo
# Parâmetros: comando, modelo, máscara, tarefas
capture_time() {
    local cmd=$1
    local model=$2
    local mask=$3
    local tasks=$4
    
    local time_value
    local output
    
    # Executa o comando e captura a saída
    output=$($cmd 2>&1)
    
    # Captura o tempo usando grep e cut (pega a última ocorrência caso haja múltiplas)
    time_value=$(echo "$output" | grep "TEMPO_TOTAL" | tail -1 | cut -d'=' -f2)
    
    # Remove espaços em branco e quebras de linha
    time_value=$(echo "$time_value" | tr -d '[:space:]')
    
    # Valida o valor
    if [ -z "$time_value" ] || ! validate_float "$time_value"; then
        echo "AVISO: Tempo capturado inválido para $model, máscara ${mask}x${mask}, tarefas $tasks" >&2
        echo "       Valor capturado: '$time_value'" >&2
        echo "       Saída do programa:" >&2
        echo "$output" | head -20 >&2
        return 1
    fi
    
    echo "$time_value"
    return 0
}

# Função para executar múltiplas vezes e calcular média
# Parâmetros: comando, modelo, máscara, tarefas, num_executions
run_and_average() {
    local cmd=$1
    local model=$2
    local mask=$3
    local tasks=$4
    local num_executions=$5
    
    local times=()
    local valid_executions=0
    local time_value
    local sum=0
    local average
    
    echo "  Executando $model, máscara ${mask}x${mask}, tarefas $tasks..." >&2
    
    while [ $valid_executions -lt $num_executions ]; do
        time_value=$(capture_time "$cmd" "$model" "$mask" "$tasks")
        capture_exit_code=$?
        
        if [ $capture_exit_code -eq 0 ] && [ -n "$time_value" ]; then
            times+=($time_value)
            sum=$(echo "$sum + $time_value" | bc -l)
            valid_executions=$((valid_executions + 1))
            echo "    Execução $valid_executions/$num_executions: $time_value segundos" >&2
        else
            echo "    Tentando novamente..." >&2
        fi
    done
    
    # Calcula a média com 6 casas decimais
    average=$(echo "scale=6; $sum / $num_executions" | bc -l)
    
    # Normaliza o número: se começar com ponto, adiciona zero antes
    # bc pode retornar números como .499203 em vez de 0.499203
    if [[ $average =~ ^\.[0-9]+$ ]]; then
        average="0$average"
    fi
    
    # Garante que tem exatamente 6 casas decimais
    # Separa parte inteira e decimal
    if [[ $average =~ ^([0-9]+)\.([0-9]+)$ ]]; then
        integer_part="${BASH_REMATCH[1]}"
        decimal_part="${BASH_REMATCH[2]}"
        
        # Preenche ou trunca a parte decimal para ter exatamente 6 dígitos
        while [ ${#decimal_part} -lt 6 ]; do
            decimal_part="${decimal_part}0"
        done
        if [ ${#decimal_part} -gt 6 ]; then
            decimal_part=$(echo "$decimal_part" | cut -c1-6)
        fi
        
        average="${integer_part}.${decimal_part}"
    elif [[ $average =~ ^([0-9]+)$ ]]; then
        # Se for apenas inteiro, adiciona .000000
        average="${average}.000000"
    fi
    
    echo "$average"
}

# Testa versão sequencial
echo "Testando versão sequencial..."

for mask in 3 5 7; do
    cmd="./bin/sequential $mask $INPUT_FILE"
    avg_time=$(run_and_average "$cmd" "Sequencial" "$mask" "-" 5)
    echo "Sequencial,${mask}x${mask},-,${avg_time}" >> $CSV_FILE
done

echo ""

# Testa versão MPI
echo "Testando versão MPI..."

for mask in 3 5 7; do
    for procs in 1 2 3 4; do
        cmd="mpirun -np $procs ./bin/mpi_version $mask $INPUT_FILE"
        avg_time=$(run_and_average "$cmd" "MPI" "$mask" "$procs" 5)
        echo "MPI,${mask}x${mask},${procs},${avg_time}" >> $CSV_FILE
    done
done

echo ""

# Testa versão OpenMP
echo "Testando versão OpenMP..."

for mask in 3 5 7; do
    for threads in 1 2 3 4; do
        cmd="./bin/openmp_version $mask $threads $INPUT_FILE"
        avg_time=$(run_and_average "$cmd" "OpenMP" "$mask" "$threads" 5)
        echo "OpenMP,${mask}x${mask},${threads},${avg_time}" >> $CSV_FILE
    done
done

echo ""
echo "Benchmark concluído!"
echo "Resultados salvos em: $CSV_FILE"
echo ""
echo "=== VISUALIZAÇÃO FORMATADA DO CSV ==="
column -t -s',' $CSV_FILE
