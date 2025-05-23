#include "webserver.h"

WebServerESP32::WebServerESP32(const char* ssidAP, const char* passwordAP, uint16_t port)
    : _ssidAP(ssidAP), _passwordAP(passwordAP), _port(port), _server(port), _ws("/ws") { // Initialize _ws
}

void WebServerESP32::begin() {
    Serial.begin(115200);

    // Configure Wi-Fi
    Serial.println("[WEBSERVER] Configuring Wi-Fi Access Point...");
    setup_wifi();

    // Mount SPIFFS
    Serial.println("[WEBSERVER] Mounting SPIFFS filesystem...");
    if (!SPIFFS.begin(true)) {
        Serial.println("[WEBSERVER] ERROR: Failed to mount SPIFFS");
        return;
    }
    Serial.println("[WEBSERVER] SPIFFS mounted successfully");

    // Configure web server (including WebSocket)
    Serial.println("[WEBSERVER] Configuring web server...");
    setup_server();

    // Start server
    _server.begin();
    Serial.println("[WEBSERVER] HTTP and WebSocket server started successfully");
}

void WebServerESP32::setup_wifi() {
    // Configure static IP for AP mode
    IPAddress local_IP(10, 10, 10, 1); // Set your desired IP address
    IPAddress gateway(10, 10, 10, 1);  // Set your desired gateway address
    IPAddress subnet(255, 255, 255, 0); // Set your desired subnet mask
    WiFi.softAPConfig(local_IP, gateway, subnet);

    // Configure ESP32 in AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_ssidAP, _passwordAP);

    // Display access point information
    Serial.println();
    Serial.printf("[WEBSERVER] Access point started successfully\n");
    Serial.printf("[WEBSERVER] SSID: %s\n", _ssidAP);
    Serial.printf("[WEBSERVER] IP Address: %s\n", WiFi.softAPIP().toString().c_str());
}

void WebServerESP32::setup_server() {
    // Attach the WebSocket event handler if it's set
    if (_wsHandler) {
        _ws.onEvent(_wsHandler);
        Serial.println("[WEBSERVER] WebSocket handler attached");
    } else {
         Serial.println("[WEBSERVER] WARNING: WebSocket handler not set!");
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