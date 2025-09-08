#include "BleBattery.h"

const int ADC_PIN = A0;
const float VOLTAGE_DIVIDER_RATIO = 2.0f;
const float ADC_REFERENCE_VOLTAGE = 3.3f;
const int ADC_RESOLUTION_BITS = 12;

const char *BASE_SUFFIX = "-6a47-4d2b-9f2c-5a6e7b8c9d0f"; // keep the '4' for version 4

const uint32_t UUID_PREFIX_BASE = 0xA1B21000; // pattern: 0xA1B2100n

BleBattery *ble;

void buildUuid(uint32_t shortId, const char *suffix, char *out, size_t outLen)
{
  // Format: 8-4-4-4-12 = 36 chars + null
  // shortId will fill the first 8 hex digits
  snprintf(out, outLen, "%08lX%s", (unsigned long)shortId, suffix);
}

float readBatteryVoltage()
{
#if defined(analogReadResolution)
  analogReadResolution(ADC_RESOLUTION_BITS);
#endif
  int maxADC = (1 << ADC_RESOLUTION_BITS) - 1;
  int raw = analogRead(ADC_PIN);
  float measured = ((float)raw / (float)maxADC) * ADC_REFERENCE_VOLTAGE;
  return measured * VOLTAGE_DIVIDER_RATIO;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 3000)
    ;

  uint32_t serviceShort = UUID_PREFIX_BASE + 0; // 0xA1B21000
  uint32_t charShort = UUID_PREFIX_BASE + 1;    // 0xA1B21001

  char serviceUuid[37];
  char charUuid[37];
  buildUuid(serviceShort, BASE_SUFFIX, serviceUuid, sizeof(serviceUuid));
  buildUuid(charShort, BASE_SUFFIX, charUuid, sizeof(charUuid));

  Serial.print("UUID prefix base: 0x");
  Serial.println((unsigned long)UUID_PREFIX_BASE, HEX);
  Serial.print("Service UUID: ");
  Serial.println(serviceUuid);
  Serial.print("Char UUID   : ");
  Serial.println(charUuid);

  ble = new BleBattery("Feather-Batt", serviceUuid, charUuid, 30000UL, 4000UL);
  if (!ble->begin())
  {
    Serial.println("BLE init failed");
    while (1)
      ;
  }

  float v = readBatteryVoltage();
  uint16_t mv = (uint16_t)round(v * 1000.0f);
  ble->updateVoltage(mv);

  Serial.println("Setup complete");
}

void loop()
{
  static unsigned long lastUpdate = 0;
  const unsigned long UPDATE_INTERVAL = 2000UL;
  unsigned long now = millis();

  if (now - lastUpdate >= UPDATE_INTERVAL)
  {
    float v = readBatteryVoltage();
    uint16_t mv = (uint16_t)round(v * 1000.0f);
    ble->updateVoltage(mv);
    Serial.print("Battery mV: ");
    Serial.println(mv);
    lastUpdate = now;
  }

  ble->poll();
  delay(50);
}
