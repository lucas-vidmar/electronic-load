#include "webserver.h"

WebServerESP32::WebServerESP32(const char* ssidAP, const char* passwordAP, uint16_t port)
    : _ssidAP(ssidAP), _passwordAP(passwordAP), _port(port), _server(port) {
}

void WebServerESP32::begin() {
    Serial.begin(115200);
    
    // Configure Wi-Fi
    setup_wifi();
    
    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Error mounting SPIFFS");
        return;
    }
    
    // Configure web server
    setup_server();
    
    // Start server
    _server.begin();
}

void WebServerESP32::setup_wifi() {
    // Configure ESP32 in AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_ssidAP, _passwordAP);
    
    // Display access point information
    Serial.println();
    Serial.print("Access point started. IP: ");
    Serial.println(WiFi.softAPIP());
}

void WebServerESP32::setup_server() { }

void WebServerESP32::on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest) {
    _server.on(uri, method, onRequest);
}

void WebServerESP32::serve_static(const char* uri, fs::FS& fs, const char* path, const char* cache_control) {
    _server.serveStatic(uri, fs, path, cache_control);
}

void WebServerESP32::set_not_found_handler(ArRequestHandlerFunction handler) {
    _server.onNotFound(handler);
}

void WebServerESP32::set_default_file(const char* filename) {
    // Configure default file for serving static files
    _server.serveStatic("/", SPIFFS, "/").setDefaultFile(filename);
}