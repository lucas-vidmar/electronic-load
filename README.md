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

## Contributing

Pull requests for improvements or bug fixes are welcome. Please open an issue first to discuss ideas and coordinate development. 

## License

This project is distributed under a permissive license. See the repository for details.
