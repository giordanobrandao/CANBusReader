# CANBusReader

#### Leitor Wi-Fi para Fiat Mobi com ESP32

## Descrição

Um leitor de dados veiculares (OBD2) de baixo custo, baseado em ESP32, que cria uma interface web acessível por Wi-Fi.

## Funcionalidades Principais

* Leitura de dados OBD2 em tempo real.
* Não requer aplicativo, apenas um navegador web.
* Interface web simples e responsiva e customizável.
* Baixo custo e hardware de fácil acesso

## Modificações na interface web

Caso queira implementar mudanças na página web, basta alterar os arquivos no diretório ``` frontend ```. Em seguida, deve-se executar o script em python no terminal da seguinte forma:

#### Arquivo HTML

* ``` python3 ./frontend/main.html ./main/main_html.h MAIN_HTML_H html ```

#### Arquivo CSS

* ``` python3 ./frontend/main.css ./main/main_css.h MAIN_CSS_H css ```

#### Arquivo JS

* ``` python3 ./frontend/main.js ./main/main_js.h MAIN_JS_H js ```

Isto é necessário para converter os códigos em um formato compacto e que o programa entenda.

## Esquemático

![Texto Alternativo](https://github.com/giordanobrandao/CANBusReader/blob/main/esquematico.jpeg?raw=true "Esquemático")

## Ferramentas de desenvolvimento

### IDE

Este projeto é desenvolvido utilizando [Arduino IDE](https://www.arduino.cc/en/software/).

### Bibliotecas

* ``` ESP32-CAN ```
* ``` ESPAsyncWebServer ```
* ``` AsyncTCP ```

## Uso

1. Conecte o dispositivo na porta OBD2 do carro.
2. Ligue a ignição do veículo.
3. No seu smartphone ou notebook, conecte-se ao ponto de acesso do microcontrolador.
4. Abra o navegador e acesse o endereço ``` http://192.168.4.1 ```.
5. A página exibirá os dados do carro.




