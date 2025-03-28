#include "webserver.h"

WebServerESP32::WebServerESP32(const char* ssidAP, const char* passwordAP, uint16_t port)
    : _ssidAP(ssidAP), _passwordAP(passwordAP), _port(port), _server(port) {
}

void WebServerESP32::begin() {
    Serial.begin(115200);
    
    // Configurar Wi-Fi
    setupWiFi();
    
    // Montar SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Error al montar SPIFFS");
        return;
    }
    
    // Configurar el servidor web
    setupServer();
    
    // Iniciar el servidor
    _server.begin();
}

void WebServerESP32::setupWiFi() {
    // Configurar el ESP32 en modo AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_ssidAP, _passwordAP);
    
    // Mostrar información del punto de acceso
    Serial.println();
    Serial.print("Punto de acceso iniciado. IP: ");
    Serial.println(WiFi.softAPIP());
}

void WebServerESP32::setupServer() { }

void WebServerESP32::on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest) {
    _server.on(uri, method, onRequest);
}

void WebServerESP32::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control) {
    _server.serveStatic(uri, fs, path, cache_control);
}

void WebServerESP32::setNotFoundHandler(ArRequestHandlerFunction handler) {
    _server.onNotFound(handler);
}

void WebServerESP32::setDefaultFile(const char* filename) {
    // Configurar el archivo por defecto para servir estáticos
    _server.serveStatic("/", SPIFFS, "/").setDefaultFile(filename);
}