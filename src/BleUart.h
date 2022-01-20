// https://github.com/STEMpedia/DabbleESP32/blob/master/src/esp32BLEUtilities.h
#ifndef BLE_UART_H
#define BLE_UART_H
#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <stdint.h>

// UART service UUID
#define UUID_Service "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UUID_Transmit "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define UUID_Receive "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

class BleUart {
 public:
  void begin(std::string a);
  void write(uint8_t a);
  void write(std::string x);
  void write(int a);
  void write(float a);
  uint8_t available();
  uint8_t read();
  void stop();
};
extern bool BLE_status;  // extern BLE_status
extern BleUart bleUart;
#endif
