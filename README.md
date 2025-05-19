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

## Project Version History

### Version 0.1 (June - July 2024): Initial Arduino Exploration

- Project started using the Arduino environment.
- Basic General Purpose Input/Output (GPIO) tests to verify fundamental hardware (LEDs, pin reading).
- Initial setup of serial communication for debugging.

### Version 0.2 (July - August 2024): Migration to ESP-IDF and Basic Display

- Migration of the base code from Arduino to Espressif IDF (ESP-IDF) using C.
- Reimplementation of GPIO tests in the ESP-IDF environment.
- Integration of a basic display library (TFT eSPI) to show simple information (text, basic shapes).
- First display tests on the LCD screen.

### Version 0.3 (August - September 2024): Attempted LVGL Integration in ESP-IDF

- Attempt to integrate the LVGL graphics library directly within the ESP-IDF environment.
- Initial setup of display and touch drivers (if applicable) for LVGL.
- Difficulties encountered in configuring and running LVGL stably with ESP-IDF.
- Initial tests with basic LVGL widgets with inconsistent results.

### Version 1.0 (October - November 2024): PlatformIO Establishment and Basic Hardware

- Successful migration of the project to PlatformIO for better dependency and development environment management.
- Initial hardware setup in PlatformIO: LED, analog switches, I2C.
- Initial implementation and fixes for DAC and ADC within PlatformIO.

### Version 1.1 (November 2024): Basic UI Integration with LVGL (in PlatformIO)

- Successful integration of the LVGL graphics library (now within PlatformIO).
- Development of the basic menu and UI structure.
- Initial (not fully functional) implementation of the UI for CC mode.

### Version 2.0 (February - Early March 2025): Functional CC Mode and Major Refactoring

- Complete and functional implementation of Constant Current (CC) mode.
- Significant UI refactoring (screen, buttons, menus, colors, fonts).
- Refactoring of encoder handling and the state machine (FSM).
- Memory optimization for the UI.
- Digit configuration available in the UI.

### Version 2.1 (March 2025): CV, CR, CW Modes and Testing

- Addition of functional Constant Voltage (CV), Constant Resistance (CR), and Constant Power (CW) modes.
- Laboratory tests performed and adjustments applied (limits, CC mode).
- Refactoring of DUT reading.

### Version 3.0 (Late March - Early April 2025): Web Server and Initial Web UI

- Addition of a functional web server.
- Separation of HTML and JavaScript code.
- Development of the initial web interface (frontend).
- Basic implementation of WebSocket communication for the web UI.
- Documentation and cleanup.

### Version 3.1 (April 2025): Fan Control and RTC

- Implementation of fan control.
- Addition of a Real-Time Clock (RTC).
- Adjustments to hardware pins and fan parameters.

### Version 3.2 (April 2025): Web UI Improvements and Final Fixes

- Integration of fan information in the web interface.
- Unification of data collection for the web and WebSocket improvements.
- General and specific bug fixes (encoder, DAC calculation, web UI update).
- Improvements in the style and design of the web interface (CSS, HTML).

### Version 3.3 (May 2025): Settings, UI, and PID Improvements

- Tuning and optimization of the fan PID control.
- Critical bug fixes in the settings menu (crashes and parameter synchronization).
- Addition of a settings menu accessible from the UI.
- Cleanup and restructuring of the LVGL UI code (new layout, status indicators, minor visual adjustments).
- Documentation improvements and updates.

## Contributing

Pull requests for improvements or bug fixes are welcome. Please open an issue first to discuss ideas and coordinate development.

## License

This project is distributed under a permissive license. See the repository for details.
