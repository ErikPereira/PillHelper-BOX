/********************************************************************************
* BIBLIOTECAS
********************************************************************************/
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdate.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

/********************************************************************************
* PINOS
********************************************************************************/
const byte ESP_LED = 2;
const byte BUZZER_PIN = 12;
const byte LED_TAPE_PIN = 13;
const byte BTN_UPDATE_PIN = 26;
const byte BTN_STOP_PIN = 25;

const byte NUMLED = 32; // Quantidade de leds que exitem na fita

/********************************************************************************
* DEFINIÇÕES DE CONSTANTES
********************************************************************************/
// URL da API
const char* API_URL = "http://192.168.1.11:3000/getAlarms";
const char* TOKEN_ACCESS = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiZGV2Iiwic3lzdGVtIjoiQVBJIn0.6lrvVMtHdb6P5f-Au4c36SK1OT8kb0_gg5BuEok_TpU";
    
// Intervalo NTP (60s * 60m * 6 = 6h)
const int NTP_INT = 60 * 60 * 6;

// Headers do Servidor Web
const char* WEBSERVER_HEADER_KEYS[] = {"User-Agent"};

// Porta Servidor DNS
const byte DNSSERVER_PORT = 53;

// Porta do servidor Web
const byte WEBSERVER_PORT = 8888;

// Número máximo de arquivos no SPIFFS
const byte DIRECTORY_MAX_FILES = 20;

// Tamanho do Objeto JSON
const size_t JSON_CONFIG_SIZE = JSON_OBJECT_SIZE(4) + 160;
const size_t JSON_VCS_SIZE = JSON_OBJECT_SIZE(8) + 390;

/********************************************************************************
* ENTIDADES
********************************************************************************/
// Web Server    
WebServer server(WEBSERVER_PORT);

// DNS Server
DNSServer dnsServer;

// WiFi Client
WiFiClient espClient;

// Adafruit NeoPixel (Fita de LEDS)
Adafruit_NeoPixel pixels(NUMLED, LED_TAPE_PIN, NEO_GRB + NEO_KHZ800);

/********************************************************************************
* VARIÁVEIS
********************************************************************************/
// Flag de Modo AP
bool softAPActive = false;

// Dispositivo
char cDeviceID[35]; // Identificação do dispositivo

// Parâmetros
int8_t cTimeZone; // Fuso horário

// WiFi
char cWifiSSID[35]; // Rede WiFi
char cWifiPW[35]; // Senha da Rede WiFi

// JSON
DynamicJsonDocument doc(2048); // Json onde será armazenado os dados de alarmes recebidos da API

// API
unsigned long lastTime = 0; // Ultima verificação
unsigned long timerDelay = 5000; // Seta o timer para 5 segundos (5000) - para 10 minutos utilizar 600000

// String genérica
String s;

// Schedule
String schedule;

// Flag que indica se o buzzer ou o led serão ativados
int flagBuzzer = 0;
int flagLed = -1; // Indica qual o led que deverá ser aceso
