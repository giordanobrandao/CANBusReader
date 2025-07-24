#ifndef MAIN_HTML_H
#define MAIN_HTML_H

const char main_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>Painel Fiat Mobi</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        %MAIN_CSS%
    </style>
    <script>
        %MAIN_JS%
    </script>
</head>
<body>
    <h1>Painel de Instrumentos - Fiat Mobi OBD2</h1>
    <div class="card-container">
        <div class="card">
        <h2>RPM</h2>
        <p><span class="reading" id="rpm">0</span><span class="unit"> rpm</span></p>
    </div>
    <div class="card">
        <h2>Velocidade</h2>
        <p><span class="reading" id="speed">0</span><span class="unit"> km/h</span></p>
    </div>
    <div class="card">
        <h2>Temperatura Motor</h2>
        <p><span class="reading" id="temp">0</span><span class="unit"> &deg;C</span></p>
    </div>
    <div class="card">
        <h2>Carga do Motor</h2>
        <p><span class="reading" id="load">0.0</span><span class="unit"> %</span></p>
    </div>
  </div>
</body>
</html>
)rawliteral";

#endif
