#include "webserver.h"

WebServerESP32::WebServerESP32(const char* ssidAP, const char* passwordAP, uint16_t port)
    : _ssidAP(ssidAP), _passwordAP(passwordAP), _port(port), _server(port), _ws("/ws") { // Initialize _ws
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

    // Configure web server (including WebSocket)
    setup_server();

    // Start server
    _server.begin();
    Serial.println("HTTP and WebSocket server started");
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

void WebServerESP32::setup_server() {
    // Attach the WebSocket event handler if it's set
    if (_wsHandler) {
        _ws.onEvent(_wsHandler);
    } else {
         Serial.println("WebSocket handler not set!");
    }
    _server.addHandler(&_ws); // Add WebSocket handler to the server
}

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

void WebServerESP32::attachWsHandler(WsEventHandler handler) {
    _wsHandler = handler; // Store the handler
    // If the server is already running, attach immediately. Otherwise, setup_server will handle it.
    // Note: Typically, attachWsHandler should be called before begin().
    // If called after begin(), this ensures the handler is attached immediately.
    // Calling onEvent multiple times (here and in setup_server) is generally safe.
    _ws.onEvent(_wsHandler);
}

void WebServerESP32::notifyClients(const String& message) {
    _ws.textAll(message);
}

void WebServerESP32::cleanupClients() {
    _ws.cleanupClients();
}