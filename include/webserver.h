/**
 * @file webserver.h
 * @brief Header file for the WebServerESP32 class.
 * 
 * This file contains the declaration of the WebServerESP32 class, which provides an interface
 * for configuring and controlling a web server on an ESP32.
 * 
 * @date 2023
 */
#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

class WebServerESP32 {
public:
    /**
     * @brief Constructor for the WebServerESP32 class.
     * 
     * Initializes the WebServerESP32 object with the given SSID and password 
     * for the access point (AP) and an optional port number.
     * 
     * @param ssidAP The SSID of the access point.
     * @param passwordAP The password for the access point.
     * @param port The port number for the web server (default is 80).
     */
    WebServerESP32(const char* ssidAP, const char* passwordAP, uint16_t port = 80);

    /**
     * @brief Initializes and starts the web server.
     */
    void begin();

    /**
     * @brief Registers a handler function that will be called when a request with the specified URI and method is received.
     * 
     * @param uri The URI to match for the request.
     * @param method The HTTP method to match for the request.
     * @param onRequest The function that will be called when the request is received.
     */
    void on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);

    /**
     * @brief Serves a static file from the file system.
     * 
     * This function is used to serve a static file located at the specified path
     * within the file system. It maps the given URI to the file and optionally 
     * sets cache control headers.
     * 
     * @param uri The URI to map to the static file.
     * @param fs The file system object to use for accessing the file.
     * @param path The path to the static file within the file system.
     * @param cache_control Optional cache control headers to send with the response.
     */
    void serve_static(const char* uri, fs::FS& fs, const char* path, const char* cache_control = NULL);

    /**
     * @brief Sets the handler function that will be called when a requested resource is not found.
     * 
     * This function allows specifying a custom handler that will be invoked
     * whenever a client requests a resource that doesn't exist on the server.
     * 
     * @param handler The function that will be called when a 404 Not Found error occurs.
     */
    void set_not_found_handler(ArRequestHandlerFunction handler);

    /**
     * @brief Sets the default file to be served by the web server.
     * 
     * This function allows specifying the default file that will be served
     * when a client requests the root directory or a directory without specifying
     * a filename.
     * 
     * @param filename The name of the file to set as default. This should be
     *                 a null-terminated string representing the file path.
     */
    void set_default_file(const char* filename);
    
private:
    const char* _ssidAP;
    const char* _passwordAP;
    uint16_t _port;
    AsyncWebServer _server;
    
    void setup_wifi();
    void setup_server();
};