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

| Version & Date | Summary | Details |
|---|---|---|
| **0.1**<br>(June - July 2024) | Initial Arduino Exploration | - Project started using the Arduino environment.<br>- Basic General Purpose Input/Output (GPIO) tests to verify fundamental hardware (LEDs, pin reading).<br>- Initial setup of serial communication for debugging. |
| **0.2**<br>(July - August 2024) | Migration to ESP-IDF and Basic Display | - Migration of the base code from Arduino to Espressif IDF (ESP-IDF) using C.<br>- Reimplementation of GPIO tests in the ESP-IDF environment.<br>- Integration of a basic display library (TFT eSPI) to show simple information (text, basic shapes).<br>- First display tests on the LCD screen. |
| **0.3**<br>(August - September 2024) | Attempted LVGL Integration in ESP-IDF | - Attempt to integrate the LVGL graphics library directly within the ESP-IDF environment.<br>- Initial setup of display and touch drivers (if applicable) for LVGL.<br>- Difficulties encountered in configuring and running LVGL stably with ESP-IDF.<br>- Initial tests with basic LVGL widgets with inconsistent results. |
| **1.0**<br>(October - November 2024) | PlatformIO Establishment and Basic Hardware | - Successful migration of the project to PlatformIO for better dependency and development environment management.<br>- Initial hardware setup in PlatformIO: LED, analog switches, I2C.<br>- Initial implementation and fixes for DAC and ADC within PlatformIO. |
| **1.1**<br>(November 2024) | Basic UI Integration with LVGL (in PlatformIO) | - Successful integration of the LVGL graphics library (now within PlatformIO).<br>- Development of the basic menu and UI structure.<br>- Initial (not fully functional) implementation of the UI for CC mode. |
| **2.0**<br>(February - Early March 2025) | Functional CC Mode and Major Refactoring | - Complete and functional implementation of Constant Current (CC) mode.<br>- Significant UI refactoring (screen, buttons, menus, colors, fonts).<br>- Refactoring of encoder handling and the state machine (FSM).<br>- Memory optimization for the UI.<br>- Digit configuration available in the UI. |
| **2.1**<br>(March 2025) | CV, CR, CW Modes and Testing | - Addition of functional Constant Voltage (CV), Constant Resistance (CR), and Constant Power (CW) modes.<br>- Laboratory tests performed and adjustments applied (limits, CC mode).<br>- Refactoring of DUT reading. |
| **3.0**<br>(Late March - Early April 2025) | Web Server and Initial Web UI | - Addition of a functional web server.<br>- Separation of HTML and JavaScript code.<br>- Development of the initial web interface (frontend).<br>- Basic implementation of WebSocket communication for the web UI.<br>- Documentation and cleanup. |
| **3.1**<br>(April 2025) | Fan Control and RTC | - Implementation of fan control.<br>- Addition of a Real-Time Clock (RTC).<br>- Adjustments to hardware pins and fan parameters. |
| **3.2**<br>(April 2025) | Web UI Improvements and Final Fixes | - Integration of fan information in the web interface.<br>- Unification of data collection for the web and WebSocket improvements.<br>- General and specific bug fixes (encoder, DAC calculation, web UI update).<br>- Improvements in the style and design of the web interface (CSS, HTML). |
| **3.3**<br>(May 2025) | Settings, UI, and PID Improvements | - Tuning and optimization of the fan PID control.<br>- Critical bug fixes in the settings menu (crashes and parameter synchronization).<br>- Addition of a settings menu accessible from the UI.<br>- Cleanup and restructuring of the LVGL UI code (new layout, status indicators, minor visual adjustments).<br>- Documentation improvements and updates. |

## Contributing

Pull requests for improvements or bug fixes are welcome. Please open an issue first to discuss ideas and coordinate development.

## Documentation

- [Index](docs/index.md)
- [Hardware Documentation](docs/hardware.md)
- [Software Documentation](docs/software.md)