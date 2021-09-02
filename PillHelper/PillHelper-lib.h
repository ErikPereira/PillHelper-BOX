/********************************************************************************
* BIBLIOTECAS E ARQUIVOS AUXILIARES
********************************************************************************/
#include "PillHelper-def.h"

/********************************************************************************
* FUNÇÕES
********************************************************************************/

int ledDictionary(int value){
    switch (value) {
        case 0:
            return 1;   
        break;
        case 1:
            return 1;
        break;
        case 2:
            return 4;
        break;
        case 3:
            return 7;   
        break;
        case 4:
            return 9;
        break;
        case 5:
            return 18;
        break;
        case 6:
            return 16;   
        break;
        case 7:
            return 13;
        break;
        case 8:
            return 10;
        break;
        case 9:
            return 21;   
        break;
        case 10:
            return 24;
        break;
        case 11:
            return 27;
        break;
        case 12:
            return 29;
        break;
        default:
            return 1;
        break;
    }
}

String ipStr(const IPAddress &ip) {
    // Retorna IPAddress como "n.n.n.n"
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

String hexStr(const unsigned long &h, const byte &l = 8) {
    // Retorna valor em formato hexadecimal
    String s;
    s= String(h, HEX);
    s.toUpperCase();
    s = ("00000000" + s).substring(s.length() + 8 - l);
    return s;
}

String deviceID() {
    // Retorna ID padrão
    return "CaixaInteligente-" + hexStr(ESP.getEfuseMac());
}

void hold(const unsigned int &ms) {
    // Delay
    unsigned long m = millis();
    while (millis() - m < ms) {
        yield();
    }
}

time_t iso8601DateTime(String s, const int8_t tz = 0) {
    // Retorna ISO8601 como time_t
    TimeElements te;
    te.Year   = s.substring( 0,  4).toInt() - 1970;
    te.Month  = s.substring( 5,  7).toInt();
    te.Day    = s.substring( 8, 10).toInt();
    te.Hour   = s.substring(11, 13).toInt();
    te.Minute = s.substring(14, 16).toInt();
    te.Second = s.substring(17, 19).toInt();
    time_t t  = makeTime(te);
    t-= s.substring(19, 22).toInt() * 3600; // Adjusta para UTC;
    t+= tz * 3600;                          // Adjusta para hora local
    return t;
}

String dateTimeStr(time_t t, const int8_t tz = 0, const bool flBr = true) {
    // Retorna time_t como "yyyy-mm-dd hh:mm:ss" ou "dd/mm/yyyy hh:mm:ss"
    if (timeStatus() == timeNotSet) {
        return flBr ? F("00/00/0000 00:00:00") : F("0000-00-00 00:00:00");
    }
    t += tz * 3600; // Adjusta TimeZone
    String s;
    if (flBr) {
        // dd/mm/yyyy hh:mm:ss
        s = "";
        if (day(t) < 10) {
            s += '0';
        }
        s += String(day(t)) + '/';
        if (month(t) < 10) {
            s += '0';
        }
        s += String(month(t)) + '/' + String(year(t)) + ' ';
    } else {
        // yyyy-mm-dd hh:mm:ss
        s = String(year(t)) + '-';
        if (month(t) < 10) {
            s += '0';
        }
        s += String(month(t)) + '-';
        if (day(t) < 10) {
            s += '0';
        }
        s += String(day(t)) + ' ';
    }
    if (hour(t) < 10) {
        s += '0';
    }
    s += String(hour(t)) + ':';
    if (minute(t) < 10) {
        s += '0';
    }
    s += String(minute(t)) + ':';
    if (second(t) < 10) {
        s += '0';
    }
    s += String(second(t));
    return s;
}

// Função de Atualização de Data/Hora -------------------
time_t timeNTP() {
    // Retorna time_t do Servidor NTP
    if (WiFi.status() != WL_CONNECTED) {
        // Sem conexão WiFi
        return 0;
    }

    // Servidor NTP
    const char  NTP_SERVER[]    = "pool.ntp.br";
    const byte  NTP_PACKET_SIZE = 48;
    const int   UDP_LOCALPORT   = 2390;
    byte        ntp[NTP_PACKET_SIZE];
    memset(ntp, 0, NTP_PACKET_SIZE);
    ntp[0] = 0b11100011;
    ntp[1] = 0;
    ntp[2] = 6;
    ntp[3] = 0xEC;
    ntp[12] = 49;
    ntp[13] = 0x4E;
    ntp[14] = 49;
    ntp[15] = 52;
    WiFiUDP udp;
    udp.begin(UDP_LOCALPORT);
    udp.beginPacket(NTP_SERVER, 123);
    udp.write(ntp, NTP_PACKET_SIZE);
    udp.endPacket();
    hold(2000);
    unsigned long l;
    if (udp.parsePacket()) {
        // Sucesso
        udp.read(ntp, NTP_PACKET_SIZE);
        l = word(ntp[40], ntp[41]) << 16 | word(ntp[42], ntp[43]);
        l -= 2208988800UL;
    } else {
        //Erro
        l = 0;
    }
    // Retorna Data/Hora
    return l;
}

void log(const String &s) {
    // Gera log na Serial
    Serial.println("Log: " + s);
}

/********************************************************************************
* BUZZER
********************************************************************************/
void buzzerActivate(){
    if(flagBuzzer != 0){
        digitalWrite(BUZZER_PIN, HIGH);
        delay(250);
        digitalWrite(BUZZER_PIN, LOW);
        delay(250);
    }
}

/********************************************************************************
* FITA DE LED
********************************************************************************/
void ligaLed(){
    if(flagLed != -1){ 
        pixels.setPixelColor(flagLed, pixels.Color(0, 150, 0));
        pixels.show();
        delay(150);
    }
}

void apagaLed(int pin) {
    pixels.setPixelColor(pin, 0); //Desaciona o LED requerido
    pixels.show(); //Atualiza o LED
    delay(150);
}

//Função apaga LEDs geral
void clearLeds() {
    pixels.clear();
    for (int i = 0; i < NUMLED; i++) {
        pixels.setPixelColor(i, 0);
        pixels.show();
        delay(150);
    }
}

/********************************************************************************
* FUNÇÕES DE CONFIGURAÇÃO
********************************************************************************/
void configReset() {
    // Define configuração padrão
    strlcpy(cDeviceID   , deviceID().c_str()    , sizeof(cDeviceID)); 
    // Parâmetros
    cTimeZone           = -3;
    // Wi-Fi
    strlcpy(cWifiSSID   , "REDE"    , sizeof(cWifiSSID)); 
    strlcpy(cWifiPW     , "PASS"    , sizeof(cWifiPW)); 
}

boolean configRead() {
    // Lê configuração
    StaticJsonDocument<JSON_CONFIG_SIZE> jsonConfig;

    File file = SPIFFS.open(F("/Config.json"), "r");
    log("");
    log("total de Bytes: ");
    log(SPIFFS.totalBytes());
    log("total de uso de Bytes: ");
    log(SPIFFS.usedBytes());
    log("total de Bytes livre: ");
    log(SPIFFS.totalBytes() - SPIFFS.usedBytes());
    if (deserializeJson(jsonConfig, file)) {
        // Falha na leitura, assume valores padrão
        configReset();
        log(F("Falha lendo CONFIG, assumindo valores padrão."));
        return false;
    } else {
        // Sucesso na leitura
        // Dispositivo
        strlcpy(cDeviceID   , jsonConfig["DeviceID"]    | deviceID().c_str()    , sizeof(cDeviceID));
        // Parâmetros
        cTimeZone           = jsonConfig["Timezone"]    | 0;
        // Wi-Fi
        strlcpy(cWifiSSID   , jsonConfig["Wifi-SSID"]   | ""                    , sizeof(cWifiSSID));
        strlcpy(cWifiPW     , jsonConfig["Wifi-PW"]     | ""                    , sizeof(cWifiPW));

        file.close();

        log(F("\nLendo config:"));
        serializeJsonPretty(jsonConfig, Serial);
        log(F("\n"));

        return true;
    }
}

boolean configSave() {
    // Grava configuração
    StaticJsonDocument<JSON_CONFIG_SIZE> jsonConfig;

    File file = SPIFFS.open(F("/Config.json"), "w+");
    if (file) {
        // Atribui valores ao JSON e grava
        // Dispositivo
        jsonConfig["DeviceID"] = cDeviceID;
        // Parâmetros
        jsonConfig["Timezone"] = cTimeZone;
        // Wi-Fi
        jsonConfig["Wifi-SSID"] = cWifiSSID;
        jsonConfig["Wifi-PW"] = cWifiPW;

        serializeJsonPretty(jsonConfig, file);
        file.close();

        log(F("\nGravando config:"));
        serializeJsonPretty(jsonConfig, Serial);
        log(F("\n"));

        return true;
    }
    return false;
}

/*******************************************************************************
* SCHEDULE SYSTEM FUNCTIONS
* Options:
*   - Weekly
*     On (High): WHd hh:mm
*     Off (Low): WLd hh:mm
*   - Daily
*     On (High): DHhh:mm
*     Off (Low): DLhh:mm
* Example Strings:
*   WL6 16:30           - set Off weekly on Fridays 16:30 
*   DH16:30             - set On daily at 16:30
*******************************************************************************/
String hhmmStr(const time_t &t) {
    // Return time_t as String "hh:mm"
    String s = "";
    if (hour(t) < 10) {
        s += '0';
    }
    s += String(hour(t)) + ':';
    if (minute(t) < 10) {
        s += '0';
    }
    s += String(minute(t));
    return s;
}

String scheduleChk(const String &schedule) {
    // Schedule System Main Function
    
    // Local variables
    String event = "";
    String pos_led = "";
    String alarme_sonoro = "0";
    byte status;
    static time_t lastCheck = 0,
                highDT    = (now() + ((cTimeZone)*3600)),
                lowDT     = highDT;

    // Check for Interval reset
    if (schedule == "") {
        highDT = (now() + ((cTimeZone)*3600)),
        lowDT = highDT;
        return "";
    }

    // Check for minimum interval (10s)
    if ((now() + ((cTimeZone)*3600)) - lastCheck < 10) {
        return "";
    }

    // Get DateTime
    lastCheck = (now() + ((cTimeZone)*3600));

    // Get DateTime as "yyyy-mm-dd hh:mm" String
    String dt = String(year(lastCheck)) + '-';
    if (month(lastCheck) < 10) {
        dt += '0';
    }
    dt += String(month(lastCheck)) + '-';
    if (day(lastCheck) < 10) {
        dt += '0';
    }
    dt += String(day(lastCheck)) + ' ';
    if (hour(lastCheck) < 10) {
        dt += '0';
    }
    dt += String(hour(lastCheck)) + ':';
    if (minute(lastCheck) < 10) {
        dt += '0';
    }
    dt += String(minute(lastCheck));

    // Get DateTime as "dd hh:mm" String
    dt = dt.substring(8);

    // Get DateTime as "d hh:mm" String
    dt = String(weekday(lastCheck)) + dt.substring(2);
  
    // Check Weekly High - WHd hh:mm
    s = "WH" + dt;
    if (schedule.indexOf(s) != -1) {
        pos_led = String(schedule[schedule.indexOf(s) + 11]) + String(schedule[schedule.indexOf(s) + 12]);
        alarme_sonoro = String(schedule[schedule.indexOf(s) + 15]);
        log("Weekly - POS LED: " + pos_led + " SIRENE: " + alarme_sonoro);
        event = s;
        status = HIGH;
        goto process;
    }

    // Get DateTime as "hh:mm" String
    dt = dt.substring(2);

    // Check Daily High - DHhh:mm
    s = "DH " + dt;
    if (schedule.indexOf(s) != -1) {
        pos_led = String(schedule[schedule.indexOf(s) + 9]) + String(schedule[schedule.indexOf(s) + 10]);
        alarme_sonoro = String(schedule[schedule.indexOf(s) + 13]);
        log("Daily - POS LED: " + pos_led + " SIRENE: " + alarme_sonoro);
        event = s;
        status = HIGH;
        goto process;
    }

    process:  // Process event
    if (event != "") {
        flagLed = ledDictionary(pos_led.toInt());
        flagBuzzer = alarme_sonoro.toInt();
        if (status) {
            // Store HIGH DateTime
            highDT = lastCheck;
        } else {
            // Store LOW DateTime
            lowDT = lastCheck;
        }
        return event;
    }
    return "";
}

boolean scheduleSet(const String &schedule) {
    // Save Schedule entries
    File file = SPIFFS.open(F("/Schedule.txt"), "w+");
    if (file) {
        file.print(schedule);
        file.close();
        scheduleChk("");
        return true;
    }
    return false;
}

String scheduleGet() {
    // Get Schedule entries
    String s = "";
    File file = SPIFFS.open(F("/Schedule.txt"), "r");
    if (file) {
        file.setTimeout(100);
        s = file.readString();
        file.close();
    }
    return s;
}

/********************************************************************************
* FUNÇÕES DA API
********************************************************************************/
void apiCheck() {
    HTTPClient http;
    String response;
    String alarmes_schedule = "";

    http.begin(API_URL);
    http.addHeader("Authorization", TOKEN_ACCESS);
    http.addHeader("content-type", "application/json");
    String httpRequestData = "{\"uuidBox\": \"" + hexStr(ESP.getEfuseMac()) + "\"}";      
    int httpResponseCode = http.POST(httpRequestData);
    
    log("HTTP Response code: " + String(httpResponseCode));

    if(httpResponseCode == 200) {
        response = http.getString();
        deserializeJson(doc, response);
        int i = 0;
        while(doc["response"]["alarms"][i]) {       
            const char* msg_alarme = doc["response"]["alarms"][i];
            alarmes_schedule += (String(msg_alarme) + "\n");
            i++;
        }
        scheduleSet(alarmes_schedule);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
    }
    else {
      digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
    }

    http.end();

    schedule = scheduleGet();
    log(schedule);
}
