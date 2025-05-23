#include "rtc.h"

RTC::RTC() : i2c(nullptr) {}

void RTC::init(I2C* i2cPointer) {
    i2c = i2cPointer;
    if (i2c != nullptr) {
        Serial.println("[RTC] Initialized with I2C interface");
    } else {
        Serial.println("[RTC] Error: I2C pointer is null");
    }
}

void RTC::set_time(const DateTime &dt) {
    uint8_t data[8]; // 1 byte for register address + 7 bytes for date/time data
    
    // First byte is register address 0x00 (seconds register)
    data[0] = 0x00;
    
    // Convert datetime to BCD and put in data buffer
    data[1] = (dec_to_bcd(dt.seconds) & 0x7F) | 0x80; // Set bit 7 (ST bit) to enable oscillator
    data[2] = dec_to_bcd(dt.minutes);
    data[3] = dec_to_bcd(dt.hours); // Assuming 24-hour format
    data[4] = dec_to_bcd(dt.dayOfWeek);
    data[5] = dec_to_bcd(dt.date);
    data[6] = dec_to_bcd(dt.month);
    data[7] = dec_to_bcd(dt.year);
    
    // Write data to RTC
    if (i2c != nullptr) {
        i2c->write(MCP7941X_ADDRESS, data, 8);
        Serial.printf("[RTC] Time set: %02d:%02d:%02d %02d/%02d/%04d\n", 
                     dt.hours, dt.minutes, dt.seconds, dt.date, dt.month, 2000 + dt.year);
    } else {
        Serial.println("[RTC] Error: Cannot set time, I2C not initialized");
    }
}

DateTime RTC::get_time() {
    DateTime dt;
    uint8_t registerAddr = 0x00; // Start address (seconds register)
    uint8_t data[7]; // Buffer to store 7 bytes of date/time data
    
    if (i2c != nullptr) {
        // First write the register address we want to read from
        i2c->write(MCP7941X_ADDRESS, &registerAddr, 1);
        
        // Then read 7 bytes of data
        i2c->read(MCP7941X_ADDRESS, data, 7);
        
        // Convert read data to DateTime structure
        dt.seconds = bcd_to_dec(data[0] & 0x7F); // Mask out ST bit
        dt.minutes = bcd_to_dec(data[1]);
        dt.hours = bcd_to_dec(data[2] & 0x3F); // Mask out 24-hour format bits if present
        dt.dayOfWeek = bcd_to_dec(data[3] & 0x07); // Mask out any control bits
        dt.date = bcd_to_dec(data[4]);
        dt.month = bcd_to_dec(data[5] & 0x1F); // Mask out any control bits
        dt.year = bcd_to_dec(data[6]);
    } else {
        Serial.println("[RTC] Error: Cannot read time, I2C not initialized");
        // Return zero DateTime if I2C not available
        dt.seconds = dt.minutes = dt.hours = dt.date = dt.month = dt.year = dt.dayOfWeek = 0;
    }
    
    return dt;
}

uint64_t RTC::get_timestamp_ms() {
    DateTime dt = get_time();
    return datetime_to_ms(dt);
}

uint64_t RTC::elapsed_ms(uint64_t start_time, uint64_t end_time) {
    if (end_time >= start_time) {
        return end_time - start_time;
    } else {
        // Handle case where end_time is before start_time
        // (for example, if RTC was reset or set back)
        return 0;
    }
}

uint64_t RTC::datetime_to_ms(const DateTime &dt) {
    // Número de días en cada mes (sin considerar años bisiestos)
    static const uint16_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Calcular años completos desde 2000
    uint16_t year = 2000 + dt.year;
    uint64_t timestamp = 0;
    
    // Agregar días de años completos (considerando años bisiestos)
    for (uint16_t y = 2000; y < year; y++) {
        timestamp += (365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0) ? 1 : 0)) * 24ULL * 60ULL * 60ULL * 1000ULL;
    }
    
    // Agregar días de meses completos en el año actual
    for (uint8_t m = 1; m < dt.month; m++) {
        uint16_t days = days_in_month[m];
        // Añadir un día en febrero si el año actual es bisiesto
        if (m == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
            days++;
        }
        timestamp += days * 24ULL * 60ULL * 60ULL * 1000ULL;
    }
    
    // Agregar días del mes actual
    timestamp += (dt.date - 1) * 24ULL * 60ULL * 60ULL * 1000ULL;
    
    // Agregar horas, minutos y segundos
    timestamp += dt.hours * 60ULL * 60ULL * 1000ULL;
    timestamp += dt.minutes * 60ULL * 1000ULL;
    timestamp += dt.seconds * 1000ULL;
    
    return timestamp;
}

uint8_t RTC::dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

uint8_t RTC::bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}