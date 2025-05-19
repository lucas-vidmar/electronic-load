---
layout: default
title: Electronic Load System
---

## Electronic Load System Overview

### Purpose and Scope

The electronic load system is a versatile testing device designed to simulate various electrical loads for power supply testing, battery characterization, and other electronic component validation. It operates in multiple modes (Constant Current, Constant Voltage, Constant Resistance, Constant Power) and provides precision control through both a local touchscreen interface and a remote web interface.

---

## System Capabilities

| Feature                | Description                                                                                 |
| ---------------------- | ------------------------------------------------------------------------------------------- |
| **Operating Modes**    | Constant Current (CC), Constant Voltage (CV), Constant Resistance (CR), Constant Power (CW) |
| **User Interfaces**    | Local LCD touch interface and web-based remote interface                                    |
| **Precision Control**  | Digital control of load parameters with real-time feedback                                  |
| **Thermal Management** | PID-controlled fan with temperature monitoring                                              |
| **Data Monitoring**    | Real-time voltage, current, power, resistance, and temperature measurements                 |
| **Time Tracking**      | Energy consumption measurement and session uptime tracking                                  |

---

## High-Level System Architecture

<div class="mermaid">
---
config:
  theme: redux
  look: neo
  layout: elk
---
flowchart LR
 subgraph subGraph0["Interfaz de Usuario"]
    direction TB
        C1["Lectura Encoder"]
        C["Interfaz de Usuario"]
        C2["Actualización Display LCD"]
        C3["Comunicación WebServer"]
  end
 subgraph subGraph1["Control de Potencia"]
    direction TB
        D1["Lectura ADC"]
        D["Control de Potencia"]
        D2["Cálculo de parámetros"]
        D3["Configuración DAC"]
  end
 subgraph subGraph2["Monitoreo y Protección"]
    direction TB
        E1["Lectura Temperatura ADC"]
        E["Monitoreo y Protección"]
        E2["Control de ventiladores"]
        E3["Protección por sobrecarga"]
  end
    A["Inicio"] --> B["Inicialización del sistema"]
    B --> C & D & E
    C --> C1 & C2 & C3
    D --> D1 & D2 & D3
    E --> E1 & E2 & E3
</div>

* **ADC**: Measures voltage, current, and temperature
* **DAC**: Controls the load level
* **Analog Switches**: Configures operating mode and relays
* **Fan & PID Controller**: Manages cooling based on temperature feedback
* **RTC**: Tracks time for energy and uptime measurements

---

## Key Components & Data Flow

<div class="mermaid">
flowchart TD
 subgraph s1["Adquisición"]
        ADC["ADC: Tensión, Corriente, Temp"]
        RTC["RTC: Timestamp"]
        Enc["Encoder + Botón"]
  end
 subgraph Control["Control"]
        MCU["ESP32"]
        FSM["FSM"]
        PID["Control PID (Ventilador)"]
  end
 subgraph s2["Generación"]
        DAC["DAC"]
        SW["Switch Analógicos"]
  end
 subgraph s3["Visualización"]
        LCD["LCD Táctil"]
        WS["WebSocket"]
        LED["LED Integrado"]
  end
    ADC --> MCU
    RTC --> MCU
    Enc --> MCU
    MCU --> FSM & PID & LCD & WS & LED
    FSM --> DAC & SW
    PID --> FAN["Ventilador"]
</div>

---

## Main Loop Operations

The main control loop performs several critical operations during each iteration:

<div class="mermaid">
sequenceDiagram
    participant Main as "Main Loop"
    participant Meas as "Measurements"
    participant FSM as "FSM"
    participant Fan as "Fan Control"
    participant UI as "UI Updates"
    participant WS as "WebSocket"
    loop Cada iteración del bucle
        Main->>Meas: Leer tensión, corriente y temperatura
        Note right of Meas: Calcular potencia, resistencia
        Main->>FSM: Ejecutar FSM con mediciones actualizadas
        Note right of FSM: Ejecutar operaciones específicas del modo
        FSM-->>Main: Actualizar salidas (DAC, relés)
        Main->>Fan: Actualizar controlador PID
        Note right of Fan: Ajustar velocidad del ventilador según temperatura
        Main->>UI: Actualizar pantalla LCD
        Main->>WS: Enviar actualizaciones de estado
    end
</div>

---

## Operating Modes

* **Constant Current (CC)**: Maintains a user-defined current
* **Constant Voltage (CV)**: Maintains a user-defined voltage
* **Constant Resistance (CR)**: Simulates a fixed resistance
* **Constant Power (CW)**: Maintains constant power dissipation

Each mode uses specific DAC settings and switch configurations, managed by the FSM.

---

## User Interfaces

### LCD Touch Interface

* Built with LVGL
* Main menu for mode selection
* Mode-specific screens for parameter adjustment
* Real-time measurements & status indicators

### Web Interface

* ESP32-hosted web server
* WebSocket for real-time updates & JSON commands
* Full remote control of modes & settings

---

## Control & State Management

* **FSM States**: `MAIN_MENU`, `CC`, `CV`, `CR`, `CW`, `SETTINGS`
* **Transitions**: Encoder/button or WebSocket commands
* **Actions**: Configure hardware and update UI based on state

---

## Hardware Components

| Component                   | Class                      | Purpose                                |
| --------------------------- | -------------------------- | -------------------------------------- |
| Digital-to-Analog Converter | `DAC`                      | Controls load level via gate voltage   |
| Analog-to-Digital Converter | `ADC`                      | Measures voltage, current, temperature |
| Analog Switches             | `AnalogSws`                | Configures mode-select relays          |
| Cooling Fan                 | `Fan` & `PIDFanController` | Maintains safe temperature             |
| Real-Time Clock             | `RTC`                      | Tracks time for energy & uptime        |
| LCD Display                 | `LVGL_LCD`                 | Touchscreen UI                         |
| Rotary Encoder              | `Encoder`                  | User input for parameter selection     |

---

## Development Environment

* **PlatformIO** (ESP32 target)
* **Libraries**:

  * LVGL & TFT\_eSPI for LCD
  * AsyncTCP & ESPAsyncWebServer for web interface
  * ArduinoJson for JSON handling
* **Version Control**: Git (src/main.cpp, etc.)

---

## Conclusion

The electronic load system offers a modular and extensible platform for testing power supplies, batteries, and electronic components. With four operating modes, dual user interfaces, real-time feedback, and robust thermal management, it serves both educational and professional applications as a versatile test instrument.
