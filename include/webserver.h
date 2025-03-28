/**
 * @file webserver.h
 * @brief Archivo de encabezado para la clase WebServerESP32.
 *
 * Este archivo contiene la declaración de la clase WebServerESP32, que proporciona una interfaz
 * para configurar y controlar un servidor web en un ESP32.
 *
 */
#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

class WebServerESP32 {
public:
    /**
     * @brief Constructor de la clase WebServerESP32.
     * 
     * Este constructor inicializa el objeto WebServerESP32 con el SSID y la contraseña 
     * dados para el punto de acceso (AP) y un número de puerto opcional.
     * 
     * @param ssidAP El SSID del punto de acceso.
     * @param passwordAP La contraseña del punto de acceso.
     * @param port El número de puerto para el servidor web (por defecto es 80).
     */
    WebServerESP32(const char* ssidAP, const char* passwordAP, uint16_t port = 80);

    /**
     * @brief Inicializa y arranca el servidor web.
     */
    void begin();

    /**
     * @brief Registra una función manejadora que será llamada cuando se reciba una solicitud con el URI y método especificados.
     * 
     * @param uri El URI a coincidir para la solicitud.
     * @param method El método HTTP a coincidir para la solicitud.
     * @param onRequest La función que será llamada cuando se reciba la solicitud.
     */
    void on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);

    /**
     * @brief Sirve un archivo estático desde el sistema de archivos.
     * 
     * Esta función se utiliza para servir un archivo estático ubicado en la ruta especificada
     * dentro del sistema de archivos. Mapea el URI dado al archivo y opcionalmente 
     * establece las cabeceras de control de caché.
     * 
     * @param uri El URI a mapear al archivo estático.
     * @param fs El objeto del sistema de archivos a utilizar para acceder al archivo.
     * @param path La ruta al archivo estático dentro del sistema de archivos.
     * @param cache_control Cabeceras opcionales de control de caché a enviar con la respuesta.
     */
    void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control = NULL);

    /**
     * @brief Establece la función manejadora que será llamada cuando no se encuentre un recurso solicitado.
     * 
     * Esta función permite especificar un manejador personalizado que será invocado
     * siempre que un cliente solicite un recurso que no existe en el servidor.
     * 
     * @param handler La función que será llamada cuando ocurra un error 404 Not Found.
     */
    void setNotFoundHandler(ArRequestHandlerFunction handler);

    /**
     * @brief Establece el archivo predeterminado que será servido por el servidor web.
     * 
     * Esta función permite especificar el archivo predeterminado que será servido
     * cuando un cliente solicite el directorio raíz o un directorio sin especificar
     * un nombre de archivo.
     * 
     * @param filename El nombre del archivo a establecer como predeterminado. Esto debe
     *                 ser una cadena terminada en nulo que represente la ruta del archivo.
     */
    void setDefaultFile(const char* filename);
    
private:
    const char* _ssidAP;
    const char* _passwordAP;
    uint16_t _port;
    AsyncWebServer _server;
    
    void setupWiFi();
    void setupServer();
};