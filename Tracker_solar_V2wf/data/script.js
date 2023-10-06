grafico : var ctx = document.getElementById('grafico').getContext('2d');
var socket = new WebSocket("ws://192.168.4.1:81/");

var data = {
    labels: [1],
    datasets: [{
        label: 'Geração de energia',
        data: [1],
        fill: false,
        borderColor: '#00BFFF',
        borderWidth: 1
    }]
};

var config = {
    type: 'line',
    data: data,
    options: {
        scales: {
            x: {
                type: 'linear',
                position: 'bottom'
            },
            y: {
                beginAtZero: true
            }
        }
    }
};

var myChart = new Chart(ctx, config);

function adicionarDado(tempo, valor) {
    // Limita o número de pontos de dados a 15 (ou o valor que você preferir)
    if (myChart.data.labels.length >= 15) {
        myChart.data.labels.shift(); // Remove o ponto de dados mais antigo do início
        myChart.data.datasets[0].data.shift();
    }

    myChart.data.labels.push(tempo);
    myChart.data.datasets[0].data.push(valor);
    myChart.update();

    var maior = document.getElementById('maior');
    var menor = document.getElementById('menor');
    var media = document.getElementById('media');
    var dados = myChart.data.datasets[0].data;
    var soma = dados.reduce((total, valor) => total + valor, 0);
    
    maior.textContent = 'Maior: ' + Math.max(...myChart.data.datasets[0].data).toFixed(2);
    menor.textContent = 'Menor: ' + Math.min(...myChart.data.datasets[0].data).toFixed(2);
    media.textContent = 'Media: ' + (soma / dados.length).toFixed(2);

    var elementoWatts = document.getElementById('watts');
    elementoWatts.textContent = valor.toFixed(2) + " Watts"; 
}

var tempo = 0; // Declarando a variável tempo aqui

// Variável para controlar a taxa de atualização
var ultimoTempoDeAtualizacao = 0;

// Manipulador de eventos para receber dados do WebSocket
socket.onmessage = function(event) {
    var sensorData = parseFloat(event.data);
    var tempoAtual = tempo; // Você pode ajustar isso conforme necessário

    var tempoAtualMillis = new Date().getTime();
    // Verifica se passou pelo menos 1 segundo desde a última atualização
    if (tempoAtualMillis - ultimoTempoDeAtualizacao >= 1000) {
        adicionarDado(tempoAtual, sensorData);
        ultimoTempoDeAtualizacao = tempoAtualMillis;
    }

    tempo++;
};