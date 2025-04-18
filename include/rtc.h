#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "i2c.h"

#define MCP7941X_ADDRESS 0x6F // Dirección I2C del MCP7941X

// Estructura para almacenar la fecha y hora
struct DateTime {
    uint8_t seconds;    // Segundos (0-59)
    uint8_t minutes;    // Minutos (0-59)
    uint8_t hours;      // Horas (0-23, formato 24h)
    uint8_t dayOfWeek;  // Día de la semana (1-7)
    uint8_t date;       // Día del mes (1-31)
    uint8_t month;      // Mes (1-12)
    uint8_t year;       // Año (por ejemplo, 25 para 2025)
};

class RTC {
public:
    RTC();

    // Inicializa el bus I2C y verifica la presencia del dispositivo.
    void init(I2C* i2cPointer);

    // Establece la fecha y hora en el dispositivo.
    // Se activa el oscilador con el bit ST (bit 7 del registro de segundos).
    void set_time(const DateTime &dt);

    // Lee la fecha y hora desde el dispositivo.
    DateTime get_time();
    
    // Obtiene el tiempo actual en milisegundos desde el 1 de enero de 2000
    uint64_t get_timestamp_ms();
    
    // Calcula el tiempo transcurrido en milisegundos entre dos timestamps
    static uint64_t elapsed_ms(uint64_t start_time, uint64_t end_time);

private:
    // Funciones para conversiones entre formato decimal y BCD.
    static uint8_t dec_to_bcd(uint8_t val);
    static uint8_t bcd_to_dec(uint8_t val);
    
    // Convierte una estructura DateTime a milisegundos desde el 1 de enero de 2000
    static uint64_t datetime_to_ms(const DateTime &dt);

    /**
     * @brief Pointer to an I2C instance used for communication.
     */
    I2C* i2c;
};