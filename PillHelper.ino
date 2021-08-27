/********************************************************************************
* ARQUIVOS AUXILIARES
********************************************************************************/
#include "PillHelper-lib.h"

/********************************************************************************
* TRATAMENTO DAS REQUISÇÕES WEB
********************************************************************************/
void handleConfig() {
    // Config
    File file = SPIFFS.open(F("/config.html"), "r");
    if (file) {
        file.setTimeout(100);
        String s = file.readString();
        file.close();

        String redes = "";

        // WiFi.scanNetworks will return the number of networks found
        int n = WiFi.scanNetworks();
        log("Scan realizado");
        if (n == 0) {
            log("Scan - Nenhuma rede encontrada");
            redes += "<tr><td class=\"center\">Nenhuma rede encontrada!</td></tr>";
        } else {
            log("Scan - " + String(n) + " rede(s) encontrada(s)");
            redes += "<tr><th>SSID (Rede)</th><th>Segurança</th><th>Sinal</th></tr>"; // Cabeçalho da tabela
            for (int i = 0; i < n; ++i) {
                redes += "<tr>";
                redes += "<td><button class=\"link\" onclick=\"ssidWifi('" + WiFi.SSID(i) + "');\">" + WiFi.SSID(i) + "</button></td>"; // Botão com o nome da rede WiFi
                redes += "<td>";
                 
                // Rede com senha ou sem senha
                if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
                  redes += "<img class=\"center\" src=\"cadeado-aberto.png\" alt=\"Rede sem proteção por senha\" title=\"Rede sem senha\">";
                } else {
                  redes += "<img class=\"center\" src=\"cadeado-fechado.png\" alt=\"Rede com proteção por senha\" title=\"Rede com senha\">";
                }
                
                redes += "</td>";

                // Converte o sinal do WiFi de dBm para Porcentagem
                int sinal = 2 * (WiFi.RSSI(i) + 100);

                redes += "<td class=\"center\">" + String(sinal) + " %</td>";
                redes += "</tr>";
                // Print SSID and RSSI for each network found
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(")");
                Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
                delay(10);
            }
        }

        // Atualiza conteúdo dinâmico
        // Parâmetros
        s.replace(F("#status#")     , (WiFi.status() == WL_CONNECTED ? String("Conectado!<br>SSID: ") + cWifiSSID :  String("Não conectado!")) );
        s.replace(F("#ID#")         , hexStr(ESP.getEfuseMac()));
        s.replace(F("#timeZone#")   , String(cTimeZone));
        s.replace(F("#redes#")      , redes);

        // Send data
        server.send(200, F("text/html"), s);
        log("Config - Cliente: " + ipStr(server.client().remoteIP()));
    } else {
        server.send(500, F("text/plain"), F("Config - ERROR 500"));
        log(F("Config - ERRO lendo arquivo"));
    }
}

void handleConfigSave() {
    // Grava Config
    // Verifica número de campos recebidos
    // ESP32, caso pelo menos um paramentro enviado, executa código abaixo
    if (server.args() >= 1) {

        String s;

        // Parâmetro cTimeZone
        cTimeZone = server.arg(F("timeZone")).toInt();
        // Parâmetro cWifiSSID
        s = server.arg(F("ssid"));
        s.trim();
        strlcpy(cWifiSSID, s.c_str(), sizeof(cWifiSSID)); 
        // Parâmetro cWifiPW
        s = server.arg(F("wifiPW"));
        s.trim();
        if (s == "*") {
            s = cWifiPW;
        }
        strlcpy(cWifiPW, s.c_str(), sizeof(cWifiPW)); 

        // Grava configuração
        if (configSave()) {
            server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Configuração salva.');history.back()</script></html>"));
            log("ConfigSave - Cliente: " + ipStr(server.client().remoteIP()));
        } else {
            server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Falha salvando configuração.');history.back()</script></html>"));
            log(F("ConfigSave - ERRO salvando Config"));
        }
    } else {
        server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Erro de parâmetros.');history.back()</script></html>"));
    }
}

void handleReconfig() {
    // Reinicia Config
    configReset();

    // Grava configuração
    if (configSave()) {
        server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Configuração reiniciada.');window.location = '/'</script></html>"));
        log("Reconfig - Cliente: " + ipStr(server.client().remoteIP()));
    } else {
        server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Falha reiniciando configuração.');history.back()</script></html>"));
        log(F("Reconfig - ERRO reiniciando Config"));
    }
}

void handleReboot() {
    log("Reboot - Cliente: " + ipStr(server.client().remoteIP()));
    delay(100);
    ESP.restart();
}

void handleStream(const String f, const String mime) {
    // Processa requisição de arquivo
    File file = SPIFFS.open("/" + f, "r");
    if (file && !file.isDirectory()) {
        // Define cache para 2 dias
        server.sendHeader(F("Cache-Control"), F("public, max-age=172800"));
        server.streamFile(file, mime);
        file.close();
        log(f + " - Cliente: " + ipStr(server.client().remoteIP()));
    } else {
        server.send(500, F("text/plain"), F("Stream - ERROR 500"));
        log(F("Stream - ERRO lendo arquivo"));
    }
}

void handleCadeadoAberto() {
    // Cadeado Aberto
    handleStream(F("cadeado-aberto.png"), F("image/png"));
}

void handleCadeadoFechado() {
    // Cadeado Fechado
    handleStream(F("cadeado-fechado.png"), F("image/png"));
}

void handleJavaScript() {
    // JavaScript
    handleStream(F("js.js"), F("application/javascript"));
}

void handleCSS() {
    // CSS
    handleStream(F("style.css"), F("text/css"));
}

/********************************************************************************
* SETUP
********************************************************************************/
void setup() {

    Serial.begin(115200);

    log("\nIniciando...");

    // SPIFFS
    if (!SPIFFS.begin()) {
        log(F("SPIFFS ERRO"));
        while (true);
    }

    // Lê configuração
    configRead();

    // Fita de LED
    pixels.begin();
    clearLeds();

    // LED
    pinMode(ESP_LED, OUTPUT);

    // Buzzer
    pinMode(BUZZER_PIN, OUTPUT);

    // Botões
    pinMode(BTN_UPDATE_PIN, INPUT);
    pinMode(BTN_STOP_PIN, INPUT);

    // Schedule entries
    schedule = scheduleGet();

    // Conecta WiFi
    WiFi.begin(cWifiSSID, cWifiPW);
    log("Conectando WiFi " + String(cWifiSSID));
    byte b = 0;
    while(WiFi.status() != WL_CONNECTED && b < 10) {
        b++;
        digitalWrite(ESP_LED, HIGH);
        delay(500);
        digitalWrite(ESP_LED, LOW);
        Serial.print(".");
        delay(500);
    }
    Serial.println();

    // WiFi Access Point
    if(WiFi.status() != WL_CONNECTED)
    {
        // SoftAP ESP32
        // WiFi.setHostname("PillHelper");
        WiFi.softAP("PillHelper", "123456789");

        log("WiFi AP 'PillHelper' - IP " + ipStr(WiFi.softAPIP()));

        // Set flag
        softAPActive = true;

        // Habilita roteamento DNS
        dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
        dnsServer.start(DNSSERVER_PORT, "*", WiFi.softAPIP());
    }

    if (WiFi.status() == WL_CONNECTED) { // Se estiver conectado na rede WiFi
        // WiFi Station conectado
        log("WiFi conectado (" + String(WiFi.RSSI()) + ") IP " + ipStr(WiFi.localIP()));
        digitalWrite(ESP_LED, LOW);
    } else {
        log(F("WiFi não conectado"));
        digitalWrite(ESP_LED, HIGH);
    }

    // Define NTP
    setSyncProvider(timeNTP);
    setSyncInterval(NTP_INT);
    log(timeStatus() == timeSet ? F("Data/hora atualizada") : F("Falha ao atualizar Data/hora"));

    // mDNS
    if (!MDNS.begin(cDeviceID)) {
        log(F("mDNS ERRO"));
    }

    // WebServer
    server.on(F("/config"), handleConfig);
    server.on(F("/configSave"), handleConfigSave);
    server.on(F("/reconfig"), handleReconfig);
    server.on(F("/reboot"), handleReboot);
    server.on(F("/cadeado-aberto.png"), handleCadeadoAberto);
    server.on(F("/cadeado-fechado.png"), handleCadeadoFechado);
    server.on(F("/style.css"), handleCSS);
    server.onNotFound(handleConfig);
    server.collectHeaders(WEBSERVER_HEADER_KEYS, 1);
    server.begin();

    // Pronto
    log(F("Pronto"));
}

/********************************************************************************
* LOOP
********************************************************************************/
void loop() {
    // WatchDog ----------------------------------------
    yield();

    // DNS Requests ------------------------------------
    if (softAPActive) {
        dnsServer.processNextRequest();
    }

    // Web ---------------------------------------------
    server.handleClient();

    // Schedule ----------------------------------------
    s = scheduleChk(schedule);
    if (s != "") {
        // Event detected
        String lastEvent = "Alarme disparado " + s + " - " + dateTimeStr(now(), cTimeZone);
        log("Agendamento " + lastEvent);
    }

    // Botão de atualizar os alarmes pela API ----------
    if (digitalRead(BTN_UPDATE_PIN) == HIGH) {
        log("Botão de Update pressionado");
        if(WiFi.status()== WL_CONNECTED) {
            apiCheck();
        }
    }

    // Botão de desligar os alarmes --------------------
    if (digitalRead(BTN_STOP_PIN) == HIGH) {
        log("Botão de Stop pressionado");
        delay(250);
        flagBuzzer = 0;
        delay(250);
        flagLed = -1;
        delay(250);
        clearLeds();
        delay(30000);
        log("Timer Stop esgotado");
    }
    
    // Checa se ativa o buzzer e o led ----------------
    ligaLed();
    buzzerActivate();
    
    // // API ---------------------------------------------
    // if ((millis() - lastTime) > timerDelay) {
    //     if(WiFi.status()== WL_CONNECTED) {
    //         apiCheck();
    //     }
    //     lastTime = millis();
    // } // Fim API

}
