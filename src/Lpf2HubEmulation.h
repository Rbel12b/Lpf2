#ifndef _LPF2_HUB_EMULATION_H_
#define _LPF2_HUB_EMULATION_H_

#include "config.h"
#include "Lpf2Const.h"
#include <NimBLEDevice.h>
#include <unordered_map>

class Lpf2Port;

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

    std::unordered_map<Lpf2PortNum, Lpf2Port*> attachedPorts;
    /**
     * @brief a map that contains if a port has a device attached,
     * used to determine when to send IO attached/detached messages
     */
    std::unordered_map<Lpf2PortNum, bool> connectedDevices;

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
    void handlePortModeInformationRequestMessage(std::vector<uint8_t> message);

    std::vector<uint8_t> packVersion(Lpf2Version version);
    Lpf2Version unPackVersion(std::vector<uint8_t> version);

    void checkPort(Lpf2Port* port);

public:
    Lpf2HubEmulation();
    Lpf2HubEmulation(std::string hubName, Lpf2HubType hubType);

    /**
     * @brief reset Hub properties, does not detach ports!
     */
    void reset();

    /**
     * Starts BLE advertising
     */
    void start();

    /**
     * @brief call this periodically to check if
     * attached ports have devices attached or not
     */
    void update();

    void setHubRssi(int8_t rssi);
    void setHubBatteryLevel(uint8_t batteryLevel);
    void setHubBatteryType(Lpf2BatteryType batteryType);
    void setHubName(std::string hubName);

    std::string getHubName();
    Lpf2BatteryType getBatteryType();

    void setHubFirmwareVersion(Lpf2Version version);
    void setHubHardwareVersion(Lpf2Version version);
    void setHubButton(bool pressed);

    /**
     * @brief Attach a port object, the class will take care of the devices atached/detached.
     * @param portNum The port number that will be assigned to the port.
     * @param port The port object, it's lifetime must exceed the Lpf2HubEmulation instance's lifetime, or must live till something calls reset().
     */
    void attachPort(Lpf2PortNum portNum, Lpf2Port* port);

    void writeValue(Lpf2MessageType messageType, std::vector<uint8_t> payload, bool notify = true);

    void onMessageReceived(std::vector<uint8_t> message);

    bool isConnected = false;
    bool isSubscribed = false;
    bool isPortInitialized = false;
    BLECharacteristic *pCharacteristic;
};

#endif // _LPF2_HUB_EMULATION_H_
