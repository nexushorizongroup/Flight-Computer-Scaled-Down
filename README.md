# Model Rocket Flight Computer BOM & Facts

## Bill of Materials (BOM)

### Microcontroller
- ESP32-S3 (Dev board, with USB and SD support)

### Sensors & Modules
- **IMU (Primary):** ICM20948 (9-axis, SPI/I2C)
- **IMU (Backup):** GY-91 (MPU9250 + BMP280, I2C)
- **Barometer (Primary):** MS5611 (I2C/SPI)
- **Barometer (Backup):** BMP280 (via GY-91)
- **Power Monitor:** INA3221 (triple channel, I2C)
- **RTC:** DS3231 (I2C)
- **GPS:** M100-UBX (UART, custom protocol)
- **LoRa Radio:** LLCC68 (SPI)
- **SD Card Logger:** microSD (SPI)
- **Pyro Control:** LR7843 MOSFET driver
- **Vibration Sensing:** Uses backup IMU (GY-91) accelerometer, not a dedicated vibration sensor
- **I2C Multiplexer:** TCA9548A (optional, for sensor expansion)

### Other
- **Ring Buffer:** For telemetry and logging
- **Telemetry Frame:** Custom protocol
- **Calibration Storage:** EEPROM/Flash
- **Power Distribution:** Custom wiring

## Project Structure
- All sensor drivers are modular (.h/.cpp per sensor)
- Main logic in `main.cpp` with FreeRTOS tasks for sensor fusion, logging, radio, pyro
- Custom GPS protocol (not standard NMEA)

## Facts & Gotchyas
- **ESP32S3 Arduino:** Use Arduino core for ESP32S3, FreeRTOS tasks are supported natively
- **QueueHandle_t:** Used for inter-task communication; ensure proper queue sizes
- **IMU Calibration:** Calibration is essential for accurate flight data; check `calibration.h`
- **Barometer Altitude:** Uses sea-level pressure; adjust for launch site
- **SD Logging:** Logging is triggered by vibration/mach lock; check SD card speed
- **LoRa:** Custom telemetry frame; ensure radio config matches ground station
- **Pyro Safety:** LR7843 MOSFET is used for firing; test with dummy loads first
- **Vibration:** Events detected using backup IMU (GY-91) accelerometer RMS, not a separate vibration sensor/ADC
- **GPS:** Custom parser; verify firmware and baud rate
- **CRC:** Use ESP-IDF's CRC library for speed if needed
- **Pin Mapping:** See `hardware_pins.h` for all connections
- **Task Priorities/Core:** Tasks are pinned to cores for performance; adjust if needed
- **Backup Sensors:** GY-91 and BMP280 used as backup; logic blends data
- **Error Handling:** All sensor reads check `.valid` before use

## Setup & Usage
- Flash with PlatformIO or Arduino IDE (ESP32S3 target)
- Connect all sensors per `hardware_pins.h`
- Calibrate IMUs before flight
- Insert SD card, power up, and monitor serial/logs

## References
- [ESP32S3 Arduino Core](https://github.com/espressif/arduino-esp32)
- [FreeRTOS](https://www.freertos.org/)
- [ICM20948 Datasheet](https://invensense.tdk.com/wp-content/uploads/2018/08/DS-000189-ICM-20948-Datasheet.pdf)
- [MS5611 Datasheet](https://www.te.com/usa-en/product-CAT-BLPS0033.html)
- [LLCC68 LoRa Datasheet](https://www.semtech.com/products/communication/llcc68)

---

For any hardware or code gotchyas, see comments in `main.cpp` and sensor driver files.
