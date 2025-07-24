var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// Inicia a conexão quando a página carrega
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Tentando conectar ao WebSocket...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Conexão estabelecida.');
}

function onClose(event) {
    console.log('Conexão perdida.');
    setTimeout(initWebSocket, 2000); // Tenta reconectar a cada 2 segundos
}

// Função chamada quando uma mensagem é recebida do ESP32
function onMessage(event) {
    console.log(event.data);
    var data = JSON.parse(event.data); // Converte a string JSON em um objeto
    document.getElementById('rpm').innerHTML = data.rpm;
    document.getElementById('speed').innerHTML = data.speed;
    document.getElementById('temp').innerHTML = data.coolantTemp;
    document.getElementById('load').innerHTML = data.engineLoad.toFixed(1);
}