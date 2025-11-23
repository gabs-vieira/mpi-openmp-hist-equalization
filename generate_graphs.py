#!/usr/bin/env python3
"""
Script para gerar gráficos de speedup e eficiência
a partir do arquivo performance_metrics.csv
Gera arquivo HTML com gráficos interativos usando Chart.js
"""

import csv
import json

# Lê os dados do CSV
data = {
    'MPI': {'3x3': {}, '5x5': {}, '7x7': {}},
    'OpenMP': {'3x3': {}, '5x5': {}, '7x7': {}}
}

with open('performance_metrics.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        modelo = row['Modelo']
        mascara = row['Máscara']
        procs_threads = row['Processos/Threads']
        tempo = float(row['Tempo (s)'])
        speedup = float(row['Speedup'])
        eficiencia = float(row['Eficiência'])
        
        if modelo in ['MPI', 'OpenMP'] and mascara in data[modelo]:
            if procs_threads != '-':
                procs = int(procs_threads)
                if procs not in data[modelo][mascara]:
                    data[modelo][mascara][procs] = {}
                data[modelo][mascara][procs] = {
                    'tempo': tempo,
                    'speedup': speedup,
                    'eficiencia': eficiencia
                }

# Prepara dados para os gráficos
mascaras = ['3x3', '5x5', '7x7']

# Gera HTML com gráficos
html_content = """<!DOCTYPE html>
<html>
<head>
    <title>Análise de Desempenho - Speedup e Eficiência</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            text-align: center;
            color: #333;
        }
        .chart-container {
            margin: 30px 0;
            position: relative;
            height: 400px;
        }
        .chart-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 10px;
            color: #555;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Análise de Desempenho: Speedup e Eficiência</h1>
"""

# Adiciona gráficos para cada máscara
for mascara in mascaras:
    # Prepara dados
    procs_mpi = sorted([k for k in data['MPI'][mascara].keys()])
    speedup_mpi = [data['MPI'][mascara][p]['speedup'] for p in procs_mpi]
    eficiencia_mpi = [data['MPI'][mascara][p]['eficiencia'] for p in procs_mpi]
    
    threads_openmp = sorted([k for k in data['OpenMP'][mascara].keys()])
    speedup_openmp = [data['OpenMP'][mascara][t]['speedup'] for t in threads_openmp]
    eficiencia_openmp = [data['OpenMP'][mascara][t]['eficiencia'] for t in threads_openmp]
    
    # Linha ideal
    max_procs = max(max(procs_mpi) if procs_mpi else 1, max(threads_openmp) if threads_openmp else 1)
    ideal_x = list(range(1, max_procs + 1))
    ideal_y = ideal_x
    
    # Gráfico de Speedup
    html_content += f"""
        <div class="chart-container">
            <div class="chart-title">Speedup - Máscara {mascara}</div>
            <canvas id="speedup_{mascara}"></canvas>
        </div>
        <script>
            const ctx_speedup_{mascara} = document.getElementById('speedup_{mascara}').getContext('2d');
            new Chart(ctx_speedup_{mascara}, {{
                type: 'line',
                data: {{
                    labels: {json.dumps(procs_mpi)},
                    datasets: [
                        {{
                            label: 'Ideal (Speedup Linear)',
                            data: {json.dumps(ideal_y)},
                            borderColor: 'rgba(0, 0, 0, 0.3)',
                            borderDash: [5, 5],
                            pointRadius: 0,
                            borderWidth: 1
                        }},
                        {{
                            label: 'MPI',
                            data: {json.dumps(speedup_mpi)},
                            borderColor: 'rgb(31, 119, 180)',
                            backgroundColor: 'rgba(31, 119, 180, 0.1)',
                            pointRadius: 6,
                            pointBackgroundColor: 'rgb(31, 119, 180)',
                            borderWidth: 2
                        }},
                        {{
                            label: 'OpenMP',
                            data: {json.dumps(speedup_openmp)},
                            borderColor: 'rgb(148, 103, 189)',
                            backgroundColor: 'rgba(148, 103, 189, 0.1)',
                            pointRadius: 6,
                            pointBackgroundColor: 'rgb(148, 103, 189)',
                            borderWidth: 2,
                            borderDash: [5, 5]
                        }}
                    ]
                }},
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {{
                        x: {{
                            title: {{
                                display: true,
                                text: 'Processos/Threads'
                            }}
                        }},
                        y: {{
                            title: {{
                                display: true,
                                text: 'Speedup'
                            }},
                            beginAtZero: true
                        }}
                    }},
                    plugins: {{
                        legend: {{
                            display: true,
                            position: 'top'
                        }}
                    }}
                }}
            }});
        </script>
    """
    
    # Gráfico de Eficiência
    html_content += f"""
        <div class="chart-container">
            <div class="chart-title">Eficiência - Máscara {mascara}</div>
            <canvas id="eficiencia_{mascara}"></canvas>
        </div>
        <script>
            const ctx_eficiencia_{mascara} = document.getElementById('eficiencia_{mascara}').getContext('2d');
            new Chart(ctx_eficiencia_{mascara}, {{
                type: 'line',
                data: {{
                    labels: {json.dumps(procs_mpi)},
                    datasets: [
                        {{
                            label: 'Ideal (Eficiência = 1.0)',
                            data: [1.0, 1.0, 1.0, 1.0],
                            borderColor: 'rgba(0, 0, 0, 0.3)',
                            borderDash: [5, 5],
                            pointRadius: 0,
                            borderWidth: 1
                        }},
                        {{
                            label: 'MPI',
                            data: {json.dumps(eficiencia_mpi)},
                            borderColor: 'rgb(31, 119, 180)',
                            backgroundColor: 'rgba(31, 119, 180, 0.1)',
                            pointRadius: 6,
                            pointBackgroundColor: 'rgb(31, 119, 180)',
                            borderWidth: 2
                        }},
                        {{
                            label: 'OpenMP',
                            data: {json.dumps(eficiencia_openmp)},
                            borderColor: 'rgb(148, 103, 189)',
                            backgroundColor: 'rgba(148, 103, 189, 0.1)',
                            pointRadius: 6,
                            pointBackgroundColor: 'rgb(148, 103, 189)',
                            borderWidth: 2,
                            borderDash: [5, 5]
                        }}
                    ]
                }},
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {{
                        x: {{
                            title: {{
                                display: true,
                                text: 'Processos/Threads'
                            }}
                        }},
                        y: {{
                            title: {{
                                display: true,
                                text: 'Eficiência'
                            }},
                            beginAtZero: true,
                            max: 1.2
                        }}
                    }},
                    plugins: {{
                        legend: {{
                            display: true,
                            position: 'top'
                        }}
                    }}
                }}
            }});
        </script>
    """

html_content += """
    </div>
</body>
</html>
"""

# Salva o arquivo HTML
with open('performance_graphs.html', 'w') as f:
    f.write(html_content)

print("Gráficos gerados em: performance_graphs.html")
print("Abra o arquivo no navegador para visualizar os gráficos interativos.")
