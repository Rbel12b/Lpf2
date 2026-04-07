/**
 *  Copyright (C) 2026 - Rbel12b
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  */

#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/LWPConst.hpp"
#include <NimBLEDevice.h>
#include <unordered_map>
#include <list>

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
        class Lpf2HubCharacteristicCallbacks : public NimBLECharacteristicCallbacks
        {
            HubEmulation *_lpf2HubEmulation = nullptr;
        public:
            Lpf2HubCharacteristicCallbacks(HubEmulation *lpf2HubEmulation) : NimBLECharacteristicCallbacks(), _lpf2HubEmulation(lpf2HubEmulation) {}
            void onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, uint16_t subValue) override;
            void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
            void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
        };

        friend class Lpf2HubServerCallbacks;
        friend class Lpf2HubCharacteristicCallbacks;
    private:
        QueueHandle_t m_msgQueue = nullptr;

        BLEServer *m_bleServer = nullptr;
        BLEService *m_bleService = nullptr;
        BLEAdvertising *m_bleAdvertising = nullptr;
        BLECharacteristic *m_bleChar = nullptr;
        bool m_connected = false;
        bool m_subscribed = false;
        bool m_advertising = false;
        uint16_t m_bleConnHandle = 0;
        Lpf2HubCharacteristicCallbacks *m_bleCharCallbacks = nullptr;

        TaskHandle_t m_msgTaskHandle = nullptr;
        bool m_msgTaskShouldQuit = false;

        NimBLEAdvertisementData getAdvertisementData();
        NimBLEAdvertisementData getScanResponseData();

        void writeResponse(MessageType messageType, std::vector<uint8_t> payload);
        void writeValue(std::vector<uint8_t> message);

        HubType m_hubType = HubType::UNKNOWNHUB;

        std::unordered_map<PortNum, Port*> m_attachedPorts;
        /**
         * @brief ports that are owned by this class, they will be destructed in the destructor.
         */
        std::unordered_map<PortNum, Virtual::Port*> m_ownedPorts;
        std::vector<Virtual::GenericDevice*> m_ownedDevices;

        /**
         * @brief a map that contains if a port has a device attached,
         * used to determine when to send IO attached/detached messages
         */
        std::unordered_map<PortNum, bool> m_connectedDevices;

        bool m_useBuiltInDevices = false;

        bool m_updateHubPropertyEnabled[(unsigned int)HubPropertyType::END] = {false};
        std::vector<uint8_t> m_hubProperty[(unsigned int)HubPropertyType::END];
        void updateHubProperty(HubPropertyType propId);
        void sendHubPropertyUpdate(HubPropertyType propId);
        void resetHubProperty(HubPropertyType propId);
        void handleHubPropertyMessage(std::vector<uint8_t> message);

        bool m_hubAlertEnabled[(unsigned int)HubAlertType::END] = {false};
        bool m_hubAlert[(unsigned int)HubAlertType::END] = {false};

        bool m_firstUpdate = true;

        size_t m_lastRssiUpdate = 0;
        int8_t m_lastRssi = 0;

        struct PortInputSetupSingle
        {
            PortNum portNum;
            uint8_t mode;
            uint32_t delta;
            bool notify;
            
            std::vector<uint8_t> lastRaw;
        };
        // map<PortNum, map<ModeNum, Setup>>
        std::unordered_map<PortNum, std::unordered_map<uint8_t, PortInputSetupSingle>> m_portSetupSingle;

        void sendHubAlertUpdate(HubAlertType alert);
        void resetHubAlerts();
        void handleHubAlertsMessage(std::vector<uint8_t> message);
        void handleHubActionsMessage(std::vector<uint8_t> message);
        void handlePortInformationRequestMessage(std::vector<uint8_t> message);
        void handlePortModeInformationRequestMessage(std::vector<uint8_t> message);
        void handlePortInputFormatSetupSingleMessage(std::vector<uint8_t> message);
        void handlePortOutputCommandMessage(std::vector<uint8_t> message);

        void checkPort(PortNum portNum, Port* port);
        void checkPortModeValueSingle(PortInputSetupSingle &setup, Port* port);

        void sendPortValueSingle(PortInputSetupSingle &setup, Port* port);

        void initBuiltInPorts();
        void initBuiltInDevices();

        void destroyBuiltIn();

        void processMessages(const std::vector<uint8_t>& message);

        static inline void msgTask(void *pvParams)
        {
            HubEmulation *hubEmulation = static_cast<HubEmulation*>(pvParams);
            if (hubEmulation == nullptr) {
                abort();
            }
            hubEmulation->msgTaskLoop();
            vTaskDelete(NULL);
            return;
        }

        void msgTaskLoop();

        void update();

        void reset();

        void setHubRssi(int8_t rssi);

    public:
        HubEmulation();
        HubEmulation(std::string hubName, HubType hubType);
        ~HubEmulation();

        /**
         * @brief Starts BLE advertising, resets hub props,
         * start message handling task
         */
        void start();

        /**
         * @brief End hub emulation:
         * Disconnects / stops BLE advertising,
         * Deletes message handling task
         */
        void stop();

        /**
         * @brief sets if the library should initialize the default
         * built-in devices, defaults to false. (Call before start())
         */
        void setUseBuiltInDevices(bool use);
        void setBatteryLevel(uint8_t batteryLevel);
        void setBatteryType(BatteryType batteryType);
        void setName(std::string hubName);
        void setFirmwareVersion(Version version);
        void setHardwareVersion(Version version);
        void setButtonState(ButtonState state);
        void setAlert(HubAlertType alert, bool on);

        std::string getName();
        BatteryType getBatteryType();

        /**
         * @brief Attach a port object, the class will take care of the devices atached/detached.
         * @param portNum The port number that will be assigned to the port.
         * @param port The port object, it's lifetime must exceed the HubEmulation instance's lifetime.
         */
        void attachPort(PortNum portNum, Port* port);
    };
}; // namespace Lpf2