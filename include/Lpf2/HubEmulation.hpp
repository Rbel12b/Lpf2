#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/LWPConst.hpp"
#include <NimBLEDevice.h>
#include <unordered_map>
namespace Lpf2
{
    class Port;
    namespace Virtual
    {
        class Port;
        class GenericDevice;
    }; // namespace Virtual

    class HubEmulation
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

        HubType m_hubType = HubType::UNKNOWNHUB;

        std::unordered_map<PortNum, Port*> attachedPorts;
        /**
         * @brief ports that are owned by this class, they will be destructed in the destructor.
         */
        std::unordered_map<PortNum, Virtual::Port*> ownedPorts;
        std::vector<Virtual::GenericDevice*> ownedDevices;

        /**
         * @brief a map that contains if a port has a device attached,
         * used to determine when to send IO attached/detached messages
         */
        std::unordered_map<PortNum, bool> connectedDevices;

        bool m_useBuiltInDevices = true;

        bool updateHubPropertyEnabled[(unsigned int)HubPropertyType::END] = {false};
        std::vector<uint8_t> hubProperty[(unsigned int)HubPropertyType::END];
        void updateHubProperty(HubPropertyType propId);
        void sendHubPropertyUpdate(HubPropertyType propId);
        void resetHubProperty(HubPropertyType propId);
        void handleHubPropertyMessage(std::vector<uint8_t> message);

        bool hubAlertEnabled[(unsigned int)HubAlertType::END] = {false};
        bool hubAlert[(unsigned int)HubAlertType::END] = {false};

        bool m_firstUpdate = true;

    public:
        void updateHubAlert(HubAlertType alert, bool on);

    private:
        void sendHubAlertUpdate(HubAlertType alert);
        void resetHubAlerts();
        void handleHubAlertsMessage(std::vector<uint8_t> message);

        void handlePortInformationRequestMessage(std::vector<uint8_t> message);
        void handlePortModeInformationRequestMessage(std::vector<uint8_t> message);

        void checkPort(PortNum portNum, Port* port);

        void initBuiltInPorts();
        void initBuiltInDevices();

        void destroyBuiltIn();

    public:
        HubEmulation();
        HubEmulation(std::string hubName, HubType hubType);
        ~HubEmulation();

        /**
         * @brief reset Hub properties, does not detach ports!
         */
        void reset();

        /**
         * @brief Starts BLE advertising, resets hub props
         */
        void start();

        /**
         * @brief call this periodically to check if
         * attached ports have devices attached or not
         */
        void update();

        /**
         * @brief sets if the library should initialize the default
         * built-in devices, defaults to true.
         */
        void setUseBuiltInDevices(bool use);

        void setHubRssi(int8_t rssi);
        void setHubBatteryLevel(uint8_t batteryLevel);
        void setHubBatteryType(BatteryType batteryType);
        void setHubName(std::string hubName);

        std::string getHubName();
        BatteryType getBatteryType();

        void setHubFirmwareVersion(Version version);
        void setHubHardwareVersion(Version version);
        void setHubButton(bool pressed);

        /**
         * @brief Attach a port object, the class will take care of the devices atached/detached.
         * @param portNum The port number that will be assigned to the port.
         * @param port The port object, it's lifetime must exceed the HubEmulation instance's lifetime.
         */
        void attachPort(PortNum portNum, Port* port);

        void writeValue(MessageType messageType, std::vector<uint8_t> payload);

        void onMessageReceived(std::vector<uint8_t> message);

        bool isConnected = false;
        bool isSubscribed = false;
        bool isPortInitialized = false;
        BLECharacteristic *pCharacteristic;
    };
}; // namespace Lpf2