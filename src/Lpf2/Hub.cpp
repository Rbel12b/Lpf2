/*
 * Hub.cpp - Arduino base class for controlling Powered UP and Boost controllers
 *
 * (c) Copyright 2020 - Cornelius Munz
 * (c) Copyright 2026 - Rbel12b, canged to work with NimBLE-Arduino 2.x, and integrate the library into the Lpf2 project (https://github.com/Rbel12b/Lpf2)
 * Released under MIT License
 *
 */
#include "Lpf2/Hub.hpp"
#include "Lpf2/Util/Values.hpp"
#include "Lpf2/log/log.h"
#include "Lpf2/DeviceDescLib.hpp"
#include "Lpf2/Remote/Port_internal.hpp"
#include <sstream>
#include <iomanip>

namespace Lpf2
{
    /**
     * Derived class which could be added as an instance to the BLEClient for callback handling
     * The current hub is given as a parameter in the constructor to be able to set the
     * status flags on a disconnect event accordingly
     */
    class HubClientCallback : public BLEClientCallbacks
    {

        Hub *_lpf2Hub;

    public:
        HubClientCallback(Hub *lpf2Hub) : BLEClientCallbacks()
        {
            _lpf2Hub = lpf2Hub;
        }

        void onConnect(BLEClient *bleClient) override
        {
        }

        void onDisconnect(BLEClient *bleClient, int reason) override
        {
            _lpf2Hub->_isConnecting = false;
            _lpf2Hub->_isConnected = false;
            _lpf2Hub->onDisconnect();
            LPF2_LOG_D("Disconnected client, reason: %i", reason);
        }
    };

    /**
     * Scan for BLE servers and find the first one that advertises the service we are looking for.
     */
    class Lpf2HubAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
    {
        Hub *_lpf2Hub;

    public:
        Lpf2HubAdvertisedDeviceCallbacks(Hub *lpf2Hub) : BLEAdvertisedDeviceCallbacks()
        {
            _lpf2Hub = lpf2Hub;
        }

        void onScanEnd(const NimBLEScanResults &results, int reason) override
        {
            LPF2_LOG_D("Scan Ended reason: %d\nNumber of devices: %d", reason, results.getCount());
            for (int i = 0; i < results.getCount(); i++)
            {
                LPF2_LOG_D("device[%d]: %s", i, results.getDevice(i)->toString().c_str());
            }
        }

        void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
        {
            // Found a device, check if the service is contained and optional if address fits requested address
            LPF2_LOG_D("advertised device: %s", advertisedDevice->toString().c_str());

            if (advertisedDevice->haveServiceUUID() && advertisedDevice->getServiceUUID().equals(_lpf2Hub->_bleUuid) && (_lpf2Hub->_requestedDeviceAddress == nullptr || (_lpf2Hub->_requestedDeviceAddress && advertisedDevice->getAddress().equals(*_lpf2Hub->_requestedDeviceAddress))))
            {
                advertisedDevice->getScan()->stop();
                _lpf2Hub->_pServerAddress = new BLEAddress(advertisedDevice->getAddress());
                _lpf2Hub->setHubNameProp(advertisedDevice->getName());

                if (advertisedDevice->haveManufacturerData())
                {
                    LPF2_LOG_D("advertisement payload: %s", Utils::bytes_to_hexString(advertisedDevice->getPayload()).c_str());
                    LPF2_LOG_D("manufacturer data: %s", Utils::bytes_to_hexString(advertisedDevice->getManufacturerData()).c_str());
                    uint8_t *manufacturerData = (uint8_t *)advertisedDevice->getManufacturerData().data();
                    uint8_t manufacturerDataLength = advertisedDevice->getManufacturerData().length();
                    if (manufacturerDataLength >= 3)
                    {
                        LPF2_LOG_D("manufacturer data hub type: %x", manufacturerData[3]);
                        // check device type ID
                        switch (manufacturerData[3])
                        {
                        case DUPLO_TRAIN_HUB_ID:
                            _lpf2Hub->m_hubType = HubType::DUPLO_TRAIN_HUB;
                            break;
                        case BOOST_MOVE_HUB_ID:
                            _lpf2Hub->m_hubType = HubType::BOOST_MOVE_HUB;
                            break;
                        case POWERED_UP_HUB_ID:
                            _lpf2Hub->m_hubType = HubType::POWERED_UP_HUB;
                            break;
                        case POWERED_UP_REMOTE_ID:
                            _lpf2Hub->m_hubType = HubType::POWERED_UP_REMOTE;
                            break;
                        case CONTROL_PLUS_HUB_ID:
                            _lpf2Hub->m_hubType = HubType::CONTROL_PLUS_HUB;
                            break;
                        case MARIO_HUB_ID:
                            _lpf2Hub->m_hubType = HubType::MARIO_HUB;
                            break;
                        default:
                            _lpf2Hub->m_hubType = HubType::UNKNOWNHUB;
                            break;
                        }
                    }
                }
                _lpf2Hub->_isConnecting = true;
            }
        }
    };

    /**
     * @brief Write value to the remote characteristic
     * @param [in] command byte array which contains the ble command
     * @param [in] size length of the command byte array
     */
    void Hub::writeValue(MessageType type, const std::vector<uint8_t> &data)
    {
        if (!_isConnected || !_pRemoteCharacteristic)
            return;
        size_t size = data.size();
        std::vector<uint8_t> fullData;
        fullData.insert(fullData.begin(), {(uint8_t)(size + 2), (uint8_t)0x00});
        fullData.push_back((uint8_t)type);
        fullData.insert(fullData.end(), data.begin(), data.end());
        LPF2_LOG_D("write value: %s", Utils::bytes_to_hexString(fullData).c_str());
        _pRemoteCharacteristic->writeValue(fullData, false);
    }

    /**
     * @brief Callback function for notifications of a specific characteristic
     * @param [in] pBLERemoteCharacteristic The pointer to the characteristic
     * @param [in] pData The pointer to the received data
     * @param [in] length The length of the data array
     * @param [in] isNotify
     */
    void Hub::notifyCallback(
        NimBLERemoteCharacteristic *pBLERemoteCharacteristic,
        uint8_t *pData,
        size_t length,
        bool isNotify)
    {
        LPF2_LOG_D("notify callback, value: %s", Utils::bytes_to_hexString(std::vector<uint8_t>(pData, pData + length)).c_str());

        std::vector<uint8_t> data(pData, pData + length);

        if (data.size() < 3)
            return;

        switch ((MessageType)data[2])
        {
        case MessageType::HUB_PROPERTIES:
        {
            handleHubPropertyMessage(data);
            break;
        }
        case MessageType::GENERIC_ERROR_MESSAGES:
        {
            handleGenericErrorMessage(data);
            break;
        }
        case MessageType::HUB_ATTACHED_IO:
        {
            handleAttachedIOMessage(data);
            break;
        }
        case MessageType::PORT_INFORMATION:
        {
            handlePortInfoMessage(data);
            break;
        }
        case MessageType::PORT_MODE_INFORMATION:
        {
            handlePortModeInfoMessage(data);
            break;
        }
        case MessageType::PORT_INPUT_FORMAT_SINGLE:
        {
            handlePortInputFormatSingleMessage(data);
            break;
        }
        case MessageType::PORT_VALUE_SINGLE:
        {
            handlePortValueSingleMessage(data);
            break;
        }
        default:
        {
            LPF2_LOG_E("Unimplemented: %i", data[2]);
        }
        }
    }

    void Hub::updateHubProperty(HubPropertyType propId, std::vector<uint8_t> data, bool sendUpdate)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        auto &prop = hubProperty[(uint8_t)propId];
        prop = data;
        if (sendUpdate)
            sendHubPropertyUpdate(propId);
    }

    void Hub::sendHubPropertyUpdate(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        if (propId != HubPropertyType::ADVERTISING_NAME && propId != HubPropertyType::HW_NETWORK_ID && propId != HubPropertyType::HARDWARE_NETWORK_FAMILY)
            return;
        if (isPending_warn())
            return;
        auto &prop = hubProperty[(uint8_t)propId];
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)propId);
        payload.push_back((uint8_t)HubPropertyOperation::SET_DOWNSTREAM);
        payload.insert(payload.end(), prop.begin(), prop.end());
        writeValue(MessageType::HUB_PROPERTIES, payload);
    }

    void Hub::enableHubProperty(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        if (propId != HubPropertyType::ADVERTISING_NAME && propId != HubPropertyType::BUTTON && propId != HubPropertyType::RSSI && propId != HubPropertyType::BATTERY_VOLTAGE)
            return;
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)propId);
        payload.push_back((uint8_t)HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM);
        writeValue(MessageType::HUB_PROPERTIES, payload);
        LPF2_LOG_D("Enabled prop update: %i", (uint8_t)m_dataRequestState.propId);
    }

    void Hub::disableHubProperty(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        if (propId != HubPropertyType::ADVERTISING_NAME && propId != HubPropertyType::BUTTON && propId != HubPropertyType::RSSI && propId != HubPropertyType::BATTERY_VOLTAGE)
            return;
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)propId);
        payload.push_back((uint8_t)HubPropertyOperation::DISABLE_UPDATES_DOWNSTREAM);
        writeValue(MessageType::HUB_PROPERTIES, payload);
    }

    void Hub::resetHubProperty(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        if (propId != HubPropertyType::ADVERTISING_NAME && propId != HubPropertyType::HW_NETWORK_ID)
            return;
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)propId);
        payload.push_back((uint8_t)HubPropertyOperation::RESET_DOWNSTREAM);
        writeValue(MessageType::HUB_PROPERTIES, payload);
    }

    void Hub::requestHubPropertyUpdate(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        if (isPending_warn())
            return;
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)propId);
        payload.push_back((uint8_t)HubPropertyOperation::REQUEST_UPDATE_DOWNSTREAM);
        pending(MessageType::HUB_PROPERTIES);
        writeValue(MessageType::HUB_PROPERTIES, payload);
    }

    void Hub::handleHubPropertyMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 5))
        {
            return;
        }
        HubPropertyOperation op = (HubPropertyOperation)message[(byte)MessageByte::OPERATION];
        HubPropertyType propId = (HubPropertyType)message[(byte)MessageByte::PROPERTY];
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return;
        }
        auto &prop = hubProperty[(uint8_t)propId];

        if (pendingRequest.msgType == MessageType::HUB_PROPERTIES)
        {
            pendingRequest.valid = false;
        }

        switch (op)
        {
        case HubPropertyOperation::UPDATE_UPSTREAM:
        {
            prop.assign(message.begin() + 5, message.end());
            LPF2_LOG_D("Updating hub prop: %i, message: %s",
                       (int)propId, Utils::bytes_to_hexString(message).c_str());
            break;
        }
        default:
            goto unimplemented;
        }
        return;
    unimplemented:
        LPF2_LOG_E("Unimplemented: op: %i, propId: %i", (int)op, (int)propId);
        return;
    }

    void Hub::handleGenericErrorMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 5))
        {
            return;
        }
        GenericErrorType errorType = (GenericErrorType)message[(byte)MessageByte::OPERATION];
        MessageType msgType = (MessageType)message[(byte)MessageByte::PROPERTY];

        if (pendingRequest.msgType == msgType)
        {
            pendingRequest.valid = false;
        }

        switch (errorType)
        {
        case GenericErrorType::ACK:
        {
            LPF2_LOG_V("Acknowledged message of type: %i", (int)msgType);
            break;
        }
        default:
            goto unimplemented;
        }
        return;
    unimplemented:
        LPF2_LOG_E("Unimplemented: errorType: %i, msgType: %i", (int)errorType, (int)msgType);
        return;
    }

    void Hub::handleAttachedIOMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 5))
        {
            return;
        }
        PortNum portNum = (PortNum)message[(byte)MessageByte::PORT_ID];
        IOEvent event = (IOEvent)message[(byte)MessageByte::OPERATION];

        switch (event)
        {
        case IOEvent::ATTACHED_IO:
        {
            if (checkLenght(message, 15))
            {
                return;
            }
            if (attachedPorts.count(portNum) && attachedPorts[portNum] != DeviceType::UNKNOWNDEVICE)
            {
                LPF2_LOG_E("Port 0x%02X has another device attached.", (int)portNum);
                break;
            }
            DeviceType devType = (DeviceType)message[5]; // | message[6] << 8;
            std::vector<uint8_t> raw;
            Version HWRew;
            raw.assign(message.begin() + 7, message.end());
            HWRew = Utils::unPackVersion(raw);
            Version FWRew;
            raw.assign(message.begin() + 11, message.end());
            FWRew = Utils::unPackVersion(raw);
            LPF2_LOG_D("Attached IO: HWRew: %u.%u.%u.%u, FWRew: %u.%u.%u.%u, Port: 0x%02X, DevType: 0x%02X",
                       HWRew.Major, HWRew.Minor, HWRew.Bugfix, HWRew.Build,
                       FWRew.Major, FWRew.Minor, FWRew.Bugfix, FWRew.Build,
                       (int)portNum, (int)devType);

            attachedPorts[portNum] = devType;
            auto port = _getPort(portNum);
            if (auto desc = DeviceDescRegistry::instance().getDescriptor(devType))
            {
                port->setDevType(devType);
                port->setFromDesc(desc);
            }
            break;
        }
        case IOEvent::DETACHED_IO:
        {
            attachedPorts[portNum] = DeviceType::UNKNOWNDEVICE;
            auto port = _getPort(portNum);
            port->setDevType(DeviceType::UNKNOWNDEVICE);
            port->getModes_mod().clear();
            break;
        }
        default:
            goto unimplemented;
        }
        return;
    unimplemented:
        LPF2_LOG_E("Unimplemented: event: %i, portNum: %i", (int)event, (int)portNum);
        return;
    }

    void Hub::handlePortInfoMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 5))
        {
            return;
        }
        PortNum portNum = (PortNum)message[(byte)MessageByte::PORT_ID];
        uint8_t infoType = message[(byte)MessageByte::OPERATION];

        if (pendingRequest.msgType == MessageType::PORT_INFORMATION_REQUEST)
        {
            pendingRequest.valid = false;
        }

        auto &port = *_getPort(portNum);

        LPF2_LOG_D("Received port info: portNum: 0x%02X, infoType: %i", (int)portNum, infoType);

        switch (infoType)
        {
        case 0x01:
        {
            if (checkLenght(message, 11))
            {
                return;
            }
            port.setCaps(message[5]);

            port.getModes_mod().resize(message[6]);
            port.setModes(message[6]);

            port.setInModes(message[7] | (message[8] << 8));
            port.setOutModes(message[9] | (message[10] << 8));
            break;
        }
        case 0x02:
        {
            std::vector<uint16_t> combos;
            combos.assign(message.begin() + 5, message.end());
            port.setModeCombos(combos);
            break;
        }
        default:
            goto unimplemented;
        }
        return;
    unimplemented:
        LPF2_LOG_E("Unimplemented!");
        return;
    }

    void Hub::handlePortModeInfoMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 6))
        {
            return;
        }
        PortNum portNum = (PortNum)message[(byte)MessageByte::PORT_ID];
        uint8_t modeNum = message[(byte)MessageByte::OPERATION];
        ModeInfoType infoType = (ModeInfoType)message[(byte)MessageByte::PAYLOAD];

        if (pendingRequest.msgType == MessageType::PORT_MODE_INFORMATION_REQUEST)
        {
            pendingRequest.valid = false;
        }

        auto &port = *_getPort(portNum);
        if (port.getModes_mod().size() <= modeNum)
        {
            port.getModes_mod().resize(modeNum + 1);
        }
        auto &mode = port.getModes_mod()[modeNum];

        LPF2_LOG_D("Received mode port info: portNum: 0x%02X, mode: %i, infoType: %i", (int)portNum, modeNum, (int)infoType);

        switch (infoType)
        {
        case ModeInfoType::NAME:
        {
            mode.name.assign(message.begin() + 6, message.end());
            break;
        }
        case ModeInfoType::RAW:
        case ModeInfoType::PCT:
        case ModeInfoType::SI:
        {
            if (checkLenght(message, 14))
            {
                break;
            }
            float min, max;
            std::memcpy(&min, message.data() + 6, 4);
            std::memcpy(&max, message.data() + 10, 4);

            switch (infoType)
            {
            case ModeInfoType::RAW:
            {
                mode.min = min;
                mode.max = max;
                break;
            }
            case ModeInfoType::PCT:
            {
                mode.PCTmin = min;
                mode.PCTmax = max;
                break;
            }
            case ModeInfoType::SI:
            {
                mode.SImin = min;
                mode.SImax = max;
                break;
            }
            }
            break;
        }
        case ModeInfoType::SYMBOL:
        {
            mode.unit.assign(message.begin() + 6, message.end());
            break;
        }
        case ModeInfoType::MAPPING:
        {
            if (checkLenght(message, 8))
            {
                break;
            }
            mode.in.val = message[6];
            mode.out.val = message[7];
            break;
        }
        case ModeInfoType::MOTOR_BIAS:
        {
            if (checkLenght(message, 7))
            {
                return;
            }
            mode.motor_bias = message[6];
            break;
        }
        case ModeInfoType::CAPS:
        {
            if (checkLenght(message, 12))
            {
                break;
            }
            mode.flags.bytes[5] = message[6];
            mode.flags.bytes[4] = message[7];
            mode.flags.bytes[3] = message[8];
            mode.flags.bytes[2] = message[9];
            mode.flags.bytes[1] = message[10];
            mode.flags.bytes[0] = message[11];
            break;
        }
        case ModeInfoType::VALUE:
        {
            if (checkLenght(message, 10))
            {
                break;
            }
            mode.data_sets = message[6];
            mode.format = message[7];
            mode.figures = message[8];
            mode.decimals = message[9];
            break;
        }
        default:
            goto unimplemented;
        }
        return;
    unimplemented:
        LPF2_LOG_E("Unimplemented!");
        return;
    }

    void Hub::handlePortInputFormatSingleMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 10))
        {
            return;
        }

        if (pendingRequest.msgType == MessageType::PORT_INPUT_FORMAT_SETUP_SINGLE)
        {
            pendingRequest.valid = false;
        }

        PortInputFormatSingle inputFormat;

        inputFormat.portNum = (PortNum)message[(byte)MessageByte::PORT_ID];
        inputFormat.mode = message[(byte)MessageByte::OPERATION];
        std::memcpy(&inputFormat.delta, message.data() + 5, 4);
        inputFormat.notify = message[9];

        m_portInputFormatMap[inputFormat.portNum] = inputFormat;

        return;
    }

    void Hub::handlePortValueSingleMessage(const std::vector<uint8_t> &message)
    {
        if (checkLenght(message, 5))
        {
            return;
        }

        PortNum portNum = (PortNum)message[(byte)MessageByte::PORT_ID];
        if (!m_portInputFormatMap.count(portNum))
        {
            return;
        }
        auto &port = *_getPort(portNum);
        uint8_t mode = m_portInputFormatMap[portNum].mode;
        auto &modeData = port.getModes_mod();
        if (modeData.size() <= mode)
        {
            return;
        }
        modeData[mode].rawData.assign(message.begin() + 4, message.end());
        return;
    }

    void Hub::requestInfos()
    {
        if (pendingRequest.valid)
            return;

        if (!m_rateLimiter.okayToSend())
            return;

        m_rateLimiter.reset();

        switch (m_dataRequestState.state)
        {
        case DataRequestingState::HUB_ALERTS:
        {
            if (m_dataRequestState.mode >= 4)
            {
                m_dataRequestState.mode = 0;
                m_dataRequestState.state = DataRequestingState::HUB_PROP;
            }
            else
            {
                m_dataRequestState.mode++;
                std::vector<uint8_t> payload;
                payload.push_back((uint8_t)m_dataRequestState.mode);
                payload.push_back(0x01);
                writeValue(MessageType::HUB_ALERTS, payload);
            }
            break;
        }

        case DataRequestingState::HUB_PROP:
        {
            if (m_dataRequestState.propId >= HubPropertyType::HARDWARE_NETWORK_FAMILY) // Hub usually don't reply to this
            {
                m_dataRequestState.finishedRequests = true;
            }
            else
            {
                if (m_dataRequestState.mode == 0)
                {
                    requestHubPropertyUpdate(m_dataRequestState.propId);
                    LPF2_LOG_D("Requested prop update: %i", (uint8_t)m_dataRequestState.propId);
                    m_dataRequestState.mode++;
                }
                else
                {
                    m_dataRequestState.mode = 0;
                    if (m_dataRequestState.propId != HubPropertyType::RSSI)
                        enableHubProperty(m_dataRequestState.propId);
                    m_dataRequestState.propId = HubPropertyType((uint8_t)m_dataRequestState.propId + 1);
                }
            }
            break;
        }

        case DataRequestingState::PORT_INFO:
        {
            if (isPending_warn())
                break;

            LPF2_LOG_D("Requesting port info: infoType: %i, portNum: 0x%02X",
                       m_dataRequestState.mode + 1, (int)m_dataRequestState.portNum);

            if (m_dataRequestState.mode == 0)
            {
                std::vector<uint8_t> payload;
                payload.push_back((uint8_t)m_dataRequestState.portNum);
                payload.push_back(0x01);
                pending(MessageType::PORT_INFORMATION_REQUEST);
                writeValue(MessageType::PORT_INFORMATION_REQUEST, payload);
                m_dataRequestState.mode = 1;
            }
            else if (m_dataRequestState.mode == 1)
            {
                std::vector<uint8_t> payload;
                payload.push_back((uint8_t)m_dataRequestState.portNum);
                payload.push_back(0x02);
                pending(MessageType::PORT_INFORMATION_REQUEST);
                writeValue(MessageType::PORT_INFORMATION_REQUEST, payload);
                m_dataRequestState.mode = 0;
                m_dataRequestState.info = 0;
                m_dataRequestState.state = DataRequestingState::PORT_MODE;
            }
            break;
        }

        case DataRequestingState::PORT_MODE:
        {
            if (isPending_warn())
                break;

            LPF2_LOG_D("Requesting port mode info: mode: %i, infoType: %i, portNum: 0x%02X",
                       m_dataRequestState.mode, m_dataRequestState.info, (int)m_dataRequestState.portNum);

            std::vector<uint8_t> payload;
            payload.push_back((uint8_t)m_dataRequestState.portNum);
            payload.push_back(m_dataRequestState.mode);
            payload.push_back(m_dataRequestState.info);
            pending(MessageType::PORT_MODE_INFORMATION_REQUEST);
            writeValue(MessageType::PORT_MODE_INFORMATION_REQUEST, payload);

            if (m_dataRequestState.info == 0x80)
            {
                m_dataRequestState.mode++;
            }

            auto &port = *_getPort(m_dataRequestState.portNum);
            if (m_dataRequestState.mode >= port.getModeCount())
            {
                port.setDevType(attachedPorts[m_dataRequestState.portNum]);
                m_dataRequestState.mode = 0;
                m_dataRequestState.info = 0;
                m_dataRequestState.finishedRequests = true;
            }

            if (m_dataRequestState.info == 0x80)
            {
                m_dataRequestState.info = 0;
                break;
            }

            if (m_dataRequestState.info == 0x08)
            {
                m_dataRequestState.info = 0x80;
            }
            else if (m_dataRequestState.info == 0x05)
            {
                // Skip these
                m_dataRequestState.info = 0x80;
            }
            else
            {
                m_dataRequestState.info++;
            }
            break;
        }

        default:
            m_dataRequestState.finishedRequests = true;
            break;
        }
    }

    bool Hub::isPending_warn()
    {
        if (pendingRequest.valid)
        {
            LPF2_LOG_W("Another request is still pending.");
            return true;
        };
        return false;
    }

    void Hub::pending(MessageType msgType)
    {
        pendingRequest.sentTime = LPF2_GET_TIME();
        pendingRequest.msgType = msgType;
        pendingRequest.valid = true;
    }

    Remote::Port_internal *Hub::_getPort(PortNum portNum)
    {
        if (!remotePorts.count(portNum))
        {
            remotePorts[portNum] = nullptr;
            LPF2_LOG_D("Adding new NULL port.");
        }
        Remote::Port_internal *pPort = remotePorts[portNum];
        if (pPort == nullptr)
        {
            pPort = new Remote::Port_internal(this, portNum);
            if (!pPort)
            {
                LPF2_LOG_E("Failed to allocate port.");
            }
            LPF2_LOG_D("Initializing port.");
            remotePorts[portNum] = pPort;
        }
        return pPort;
    }

    bool Hub::checkLenght(const std::vector<uint8_t> &message, size_t lenght)
    {
        if (message.size() < lenght)
        {
            LPF2_LOG_E("Unexpected message length: %i, expected at least %i", message.size(), lenght);
            return true;
        }
        return false;
    }

    void Hub::onDisconnect()
    {
        attachedPorts.clear();
    }

    void Hub::setHubNameProp(std::string name)
    {
        updateHubProperty(HubPropertyType::ADVERTISING_NAME, std::vector<uint8_t>(name.begin(), name.end()), false);
    }

    /**
     * @brief Constructor
     */
    Hub::Hub()
        : m_rateLimiter(5)
    {
    }

    Hub::~Hub()
    {
        std::for_each(remotePorts.begin(), remotePorts.end(), [](std::pair<PortNum, Port *> pair)
                      { delete pair.second; });
    };

    /**
     * @brief Init function set the UUIDs and scan for the Hub
     */
    void Hub::init()
    {
        m_dataRequestState.finishedRequests = false;
        _isConnected = false;
        _isConnecting = false;
        _bleUuid = BLEUUID(LPF2_UUID);
        _charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
        m_hubType = HubType::UNKNOWNHUB;

        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan();

        _advertiseDeviceCallback = new Lpf2HubAdvertisedDeviceCallbacks(this);

        if (_advertiseDeviceCallback == nullptr)
        {
            LPF2_LOG_E("failed to create advertise device callback");
            return;
        }

        pBLEScan->setScanCallbacks(_advertiseDeviceCallback);

        pBLEScan->setActiveScan(true);
        // start method with callback function to enforce the non blocking scan. If no callback function is used,
        // the scan starts in a blocking manner
        pBLEScan->start(_scanDuration);
    }

    /**
     * @brief Init function set the UUIDs and scan for the Hub
     * @param [in] deviceAddress to which the arduino should connect represented by a hex string of the format: 00:00:00:00:00:00
     */
    void Hub::init(std::string deviceAddress)
    {
        _requestedDeviceAddress = new BLEAddress(deviceAddress, 0);
        init();
    }

    /**
     * @brief Init function set the BLE scan duration (default value 5s)
     * @param [in] BLE scan durtation in unit seconds
     */
    void Hub::init(uint32_t scanDuration)
    {
        _scanDuration = scanDuration;
        init();
    }

    /**
     * @brief Init function set the BLE scan duration (default value 5s)
     * @param [in] deviceAddress to which the arduino should connect represented by a hex string of the format: 00:00:00:00:00:00
     * @param [in] BLE scan durtation in unit seconds
     */
    void Hub::init(std::string deviceAddress, uint32_t scanDuration)
    {
        _requestedDeviceAddress = new BLEAddress(deviceAddress, 0);
        _scanDuration = scanDuration;
        init();
    }

    void Hub::update()
    {
        if (!isConnected())
            return;

        if (pendingRequest.valid && LPF2_GET_TIME() - pendingRequest.sentTime >= 200)
        {
            LPF2_LOG_E("Request timed out: msgType: %i", (int)pendingRequest.msgType);
            pendingRequest.valid = false;
        }

        if (pendingRequest.valid)
            return;

        if (!m_dataRequestState.finishedRequests)
        {
            requestInfos();
        }

        std::for_each(attachedPorts.begin(), attachedPorts.end(), [this](std::pair<PortNum, DeviceType> attachedPort)
                      {
        _getPort(attachedPort.first);
        if (!m_dataRequestState.finishedRequests)
        {
            return;
        }

        if (attachedPort.second != DeviceType::UNKNOWNDEVICE &&
            !(remotePorts[attachedPort.first]->deviceConnected()))
        {
            LPF2_LOG_D("Starting requests for: port: 0x%02X, dev: 0x%02X",
                (int)attachedPort.first, (int)attachedPort.second);
            m_dataRequestState.portNum = attachedPort.first;
            m_dataRequestState.state = DataRequestingState::PORT_INFO;
            m_dataRequestState.finishedRequests = false;
            m_dataRequestState.mode = 0;
        } });
    }

    /**
     * @brief Get the address of the HUB (server address)
     * @return HUB Address
     */
    NimBLEAddress Hub::getHubAddress()
    {
        NimBLEAddress pAddress = *_pServerAddress;
        return pAddress;
    }

    /**
     * @brief Send the Shutdown command to the HUB
     */
    void Hub::shutDownHub()
    {
        writeValue(MessageType::HUB_ACTIONS, {(uint8_t)HubActionType::SWITCH_OFF_HUB});
    }

    Port *Hub::getPort(PortNum portNum)
    {
        return static_cast<Port *>(_getPort(portNum));
    }

    int Hub::setPortMode(PortNum portNum, uint8_t mode, uint32_t delta, bool notify)
    {
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)portNum);
        payload.push_back(mode);
        payload.push_back((delta >> 0) && 0xFF);
        payload.push_back((delta >> 8) && 0xFF);
        payload.push_back((delta >> 16) && 0xFF);
        payload.push_back((delta >> 24) && 0xFF);
        payload.push_back(notify ? 1 : 0);
        pending(MessageType::PORT_INPUT_FORMAT_SETUP_SINGLE);
        writeValue(MessageType::PORT_INPUT_FORMAT_SETUP_SINGLE, payload);
        return 0;
    }

    bool Hub::infoReady()
    {
        return m_dataRequestState.finishedRequests && !pendingRequest.valid;
    }

    /**
     * @brief Set name of the HUB
     * @param [in] name character array which contains the name (max 14 characters are supported)
     */
    void Hub::setHubName(std::string name)
    {
        if (name.size() > 14)
        {
            return;
        }
        setHubNameProp(name);
        sendHubPropertyUpdate(HubPropertyType::ADVERTISING_NAME);
    }

    /**
     * @brief Connect to the HUB, get a reference to the characteristic and register for notifications
     */
    bool Hub::connectHub()
    {
        BLEAddress pAddress = *_pServerAddress;
        NimBLEClient *pClient = nullptr;

        LPF2_LOG_D("number of ble clients: %d", NimBLEDevice::getCreatedClientCount());

        /** Check if we have a client we should reuse first **/
        if (NimBLEDevice::getCreatedClientCount())
        {
            /** Special case when we already know this device, we send false as the
             *  second argument in connect() to prevent refreshing the service database.
             *  This saves considerable time and power.
             */
            pClient = NimBLEDevice::getClientByPeerAddress(pAddress);
            if (pClient)
            {
                if (!pClient->connect(pAddress, false))
                {
                    LPF2_LOG_E("reconnect failed");
                    return false;
                }
                LPF2_LOG_D("reconnect client");
            }
            /** We don't already have a client that knows this device,
             *  we will check for a client that is disconnected that we can use.
             */
            else
            {
                pClient = NimBLEDevice::getDisconnectedClient();
            }
        }

        /** No client to reuse? Create a new one. */
        if (!pClient)
        {
            if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS)
            {
                LPF2_LOG_W("max clients reached - no more connections available: %d", NimBLEDevice::getCreatedClientCount());
                return false;
            }

            pClient = NimBLEDevice::createClient();
        }

        if (!pClient->isConnected())
        {
            if (!pClient->connect(pAddress))
            {
                LPF2_LOG_E("failed to connect");
                return false;
            }
        }

        LPF2_LOG_D("connected to: %s, RSSI: %d", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
        BLERemoteService *pRemoteService = pClient->getService(_bleUuid);
        if (pRemoteService == nullptr)
        {
            LPF2_LOG_E("failed to get ble client");
            return false;
        }

        _pRemoteCharacteristic = pRemoteService->getCharacteristic(_charachteristicUuid);
        if (_pRemoteCharacteristic == nullptr)
        {
            LPF2_LOG_E("failed to get ble service");
            return false;
        }

        // register notifications (callback function) for the characteristic
        if (_pRemoteCharacteristic->canNotify())
        {
            _pRemoteCharacteristic->subscribe(true, std::bind(&Hub::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), true);
        }

        // add callback instance to get notified if a disconnect event appears
        pClient->setClientCallbacks(new HubClientCallback(this));

        // Set states
        _isConnected = true;
        _isConnecting = false;
        m_dataRequestState.finishedRequests = false;
        m_dataRequestState.propId = HubPropertyType::ADVERTISING_NAME;
        m_dataRequestState.mode = 0;
        m_dataRequestState.state = DataRequestingState::HUB_ALERTS;
        vTaskDelay(200);
        return true;
    }

    /**
     * @brief Retrieve the connection state. The BLE client (ESP32) has found a service with the desired UUID (HUB)
     * If this state is available, you can try to connect to the Hub
     */
    bool Hub::isConnecting()
    {
        return _isConnecting;
    }

    /**
     * @brief Retrieve the connection state. The BLE client (ESP32) is connected to the server (HUB)
     */
    bool Hub::isConnected()
    {
        return _isConnected;
    }

    /**
     * @brief Determine the scanning status
     * @return Scanning status
     */
    bool Hub::isScanning()
    {
        return pBLEScan->isScanning();
    }

    /**
     * @brief Retrieve the hub type
     * @return hub type
     */
    HubType Hub::getHubType()
    {
        return m_hubType;
    }

    std::string Hub::getAllInfoStr()
    {
        std::ostringstream oss;
        oss << "Advertising Name: " << getHubPropStr(HubPropertyType::ADVERTISING_NAME) << "\n";
        oss << "Manufacturer Name: " << getHubPropStr(HubPropertyType::MANUFACTURER_NAME) << "\n";
        oss << "HW version: " << getHubPropStr(HubPropertyType::HW_VERSION) << "\n";
        oss << "FW version: " << getHubPropStr(HubPropertyType::FW_VERSION) << "\n";
        oss << "LWP version: " << getHubPropStr(HubPropertyType::LEGO_WIRELESS_PROTOCOL_VERSION) << "\n";
        oss << "Radio FW version: " << getHubPropStr(HubPropertyType::RADIO_FIRMWARE_VERSION) << "\n";
        oss << "Primary MAC: " << getHubPropStr(HubPropertyType::PRIMARY_MAC_ADDRESS) << "\n";
        oss << "Secondary MAC: " << getHubPropStr(HubPropertyType::SECONDARY_MAC_ADDRESS) << "\n";
        oss << "HW network id: " << getHubPropStr(HubPropertyType::HW_NETWORK_ID) << "\n";
        oss << "System type ID: " << getHubPropStr(HubPropertyType::SYSTEM_TYPE_ID) << "\n";
        oss << "HW network family: " << getHubPropStr(HubPropertyType::HARDWARE_NETWORK_FAMILY) << "\n";
        oss << "RSSI: " << getHubPropStr(HubPropertyType::RSSI) << "\n";
        oss << "Battery type: " << getHubPropStr(HubPropertyType::BATTERY_TYPE) << "\n";
        oss << "Battery voltage: " << getHubPropStr(HubPropertyType::BATTERY_VOLTAGE) << "\n";
        oss << "Button state: " << getHubPropStr(HubPropertyType::BUTTON) << "\n";
        oss << "Devices:" << "\n";
        std::for_each(remotePorts.begin(), remotePorts.end(), [&oss](std::pair<PortNum, Port *> pair)
                      {
        oss << pair.second->getInfoStr();
        oss << "\n"; });
        return oss.str();
    }

    std::string Hub::getHubPropStr(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property.");
            return "";
        }
        auto &prop = hubProperty[(uint8_t)propId];
        return getHubPropStr(propId, prop);
    }

    std::string Hub::getHubPropStr(HubPropertyType propId, std::vector<uint8_t> prop)
    {
        std::string str;
        switch (propId)
        {
        case HubPropertyType::ADVERTISING_NAME:
        {
            str.assign(prop.begin(), prop.end());
            break;
        }
        case HubPropertyType::BATTERY_TYPE:
        {
            prop.resize(1);
            str = prop[0] == (uint8_t)BatteryType::NORMAL ? "Normal" : "Rechargeable";
            break;
        }
        case HubPropertyType::BATTERY_VOLTAGE:
        {
            prop.resize(1);
            str = std::to_string(prop[0]);
            break;
        }
        case HubPropertyType::BUTTON:
        {
            prop.resize(1);
            switch ((ButtonState)prop[0])
            {
            case ButtonState::RELEASED:
                str = "Released";
                break;

            case ButtonState::DOWN:
                str = "Down";
                break;

            case ButtonState::UP:
                str = "Up";
                break;

            case ButtonState::STOP:
                str = "Stop";
                break;

            default:
                break;
            }
            break;
        }
        case HubPropertyType::FW_VERSION:
        {
            prop.resize(4);
            Version version = Utils::unPackVersion(prop);
            str = std::to_string(version.Major) + "." + std::to_string(version.Minor) + "." + std::to_string(version.Bugfix) + "." + std::to_string(version.Build);
            break;
        }
        case HubPropertyType::HARDWARE_NETWORK_FAMILY:
        {
            prop.resize(1);
            str = Utils::bytes_to_hexString(prop);
            break;
        }
        case HubPropertyType::HW_NETWORK_ID:
        {
            prop.resize(1);
            str = Utils::bytes_to_hexString(prop);
            break;
        }
        case HubPropertyType::HW_VERSION:
        {
            prop.resize(4);
            Version version = Utils::unPackVersion(prop);
            str = std::to_string(version.Major) + "." + std::to_string(version.Minor) + "." + std::to_string(version.Bugfix) + "." + std::to_string(version.Build);
            break;
        }
        case HubPropertyType::LEGO_WIRELESS_PROTOCOL_VERSION:
        {
            prop.resize(2);
            str = Utils::bytes_to_hexString(prop);
            break;
        }
        case HubPropertyType::MANUFACTURER_NAME:
        {
            str.insert(str.begin(), prop.begin(), prop.end());
            break;
        }
        case HubPropertyType::SECONDARY_MAC_ADDRESS:
        case HubPropertyType::PRIMARY_MAC_ADDRESS:
        {
            prop.resize(6);
            str = Utils::byte_to_hexString(prop[0]) + ":" +
                  Utils::byte_to_hexString(prop[1]) + ":" +
                  Utils::byte_to_hexString(prop[2]) + ":" +
                  Utils::byte_to_hexString(prop[3]) + ":" +
                  Utils::byte_to_hexString(prop[4]) + ":" +
                  Utils::byte_to_hexString(prop[5]);
            break;
        }
        case HubPropertyType::RADIO_FIRMWARE_VERSION:
        {
            str.insert(str.begin(), prop.begin(), prop.end());
            break;
        }
        case HubPropertyType::RSSI:
        {
            prop.resize(1);
            str = std::to_string((int8_t)prop[0]);
            break;
        }
        case HubPropertyType::SYSTEM_TYPE_ID:
        {
            prop.resize(1);
            str = Utils::bytes_to_hexString(prop);
            break;
        }

        default:
            break;
        }
        return str;
    }

    std::string Hub::getHubName()
    {
        auto &hubName = hubProperty[(unsigned)HubPropertyType::ADVERTISING_NAME];
        std::string str;
        str.insert(str.end(), hubName.begin(), hubName.end());
        return str;
    }

    BatteryType Hub::getBatteryType()
    {
        auto &prop = hubProperty[(unsigned)HubPropertyType::BATTERY_TYPE];
        if (!prop.size())
        {
            prop.push_back((uint8_t)BatteryType::NORMAL);
        }
        return (BatteryType)prop[0];
    }
}; // namespace Lpf2