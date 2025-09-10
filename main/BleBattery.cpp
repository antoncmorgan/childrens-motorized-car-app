#include "BleBattery.h"

BleBattery::BleBattery(const char *localName, const char *serviceUuid, const char *charUuid,
                       unsigned long advPeriod, unsigned long advDuration)
    : _localName(localName), _advPeriod(advPeriod), _advDuration(advDuration),
      _lastAdvertiseCycle(0), _advertisingActive(false), _advertiseEnd(0)
{
    _service = new BLEService(serviceUuid);
    _voltageChar = new BLECharacteristic(charUuid, BLERead | BLENotify, 2);
}

BleBattery::~BleBattery()
{
    delete _service;
    delete _voltageChar;
}

bool BleBattery::begin()
{
    if (!Bluefruit.begin())
        return false;
    // Build dynamic name with last two bytes of MAC address.
    // getAddr() with no params returns ble_gap_addr_t; addr[0] is LSB.
    ble_gap_addr_t mac = Bluefruit.getAddr();
    uint8_t b0 = mac.addr[1];
    uint8_t b1 = mac.addr[0];
    snprintf(_nameBuf, sizeof(_nameBuf), "%s %02X:%02X", _localName, b0, b1);
    _nameBuf[sizeof(_nameBuf) - 1] = '\0';
    Bluefruit.setName(_nameBuf);

    _service->begin();
    _voltageChar->begin();

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addName();
    Bluefruit.Advertising.addService(*_service);

    Bluefruit.Advertising.setInterval(32, 160); // in ms (fast, slow)
    Bluefruit.Advertising.setFastTimeout(10);   // number of seconds in fast mode

    return true;
}

void BleBattery::setAdvertiseTiming(unsigned long period, unsigned long duration)
{
    _advPeriod = period;
    _advDuration = duration;
}

void BleBattery::updateVoltage(uint16_t millivolts)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(millivolts & 0xFF);
    buf[1] = (uint8_t)((millivolts >> 8) & 0xFF);
    _voltageChar->write(buf, 2);
    _voltageChar->notify(buf, 2);
}

void BleBattery::handleAdvertising()
{
    unsigned long now = millis();
    if (_advertisingActive)
    {
        if (now >= _advertiseEnd)
        {
            Bluefruit.Advertising.stop();
            _advertisingActive = false;
            _lastAdvertiseCycle = now;
        }
    }
    else
    {
        if (now - _lastAdvertiseCycle >= _advPeriod || _lastAdvertiseCycle == 0)
        {
            Bluefruit.Advertising.clearData();
            Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
            Bluefruit.Advertising.addTxPower();
            Bluefruit.Advertising.addName();
            Bluefruit.Advertising.addService(*_service);
            Bluefruit.Advertising.start();
            _advertisingActive = true;
            _advertiseEnd = now + _advDuration;
        }
    }
}

void BleBattery::poll()
{
    handleAdvertising();
}
