#if defined(ESP32)

#ifndef Lpf2HubEmulation_h
#define Lpf2HubEmulation_h

#include "config.h"
#include <NimBLEDevice.h>
#include "Lpf2Hub.h"

typedef void (*WritePortCallback)(byte port, byte value);

class Lpf2HubEmulation
{
private:
    // Notification callbacks if values are written to the characteristic
    BLEUUID _bleUuid;
    BLEUUID _charachteristicUuid;
    BLEAddress *_pServerAddress;
    BLEServer *_pServer;
    BLEService *_pService;
    BLEAddress *_hubAddress = nullptr;
    BLEAdvertising *_pAdvertising;

    Lpf2HubType _hubType = Lpf2HubType::UNKNOWNHUB;

    // List of connected devices
    Device connectedDevices[13];
    int numberOfConnectedDevices = 0;

    bool updateHubPropertyEnabled[(unsigned int)Lpf2HubPropertyReference::END] = {false};
    std::vector<uint8_t> hubProperty[(unsigned int)Lpf2HubPropertyReference::END];
    void updateHubProperty(Lpf2HubPropertyReference propRef);
    void sendHubPropertyUpdate(Lpf2HubPropertyReference propRef);
    void resetHubProperty(Lpf2HubPropertyReference propRef);
    void handleHubPropertyMessage(std::vector<uint8_t> message);

    bool hubAlertEnabled[(unsigned int)Lpf2HubAlertType::END] = {false};
    bool hubAlert[(unsigned int)Lpf2HubAlertType::END] = {false};

public:
    void updateHubAlert(Lpf2HubAlertType alert, bool on);

private:
    void sendHubAlertUpdate(Lpf2HubAlertType alert);
    void resetHubAlerts();
    void handleHubAlertsMessage(std::vector<uint8_t> message);

    void handlePortInformationRequestMessage(std::vector<uint8_t> message);

    std::vector<uint8_t> packVersion(Lpf2Version version);
    Lpf2Version unPackVersion(std::vector<uint8_t> version);

public:
    Lpf2HubEmulation();
    Lpf2HubEmulation(std::string hubName, Lpf2HubType hubType);
    void reset();
    void start();
    void setWritePortCallback(WritePortCallback callback);
    void setHubRssi(int8_t rssi);
    void setHubBatteryLevel(uint8_t batteryLevel);
    void setHubBatteryType(Lpf2BatteryType batteryType);
    void setHubName(std::string hubName);

    std::string getHubName();
    Lpf2BatteryType getBatteryType();

    void setHubFirmwareVersion(Lpf2Version version);
    void setHubHardwareVersion(Lpf2Version version);
    void setHubButton(bool pressed);

    void attachDevice(byte port, Lpf2DeviceType deviceType);
    void detachDevice(byte port);
    byte getDeviceTypeForPort(byte port);

    void writeValue(Lpf2MessageType messageType, std::vector<uint8_t> payload, bool notify = true);
    std::vector<uint8_t> getPortModeInformationRequestPayload(Lpf2DeviceType deviceType, byte port, byte mode, byte modeInformationType);
    std::vector<uint8_t> getPortInformationPayload(Lpf2DeviceType deviceType, byte port, byte informationType);

    void onMessageReceived(std::vector<uint8_t> message);

    bool isConnected = false;
    bool isSubscribed = false;
    bool isPortInitialized = false;
    BLECharacteristic *pCharacteristic;
    WritePortCallback writePortCallback = nullptr;
};

#endif // Lpf2HubEmulation_h

#endif // ESP32
