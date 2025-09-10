#ifndef BLE_BATTERY_H
#define BLE_BATTERY_H

#include <Arduino.h>
#include <bluefruit.h>

class BleBattery {
public:
  BleBattery(const char* localName, const char* serviceUuid, const char* charUuid,
             unsigned long advPeriod = 10000UL, unsigned long advDuration = 3000UL); // default: advertise ~3s every 10s
  ~BleBattery();

  bool begin();

  void setAdvertiseTiming(unsigned long period, unsigned long duration);

  void updateVoltage(uint16_t millivolts);

  void poll();
  void handleAdvertising();

private:
  const char* _localName;
  char _nameBuf[32]; // dynamic name with MAC suffix
  unsigned long _advPeriod;
  unsigned long _advDuration;
  unsigned long _lastAdvertiseCycle;
  bool _advertisingActive;
  unsigned long _advertiseEnd;

  BLEService* _service;
  BLECharacteristic* _voltageChar;
};

#endif // BLE_BATTERY_H
