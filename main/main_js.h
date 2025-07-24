#ifndef MAIN_JS_H
#define MAIN_JS_H

const char main_js[] PROGMEM = R"rawliteral(
var gateway=`ws:var websocket;window.addEventListener('load',onLoad);function onLoad(event){initWebSocket();}
function initWebSocket(){console.log('Tentando conectar ao WebSocket...');websocket=new WebSocket(gateway);websocket.onopen=onOpen;websocket.onclose=onClose;websocket.onmessage=onMessage;}
function onOpen(event){console.log('Conexão estabelecida.');}
function onClose(event){console.log('Conexão perdida.');setTimeout(initWebSocket,2000);}
function onMessage(event){console.log(event.data);var data=JSON.parse(event.data);document.getElementById('rpm').innerHTML=data.rpm;document.getElementById('speed').innerHTML=data.speed;document.getElementById('temp').innerHTML=data.coolantTemp;document.getElementById('load').innerHTML=data.engineLoad.toFixed(1);}
)rawliteral";

#endif
