/*
 * Lpf2Hub.h - Arduino base class for controlling Powered UP and Boost controllers
 *
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 *
 */

#ifndef _LPF2_HUB_H_
#define _LPF2_HUB_H_

#include "config.h"
#include "Arduino.h"
#include "NimBLEDevice.h"
#include "Lpf2Const.h"
#include "Util/RateLimiter.h"
#include "Lpf2Remote/Lpf2PortRemote.h"
#include "unordered_map"

class Lpf2Hub
{
    std::unordered_map<Lpf2PortNum, Lpf2PortRemote*> remotePorts;
    std::unordered_map<Lpf2PortNum, Lpf2DeviceType> attachedPorts; // Ports with attached devices

    std::vector<uint8_t> hubProperty[(unsigned int)Lpf2HubPropertyType::END];
    void updateHubProperty(Lpf2HubPropertyType propId, std::vector<uint8_t> data, bool sendUpdate);
    void sendHubPropertyUpdate(Lpf2HubPropertyType propId);
    void enableHubProperty(Lpf2HubPropertyType propId);
    void disableHubProperty(Lpf2HubPropertyType propId);
    void resetHubProperty(Lpf2HubPropertyType propId);
    void requestHubPropertyUpdate(Lpf2HubPropertyType propId);

    void handleHubPropertyMessage(const std::vector<uint8_t> &message);
    void handleGenericErrorMessage(const std::vector<uint8_t> &message);
    void handleAttachedIOMessage(const std::vector<uint8_t> &message);
    void handlePortInfoMessage(const std::vector<uint8_t> &message);
    void handlePortModeInfoMessage(const std::vector<uint8_t> &message);

    void requestInfos();

    /**
     * @biref returns true if another request is still pending, and writes a warning message to the log.
     */
    bool isPending_warn();

    void pending(Lpf2MessageType msgType);

    Lpf2PortRemote *_getPort(Lpf2PortNum portNum);

    /**
     * @brief check the lenght of a message, and prints an error to the log
     * @returns true if the message is smaller than the given length
     */
    bool checkLenght(const std::vector<uint8_t> &message, size_t lenght);

public:
    Lpf2Hub();
    ~Lpf2Hub();

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
    Lpf2HubType getHubType();
    void setHubName(std::string name);
    void shutDownHub();

    Lpf2Port *getPort(Lpf2PortNum portNum);

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
    std::string getHubPropStr(Lpf2HubPropertyType propId);
    static std::string getHubPropStr(Lpf2HubPropertyType propId, std::vector<uint8_t> prop);

    /* HUB props*/

    std::string getHubName();
    Lpf2BatteryType getBatteryType();

    // write (set) operations on port devices
    void writeValue(Lpf2MessageType type, const std::vector<uint8_t> &data);

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
        Lpf2MessageType msgType;
    } pendingRequest;

    BLEUUID _bleUuid;
    BLEUUID _charachteristicUuid;
    BLEAddress *_pServerAddress;
    BLEAddress *_requestedDeviceAddress = nullptr;
    BLERemoteCharacteristic *_pRemoteCharacteristic;
    BLEScan *pBLEScan;
    Lpf2HubType _hubType;
    bool _isConnecting = false;
    bool _isConnected = false;

private:
    struct
    {
        union
        {
            Lpf2HubPropertyType propId;
            Lpf2PortNum portNum;
        };
        uint8_t mode;
        uint8_t info;
        DataRequestingState state;
        bool finishedRequests = false;
    } m_dataRequestState;

    RateLimiter m_rateLimiter;


    // Notification callback
    NimBLEScanCallbacks *_advertiseDeviceCallback = nullptr;

    // BLE settings
    uint32_t _scanDuration = 10;
};

#endif // Lpf2Hub_h
