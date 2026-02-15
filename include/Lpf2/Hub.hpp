/*
 * Lpf2Hub.hpp - Arduino base class for controlling Powered UP and Boost controllers
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
 */

#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/LWPConst.hpp"
#include "Lpf2/Util/RateLimiter.hpp"
#include "Lpf2/Remote/Port.hpp"

#include "NimBLEDevice.h"
#include "unordered_map"

namespace Lpf2
{
    namespace Remote
    {
        class Port_internal;
    };
    class Hub
    {
        friend class HubClientCallback;
        std::unordered_map<PortNum, Remote::Port_internal *> remotePorts;
        std::unordered_map<PortNum, DeviceType> attachedPorts; // Ports with attached devices

        std::vector<uint8_t> hubProperty[(unsigned int)HubPropertyType::END];
        void updateHubProperty(HubPropertyType propId, std::vector<uint8_t> data, bool sendUpdate);
        void sendHubPropertyUpdate(HubPropertyType propId);
        void enableHubProperty(HubPropertyType propId);
        void disableHubProperty(HubPropertyType propId);
        void resetHubProperty(HubPropertyType propId);
        void requestHubPropertyUpdate(HubPropertyType propId);

        void handleHubPropertyMessage(const std::vector<uint8_t> &message);
        void handleGenericErrorMessage(const std::vector<uint8_t> &message);
        void handleAttachedIOMessage(const std::vector<uint8_t> &message);
        void handlePortInfoMessage(const std::vector<uint8_t> &message);
        void handlePortModeInfoMessage(const std::vector<uint8_t> &message);
        void handlePortInputFormatSingleMessage(const std::vector<uint8_t> &message);
        void handlePortValueSingleMessage(const std::vector<uint8_t> &message);

        void requestInfos();

        /**
         * @biref returns true if another request is still pending, and writes a warning message to the log.
         */
        bool isPending_warn();

        void pending(MessageType msgType);

        Remote::Port_internal *_getPort(PortNum portNum);

        /**
         * @brief check the lenght of a message, and prints an error to the log
         * @returns true if the message is smaller than the given length
         */
        bool checkLenght(const std::vector<uint8_t> &message, size_t lenght);

        void onDisconnect();

    public:
        Hub();
        ~Hub();

        // initializer methods
        void init();
        void init(uint32_t scanDuration);
        void init(std::string deviceAddress);
        void init(std::string deviceAddress, uint32_t scanDuration);

        void update();

        // hub related methods
        bool connectHub();
        bool isConnected();
        bool isConnecting();
        bool isScanning();
        NimBLEAddress getHubAddress();
        HubType getHubType();
        void setHubName(std::string name);
        void shutDownHub();

        Port *getPort(PortNum portNum);
        int setPortMode(PortNum portNum, uint8_t mode, uint32_t delta, bool notify = true);

        /**
         * @brief returns true if all infomation requests were sent, and answered (or timed out).
         */
        bool infoReady();

        /**
         * @brief Internal, do not use!
         */
        void setHubNameProp(std::string name);
        /**
         * @brief returns all available information sent by the hub (Hub properties, port modes ...)
         */
        std::string getAllInfoStr();
        std::string getHubPropStr(HubPropertyType propId);
        static std::string getHubPropStr(HubPropertyType propId, std::vector<uint8_t> prop);

        /* HUB props*/

        std::string getHubName();
        BatteryType getBatteryType();

        void writeValue(MessageType type, const std::vector<uint8_t> &data);

        // BLE specific stuff
        void notifyCallback(NimBLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);

        enum class DataRequestingState
        {
            HUB_ALERTS,
            HUB_PROP,
            PORT_INFO,
            PORT_MODE
        };

        class PendingRequest
        {
        public:
            bool valid = false;
            size_t sentTime = 0;
            MessageType msgType;
        } pendingRequest;

        BLEUUID _bleUuid;
        BLEUUID _charachteristicUuid;
        BLEAddress *_pServerAddress;
        BLEAddress *_requestedDeviceAddress = nullptr;
        BLERemoteCharacteristic *_pRemoteCharacteristic;
        BLEScan *pBLEScan;
        HubType m_hubType;
        bool _isConnecting = false;
        bool _isConnected = false;

        struct PortInputFormatSingle
        {
            PortNum portNum;
            uint8_t mode;
            uint32_t delta;
            bool notify;
        };

    private:
        std::unordered_map<PortNum, PortInputFormatSingle> m_portInputFormatMap;

    private:
        struct
        {
            union
            {
                HubPropertyType propId;
                PortNum portNum;
            };
            uint8_t mode;
            uint8_t info;
            DataRequestingState state;
            bool finishedRequests = false;
        } m_dataRequestState;

        Utils::RateLimiter m_rateLimiter;

        // Notification callback
        NimBLEScanCallbacks *_advertiseDeviceCallback = nullptr;

        // BLE settings
        uint32_t _scanDuration = 10;
    };
}; // namespace Lpf2