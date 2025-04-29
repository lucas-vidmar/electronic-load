# Electronic Load Project

This PlatformIO-based project implements a configurable electronic load on an ESP32-based board. It uses:

- LVGL for a touch-friendly user interface
- TFT_eSPI for display interaction
- ESPAsyncWebServer for handling HTTP requests
- Various Arduino libraries for WiFi, SPIFFS, and other peripherals

## Getting Started

1. Clone or download this repository into your PlatformIO workspace.
2. Open the project in your preferred PlatformIO-supported IDE.
3. Adjust settings in `platformio.ini` as needed for your board.
4. Build and upload the firmware to the ESP32 board.
5. Access the web interface over Wi-Fi or view the UI on a connected display.

## Features

- UI rendering with LVGL
- Configurable load parameters via on-screen or web-based controls
- Real-time monitoring of input parameters
- Expandable code structure for sensors and hardware drivers

## Registro de Versiones del Proyecto

### Versión 0.1 (Junio - Julio 2024): Exploración Inicial con Arduino

- Inicio del proyecto utilizando el entorno Arduino.
- Pruebas básicas de Entrada/Salida de Propósito General (GPIO) para verificar el hardware fundamental (LEDs, lectura de pines).
- Configuración inicial de la comunicación serial para depuración.

### Versión 0.2 (Julio - Agosto 2024): Migración a ESP-IDF y Pantalla Básica

- Migración del código base de Arduino a Espressif IDF (ESP-IDF) utilizando C.
- Reimplementación de las pruebas GPIO en el entorno ESP-IDF.
- Integración de una librería de pantalla básica (TFT eSPI) para mostrar información simple (texto, formas básicas).
- Primeras pruebas de visualización en la pantalla LCD.

### Versión 0.3 (Agosto - Septiembre 2024): Intento de Integración con LVGL en ESP-IDF

- Intento de integrar la librería gráfica LVGL directamente dentro del entorno ESP-IDF.
- Configuración inicial de los drivers de pantalla y táctil (si aplica) para LVGL.
- Dificultades encontradas al configurar y hacer funcionar LVGL de manera estable con ESP-IDF.
- Pruebas iniciales con widgets básicos de LVGL con resultados inconsistentes.

### Versión 1.0 (Octubre - Noviembre 2024): Establecimiento en PlatformIO y Hardware Básico

- **Hito:** Migración exitosa del proyecto a PlatformIO buscando un mejor manejo de dependencias y entorno de desarrollo.
- Configuración inicial del hardware en PlatformIO: LED, interruptores analógicos, I2C.
- Implementación y correcciones iniciales para DAC y ADC dentro de PlatformIO.

### Versión 1.1 (Noviembre 2024): Integración de UI Básica con LVGL (en PlatformIO)

- Integración exitosa de la librería gráfica LVGL (ahora dentro de PlatformIO).
- Desarrollo de la estructura básica del menú y la UI.
- Implementación inicial (no completamente funcional) de la UI para el modo CC.

### Versión 2.0 (Febrero - Principios de Marzo 2025): Modo CC Funcional y Refactorización Mayor

- **Hito Funcional:** Implementación completa y funcional del modo de Corriente Constante (CC).
- Refactorización significativa de la UI (pantalla, botones, menús, colores, fuentes).
- Refactorización del manejo del encoder y la máquina de estados (FSM).
- Optimización de memoria para la UI.
- Configuración de dígitos en la UI disponible.

### Versión 2.1 (Marzo 2025): Modos CV, CR, CW y Pruebas

- **Hito Funcional:** Adición de los modos funcionales de Voltaje Constante (CV), Resistencia Constante (CR) y Potencia Constante (CW).
- Pruebas de laboratorio realizadas y ajustes aplicados (límites, modo CC).
- Refactorización de la lectura del DUT.

### Versión 3.0 (Finales de Marzo - Principios de Abril 2025): Servidor Web y UI Web Inicial

- **Hito Funcional:** Adición de un servidor web funcional.
- Separación del código HTML y JavaScript.
- Desarrollo de la interfaz web (frontend) inicial.
- Implementación básica de la comunicación vía WebSockets para la UI web.
- Documentación y limpieza.

### Versión 3.1 (Abril 2025): Control de Ventiladores y RTC

- **Hito Funcional:** Implementación del control de ventiladores (fans).
- **Hito Funcional:** Adición de un Reloj de Tiempo Real (RTC).
- Ajustes en pines de hardware y parámetros de ventiladores.

### Versión 3.2 (Abril 2025): Mejoras Web UI y Correcciones Finales

- Integración de información de ventiladores en la interfaz web.
- Unificación de la recolección de datos para la web y mejoras en WebSockets.
- Corrección de errores generales y específicos (encoder, cálculo DAC, actualización UI web).
- Mejoras en el estilo y diseño de la interfaz web (CSS, HTML).


## Contributing

Pull requests for improvements or bug fixes are welcome. Please open an issue first to discuss ideas and coordinate development. 

## License

This project is distributed under a permissive license. See the repository for details.
