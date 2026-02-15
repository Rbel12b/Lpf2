#include "Lpf2/log/log.h"
#include "Lpf2/Hub.hpp"
#include "Lpf2/HubEmulation.hpp"
#include "Lpf2/Util/Values.hpp"
#include "Lpf2/Virtual/Port.hpp"
#include "Lpf2/Virtual/Device.hpp"
#include "Lpf2/DeviceDescLib.hpp"

namespace Lpf2
{
    class Lpf2HubServerCallbacks : public NimBLEServerCallbacks
    {

        HubEmulation *_lpf2HubEmulation;

    public:
        Lpf2HubServerCallbacks(HubEmulation *lpf2HubEmulation) : NimBLEServerCallbacks()
        {
            _lpf2HubEmulation = lpf2HubEmulation;
        }

        void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
        {
            LPF2_LOG_D("Device connected");
            _lpf2HubEmulation->isConnected = true;
            pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
        };

        void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
        {
            LPF2_LOG_D("Device disconnected, reason: %i", reason);
            _lpf2HubEmulation->isConnected = false;
            _lpf2HubEmulation->isSubscribed = false;
            _lpf2HubEmulation->isPortInitialized = false;
        }
    };

    class Lpf2HubCharacteristicCallbacks : public NimBLECharacteristicCallbacks
    {
        HubEmulation *_lpf2HubEmulation;

    public:
        Lpf2HubCharacteristicCallbacks(HubEmulation *lpf2HubEmulation) : NimBLECharacteristicCallbacks()
        {
            _lpf2HubEmulation = lpf2HubEmulation;
        }

        void onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, uint16_t subValue) override
        {
            LPF2_LOG_D("Client subscription status: %s (%d)",
                       subValue == 0 ? "Un-Subscribed" : subValue == 1 ? "Notifications"
                                                     : subValue == 2   ? "Indications"
                                                     : subValue == 3   ? "Notifications and Indications"
                                                                       : "unknown subscription status",
                       subValue);

            _lpf2HubEmulation->isSubscribed = subValue != 0;
        }

        void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
        {
            std::vector<uint8_t> msgReceived = pCharacteristic->getValue();
            pCharacteristic->setValue(nullptr, 0);

            if (msgReceived.size() == 0)
            {
                LPF2_LOG_W("empty message received");
                return;
            }

            _lpf2HubEmulation->onMessageReceived(msgReceived);
        }

        void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
        {
            LPF2_LOG_D("read request");
        }
    };

    void HubEmulation::onMessageReceived(std::vector<uint8_t> message)
    {
        MessageType type = (MessageType)message[(byte)MessageHeader::MESSAGE_TYPE];
        LPF2_DEBUG_EXPR_D(
            std::string hexMessage = Utils::bytes_to_hexString(message);
            LPF2_LOG_D("message received (%d): %s", message.size(), hexMessage.c_str()););
        LPF2_LOG_V("message type: %d", (byte)type);

        switch (type)
        {
        case MessageType::HUB_PROPERTIES:
            handleHubPropertyMessage(message);
            break;
        case MessageType::HUB_ACTIONS:
            LPF2_LOG_W("HUB_ACTIONS not implemented yet");
            break;
        case MessageType::HUB_ALERTS:
            handleHubAlertsMessage(message);
            break;
        case MessageType::HW_NETWORK_COMMANDS:
            LPF2_LOG_W("HW_NETWORK_COMMANDSnot implemented yet");
            break;
        case MessageType::FW_UPDATE_GO_INTO_BOOT_MODE:
            LPF2_LOG_W("FW_UPDATE_GO_INTO_BOOT_MODE not implemented yet");
            break;
        case MessageType::FW_UPDATE_LOCK_MEMORY:
            LPF2_LOG_W("FW_UPDATE_LOCK_MEMORY not implemented yet");
            break;
        case MessageType::FW_UPDATE_LOCK_STATUS_REQUEST:
            LPF2_LOG_W("FW_UPDATE_LOCK_STATUS_REQUEST not implemented yet");
            break;
        case MessageType::PORT_INFORMATION_REQUEST:
            handlePortInformationRequestMessage(message);
            break;
        case MessageType::PORT_MODE_INFORMATION_REQUEST:
            handlePortModeInformationRequestMessage(message);
            break;
        case MessageType::PORT_INPUT_FORMAT_SETUP_COMBINEDMODE:
            LPF2_LOG_W("PORT_INPUT_FORMAT_SETUP_COMBINEDMODE not implemented yet");
            break;
        case MessageType::VIRTUAL_PORT_SETUP:
            LPF2_LOG_W("VIRTUAL_PORT_SETUP not implemented yet");
            break;
        case MessageType::PORT_OUTPUT_COMMAND:
            LPF2_LOG_W("PORT_OUTPUT_COMMAND not implemented yet");
            break;

        default:
            goto unimplemented;
        }

        //   // It's a port out command:
        //   // execute and send feedback to the App
        //   if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (char)MessageType::PORT_OUTPUT_COMMAND)
        //   {
        //     byte port = msgReceived[(byte)PortOutputMessage::PORT_ID];
        //     byte startCompleteInfo = msgReceived[(byte)PortOutputMessage::STARTUP_AND_COMPLETION];
        //     byte subCommand = msgReceived[(byte)PortOutputMessage::SUB_COMMAND];

        //     // Reply to the App "Command excecuted" if the App requests a feedback.
        //     // https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#port-output-command-feedback-format
        //     if ((startCompleteInfo & 0x01) != 0) // Command feedback (status) requested
        //     {
        //       std::string payload;
        //       payload.push_back((char)port);
        //       payload.push_back((byte)PortFeedbackMessage::BUFFER_EMPTY_AND_COMMAND_COMPLETED | (byte)PortFeedbackMessage::IDLE);
        //       _lpf2HubEmulation->writeValue(MessageType::PORT_OUTPUT_COMMAND_FEEDBACK, payload);
        //     }

        //     if (subCommand == 0x51) // OUT_PORT_CMD_WRITE_DIRECT
        //     {
        //       byte commandMode = msgReceived[0x06];
        //       byte power = msgReceived[0x07];
        //       if (_lpf2HubEmulation->writePortCallback != nullptr)
        //       {
        //         _lpf2HubEmulation->writePortCallback(msgReceived[(byte)PortOutputMessage::PORT_ID], power); // WRITE_DIRECT_VALUE
        //       }
        //     }
        //     else if (subCommand == 0x07) // StartSpeed (Speed, MaxPower, UseProfile)
        //     {
        //       byte speed = msgReceived[0x06];
        //       byte maxSpeed = msgReceived[0x07];
        //       byte useProfile = msgReceived[0x08];
        //       if (_lpf2HubEmulation->writePortCallback != nullptr)
        //       {
        //         _lpf2HubEmulation->writePortCallback(msgReceived[(byte)PortOutputMessage::PORT_ID], speed); // WRITE_DIRECT_VALUE
        //       }
        //     }
        //   }

        //   if (msgReceived[(byte)MessageHeader::MESSAGE_TYPE] == (byte)MessageType::HUB_ACTIONS && msgReceived[3] == (byte)ActionType::SWITCH_OFF_HUB)
        //   {
        //     LPF2_LOG_D("disconnect");
        //     std::string payload;
        //     payload.push_back(0x31);
        //     _lpf2HubEmulation->writeValue(MessageType::HUB_ACTIONS, payload);
        //     delay(100);
        //     LPF2_LOG_D("restart ESP");
        //     delay(1000);
        //     ESP.restart();
        //   }

        return;

    unimplemented:
        LPF2_LOG_E("Unimplemented!");
        return;
    }

    void HubEmulation::updateHubProperty(HubPropertyType propId)
    {
        if (!updateHubPropertyEnabled[(uint8_t)propId])
            return;
        sendHubPropertyUpdate(propId);
    }

    void HubEmulation::sendHubPropertyUpdate(HubPropertyType propId)
    {
        if (propId == HubPropertyType::HARDWARE_NETWORK_FAMILY)
        {
            // Real hubs returned erros in my tests.
            std::vector<uint8_t> payload;
            payload.push_back((uint8_t)MessageType::HUB_PROPERTIES);
            payload.push_back((uint8_t)GenericErrorType::CMD_NOT_RECOGNIZED);
            writeValue(MessageType::GENERIC_ERROR_MESSAGES, payload);
        }
        auto &prop = hubProperty[(uint8_t)propId];
        LPF2_LOG_D("Sent prop update: %s", Hub::getHubPropStr(propId, prop).c_str());
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)propId);
        payload.push_back((uint8_t)HubPropertyOperation::UPDATE_UPSTREAM);
        payload.insert(payload.end(), prop.begin(), prop.end());
        writeValue(MessageType::HUB_PROPERTIES, payload);
    }

    void HubEmulation::resetHubProperty(HubPropertyType propId)
    {
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property requested.");
            return;
        }
        auto &prop = hubProperty[(uint8_t)propId];
        // These values were extracted from a real LEGO Technic Hub (except MAC, and rssi)
        switch (propId)
        {
        case HubPropertyType::ADVERTISING_NAME:
        {
            prop.resize(0);
            std::string name = "Technic Hub";
            prop.insert(prop.end(), name.begin(), name.end());
            break;
        }
        case HubPropertyType::BATTERY_TYPE:
        {
            prop.resize(1);
            prop[0] = (uint8_t)BatteryType::NORMAL;
            break;
        }
        case HubPropertyType::BATTERY_VOLTAGE:
        {
            prop.resize(1);
            prop[0] = 100;
            break;
        }
        case HubPropertyType::BUTTON:
        {
            prop.resize(1);
            prop[0] = (uint8_t)ButtonState::RELEASED;
            break;
        }
        case HubPropertyType::FW_VERSION:
        {
            Version version;
            version.Major = 1;
            version.Minor = 2;
            version.Bugfix = 0;
            version.Build = 0;
            prop = Utils::packVersion(version);
            break;
        }
        case HubPropertyType::HARDWARE_NETWORK_FAMILY:
        {
            prop.resize(1);
            prop[0] = 0x00;
            break;
        }
        case HubPropertyType::HW_NETWORK_ID:
        {
            prop.resize(1);
            prop[0] = 0x00;
            break;
        }
        case HubPropertyType::HW_VERSION:
        {
            Version version;
            version.Major = 0;
            version.Minor = 8;
            version.Bugfix = 0;
            version.Build = 0;
            prop = Utils::packVersion(version);
            break;
        }
        case HubPropertyType::LEGO_WIRELESS_PROTOCOL_VERSION:
        {
            prop.resize(2);
            prop[0] = 0x00;
            prop[1] = 0x03;
            break;
        }
        case HubPropertyType::MANUFACTURER_NAME:
        {
            prop.resize(0);
            std::string str = "LEGO System A/S";
            prop.insert(prop.end(), str.begin(), str.end());
            break;
        }
        case HubPropertyType::PRIMARY_MAC_ADDRESS:
        {
            prop.resize(0);
            auto mac = ESP.getEfuseMac();
            prop.push_back((char)((mac >> 40) & 0xFF));
            prop.push_back((char)((mac >> 32) & 0xFF));
            prop.push_back((char)((mac >> 24) & 0xFF));
            prop.push_back((char)((mac >> 16) & 0xFF));
            prop.push_back((char)((mac >> 8) & 0xFF));
            prop.push_back((char)(mac & 0xFF));
            break;
        }
        case HubPropertyType::RADIO_FIRMWARE_VERSION:
        {
            prop.resize(0);
            std::string str = "3_02_00_v1.1";
            prop.insert(prop.end(), str.begin(), str.end());
            break;
        }
        case HubPropertyType::RSSI:
        {
            prop.resize(1);
            prop[0] = 0x63; // Random value
            break;
        }
        case HubPropertyType::SECONDARY_MAC_ADDRESS:
        {
            prop.resize(0);
            auto mac = ESP.getEfuseMac();
            prop.push_back((char)((mac >> 40) & 0xFF));
            prop.push_back((char)((mac >> 32) & 0xFF));
            prop.push_back((char)((mac >> 24) & 0xFF));
            prop.push_back((char)((mac >> 16) & 0xFF) + 0x3A);
            prop.push_back((char)((mac >> 8) & 0xFF));
            prop.push_back((char)(mac & 0xFF));
            break;
        }
        case HubPropertyType::SYSTEM_TYPE_ID:
        {
            prop.resize(1);
            prop[0] = 0x80;
            break;
        }

        default:
            break;
        }
    }

    void HubEmulation::updateHubAlert(HubAlertType alert, bool on)
    {
        if (alert >= HubAlertType::END)
        {
            LPF2_LOG_E("Invalid Hub alert type: %i", (int)alert);
            return;
        }
        hubAlert[(uint8_t)alert] = on;
        if (hubAlertEnabled[(uint8_t)alert])
        {
            sendHubAlertUpdate(alert);
        }
    }

    void HubEmulation::sendHubAlertUpdate(HubAlertType alert)
    {
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)alert);
        payload.push_back((uint8_t)HubAlertOperation::UPDATE_UPSTREAM);
        payload.push_back(hubAlert[(uint8_t)alert] ? 255 : 0);
        writeValue(MessageType::HUB_ALERTS, payload);
    }

    void HubEmulation::resetHubAlerts()
    {
        for (uint8_t i = 0; i < (uint8_t)HubAlertType::END; i++)
        {
            hubAlertEnabled[i] = false;
            hubAlert[i] = false;
        }
    }

    void HubEmulation::handleHubAlertsMessage(std::vector<uint8_t> message)
    {
        if (message.size() < 5)
        {
            LPF2_LOG_E("Unexpected message length: %i", message.size());
            return;
        }
        HubAlertType alertType = (HubAlertType)message[(byte)MessageByte::PROPERTY];
        HubAlertOperation alertOperation = (HubAlertOperation)message[(byte)MessageByte::OPERATION];
        if (alertType >= HubAlertType::END)
        {
            LPF2_LOG_E("Invalid HUB alert type requested.");
            return;
        }
        switch (alertOperation)
        {
        case HubAlertOperation::ENABLE_UPDATES_DOWNSTREAM:
        {
            LPF2_LOG_D("Enable Hub alert: %i", (int)alertType);
            hubAlertEnabled[(uint8_t)alertType] = true;
            break;
        }
        case HubAlertOperation::DISABLE_UPDATES_DOWNSTREAM:
        {
            LPF2_LOG_D("Disable Hub alert: %i", (int)alertType);
            hubAlertEnabled[(uint8_t)alertType] = false;
            break;
        }
        case HubAlertOperation::REQUEST_UPDATE_DOWNSTREAM:
        {
            LPF2_LOG_D("Request Hub alert update: %i", (int)alertType);
            sendHubAlertUpdate(alertType);
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

    void HubEmulation::handlePortInformationRequestMessage(std::vector<uint8_t> message)
    {
        PortNum portNum = (PortNum)message[(uint8_t)MessageByte::PORT_ID];
        if (attachedPorts.find(portNum) == attachedPorts.end())
        {
            LPF2_LOG_W("Port information request for unattached port %d", portNum);
            return;
        }
        Port *port = attachedPorts[portNum];
        DeviceType deviceType = port->getDeviceType();
        uint8_t informationType = message[(uint8_t)MessageByte::OPERATION];

        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)portNum);
        payload.push_back((uint8_t)informationType);

        LPF2_LOG_D("Requested port info: port: 0x%02X, infoType: %i", (uint8_t)portNum, (uint8_t)informationType);

        switch (informationType)
        {
        case 0x01:
        {
            payload.push_back(port->getCapatibilities());
            payload.push_back(port->getModeCount());
            payload.push_back(port->getInputModes() >> 8);
            payload.push_back(port->getInputModes() & 0xF);
            payload.push_back(port->getOutputModes() >> 8);
            payload.push_back(port->getOutputModes() & 0xF);
        }
        break;

        case 0x02:
        {
            for (uint8_t i = 0; i < port->getModeComboCount() && i < 16; i++)
            {
                payload.push_back(port->getModeCombo(i) >> 8);
                payload.push_back(port->getModeCombo(i) & 0xF);
            }
        }
        break;

        default:
            LPF2_LOG_E("Invalid port information request type: %i", informationType);
            return;
        }

        writeValue(MessageType::PORT_INFORMATION, payload);
    }

    void HubEmulation::handlePortModeInformationRequestMessage(std::vector<uint8_t> message)
    {
        PortNum portNum = (PortNum)message[(uint8_t)MessageByte::PORT_ID];
        if (attachedPorts.find(portNum) == attachedPorts.end())
        {
            LPF2_LOG_W("Port information request for unattached port %d", portNum);
            return;
        }
        Port *port = attachedPorts[portNum];
        DeviceType deviceType = port->getDeviceType();
        uint8_t modeNum = message[(uint8_t)MessageByte::OPERATION];
        ModeInfoType modeInfoType = (ModeInfoType)message[(uint8_t)MessageByte::SUB_COMMAND];

        if (modeNum >= port->getModes().size())
        {
            LPF2_LOG_E("Invalid mode number requested: %i", modeNum);
            return;
        }
        LPF2_LOG_D("Requested port mode info: port: 0x%02X, modeNum: %i, modeInfoType: %i", (uint8_t)portNum, (uint8_t)modeNum, (uint8_t)modeInfoType);
        auto &mode = port->getModes()[modeNum];

        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)portNum);
        payload.push_back(modeNum);
        payload.push_back((uint8_t)modeInfoType);

        switch (modeInfoType)
        {
        case ModeInfoType::NAME:
            // Mode name, without null termination
            payload.insert(payload.end(), mode.name.begin(), mode.name.end());
            break;

        case ModeInfoType::RAW:
            payload.resize(3 + 8);
            std::memcpy(&payload[3], &mode.min, 4);
            std::memcpy(&payload[7], &mode.max, 4);
            break;

        case ModeInfoType::PCT:
            payload.resize(3 + 8);
            std::memcpy(&payload[3], &mode.PCTmin, 4);
            std::memcpy(&payload[7], &mode.PCTmax, 4);
            break;

        case ModeInfoType::SI:
            payload.resize(3 + 8);
            std::memcpy(&payload[3], &mode.SImin, 4);
            std::memcpy(&payload[7], &mode.SImax, 4);
            break;

        case ModeInfoType::SYMBOL:
            if (mode.unit.length() == 0)
            {
                payload.push_back('\0');
            }
            else
            {
                payload.insert(payload.end(), mode.unit.begin(), mode.unit.end());
            }

        case ModeInfoType::MAPPING:
            payload.push_back(mode.in.val);
            payload.push_back(mode.out.val);
            break;

        case ModeInfoType::MOTOR_BIAS:
            payload.push_back(0);
            break;

        case ModeInfoType::CAPS:
            payload.push_back(mode.flags.bytes[5]);
            payload.push_back(mode.flags.bytes[4]);
            payload.push_back(mode.flags.bytes[3]);
            payload.push_back(mode.flags.bytes[2]);
            payload.push_back(mode.flags.bytes[1]);
            payload.push_back(mode.flags.bytes[0]);
            break;

        case ModeInfoType::VALUE:
            payload.push_back(mode.data_sets);
            payload.push_back(mode.format);
            payload.push_back(mode.figures);
            payload.push_back(mode.decimals);
            break;

        default:
            LPF2_LOG_E("Invalid Port Mode Info requested.");
            break;
        }

        writeValue(MessageType::PORT_MODE_INFORMATION, payload);
    }

    void HubEmulation::checkPort(PortNum portNum, Port *port)
    {
        if (connectedDevices[portNum] == port->deviceConnected())
            return;

        connectedDevices[portNum] = port->deviceConnected();

        if (connectedDevices[portNum])
        {
            LPF2_LOG_I("Device connected to port %d", portNum);
            std::vector<uint8_t> payload;
            payload.push_back((uint8_t)portNum);
            payload.push_back((uint8_t)IOEvent::ATTACHED_IO);
            payload.push_back((uint8_t)port->getDeviceType());
            payload.push_back(0x00);
            payload.insert(payload.end(), {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10}); // version numbers
            writeValue(MessageType::HUB_ATTACHED_IO, payload);
        }
        else
        {
            LPF2_LOG_I("Device disconnected from port %d", portNum);
            std::vector<uint8_t> payload;
            payload.push_back((char)portNum);
            payload.push_back((char)IOEvent::DETACHED_IO);
            writeValue(MessageType::HUB_ATTACHED_IO, payload);
        }
    }

    void HubEmulation::initBuiltInPorts()
    {
#define INIT_PORT(portNum)                      \
    do                                          \
    {                                           \
        auto port = new Virtual::Port();        \
        attachPort((PortNum)portNum, port);     \
        ownedPorts[(PortNum)portNum] = port;    \
    } while (0)

        switch (m_hubType)
        {
        case HubType::CONTROL_PLUS_HUB:
        {
            INIT_PORT(ControlPlusHubPort::ACCELEROMETER);
            INIT_PORT(ControlPlusHubPort::CURRENT);
            INIT_PORT(ControlPlusHubPort::GESTURE);
            INIT_PORT(ControlPlusHubPort::GYRO);
            INIT_PORT(ControlPlusHubPort::LED);
            INIT_PORT(ControlPlusHubPort::TEMP2);
            INIT_PORT(ControlPlusHubPort::TEMP);
            INIT_PORT(ControlPlusHubPort::TILT);
            INIT_PORT(ControlPlusHubPort::VOLTAGE);
            break;
        }
        default:
            break;
        }

#undef INIT_PORT
    }

    void HubEmulation::initBuiltInDevices()
    {
#define INIT_DEVICE(portNum, desc)                      \
    do                                                  \
    {                                                   \
        auto &port = *(ownedPorts[(PortNum)portNum]);   \
        auto device = new Virtual::GenericDevice(desc); \
        port.attachDevice(device);                      \
        ownedDevices.push_back(device);                 \
    } while (0)

        switch (m_hubType)
        {
        case HubType::CONTROL_PLUS_HUB:
        {
            INIT_DEVICE(ControlPlusHubPort::ACCELEROMETER,
                        DeviceDescriptors::LPF2_DEVICE_TECHNIC_MEDIUM_HUB_ACCELEROMETER);

            INIT_DEVICE(ControlPlusHubPort::CURRENT,
                        DeviceDescriptors::LPF2_DEVICE_CURRENT_SENSOR);

            INIT_DEVICE(ControlPlusHubPort::GESTURE,
                        DeviceDescriptors::LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GEST_SENSOR);

            INIT_DEVICE(ControlPlusHubPort::GYRO,
                        DeviceDescriptors::LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GYRO_SENSOR);

            INIT_DEVICE(ControlPlusHubPort::LED,
                        DeviceDescriptors::LPF2_DEVICE_HUB_LED);

            INIT_DEVICE(ControlPlusHubPort::TEMP2,
                        DeviceDescriptors::LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR);

            INIT_DEVICE(ControlPlusHubPort::TEMP,
                        DeviceDescriptors::LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR);

            INIT_DEVICE(ControlPlusHubPort::TILT,
                        DeviceDescriptors::LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TILT_SENSOR);

            INIT_DEVICE(ControlPlusHubPort::VOLTAGE,
                        DeviceDescriptors::LPF2_DEVICE_VOLTAGE_SENSOR);
            break;
        }
        default:
            break;
        }

#undef INIT_DEVICE
    }

    void HubEmulation::destroyBuiltIn()
    {
        for (auto &port : ownedPorts)
        {
            if (port.second)
            {
                delete port.second;
            }
        }
        ownedPorts.clear();
        for (auto &device : ownedDevices)
        {
            if (device)
            {
                delete device;
                device = nullptr;
            }
        }
        ownedDevices.clear();
    }

    void HubEmulation::handleHubPropertyMessage(std::vector<uint8_t> message)
    {
        if (message.size() < 4)
        {
            LPF2_LOG_E("Unexpected message length: %i", message.size());
            return;
        }
        HubPropertyOperation op = (HubPropertyOperation)message[(byte)MessageByte::OPERATION];
        HubPropertyType propId = (HubPropertyType)message[(byte)MessageByte::PROPERTY];
        if (propId >= HubPropertyType::END)
        {
            LPF2_LOG_E("Invalid HUB property requested.");
            return;
        }
        switch (op)
        {
        case HubPropertyOperation::REQUEST_UPDATE_DOWNSTREAM:
        {
            LPF2_LOG_D("Requested Hub prop update: %i", (int)propId);
            sendHubPropertyUpdate(propId);
            break;
        }
        case HubPropertyOperation::SET_DOWNSTREAM:
        {
            LPF2_LOG_D("Set Hub prop: %i", (int)propId);
            if (message.size() < 5)
            {
                LPF2_LOG_E("Unexpected message length: %i", message.size());
                return;
            }
            std::vector<uint8_t> val;
            val.insert(val.end(), message.begin() + 5, message.end());
            hubProperty[(uint8_t)propId] = val;
            break;
        }
        case HubPropertyOperation::DISABLE_UPDATES_DOWNSTREAM:
        {
            LPF2_LOG_D("Disable Hub prop: %i", (int)propId);
            updateHubPropertyEnabled[(uint8_t)propId] = false;
            break;
        }
        case HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM:
        {
            LPF2_LOG_D("Enable Hub prop: %i", (int)propId);
            updateHubPropertyEnabled[(uint8_t)propId] = true;
            break;
        }
        case HubPropertyOperation::RESET_DOWNSTREAM:
        {
            LPF2_LOG_D("Reset Hub prop: %i", (int)propId);
            resetHubProperty(propId);
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

    HubEmulation::HubEmulation() {};

    HubEmulation::HubEmulation(std::string hubName, HubType hubType)
    {
        setHubName(hubName);
        m_hubType = hubType;
    }

    HubEmulation::~HubEmulation()
    {
        destroyBuiltIn();
    }

    void HubEmulation::reset()
    {
        LPF2_LOG_D("Resetting props.");
        for (uint8_t i = 0; i < (uint8_t)HubPropertyType::END; i++)
        {
            resetHubProperty((HubPropertyType)i);
            updateHubPropertyEnabled[i] = false;
        }
        resetHubAlerts();
    }

    void HubEmulation::attachPort(PortNum portNum, Port *port)
    {
        if (attachedPorts.find(portNum) != attachedPorts.end())
        {
            LPF2_LOG_W("Port %d is already attached, overwriting!", portNum);
        }
        if (!port)
        {
            LPF2_LOG_E("Cannot attach null port to port %d", portNum);
            return;
        }
        connectedDevices[portNum] = false;
        attachedPorts[portNum] = port;
    }

    void HubEmulation::writeValue(MessageType messageType, std::vector<uint8_t> payload)
    {
        if (!isConnected || !pCharacteristic)
            return;
        std::vector<uint8_t> message;
        message.push_back((char)(payload.size() + 3)); // length of message
        message.push_back(0x00);                       // hub id (not used)
        message.push_back((char)messageType);          // message type
        message.insert(message.end(), payload.begin(), payload.end());
        LPF2_DEBUG_EXPR_D(
            std::string hexMessage = Utils::bytes_to_hexString(message);
            LPF2_LOG_D("write message (%d): %s", message.size(), hexMessage.c_str()););

        pCharacteristic->setValue(message);
        pCharacteristic->notify();
    }

    void HubEmulation::setHubButton(bool pressed)
    {
        auto &property = hubProperty[(unsigned)HubPropertyType::BUTTON];
        if (!property.size())
        {
            property.resize(1);
        }
        property[0] = uint8_t(pressed ? ButtonState::PRESSED : ButtonState::RELEASED);
        updateHubProperty(HubPropertyType::BUTTON);
    }

    void HubEmulation::update()
    {
        if (!isSubscribed)
            return;
        if (m_firstUpdate)
        {
            m_firstUpdate = false;
            if (m_useBuiltInDevices)
            {
                initBuiltInDevices();
            }
        }
        std::for_each(attachedPorts.begin(), attachedPorts.end(),
                      [this](auto &pair)
                      {
                          this->checkPort(pair.first, pair.second);
                      });
    }

    void HubEmulation::setUseBuiltInDevices(bool use)
    {
        m_useBuiltInDevices = use;
    }

    void HubEmulation::setHubRssi(int8_t rssi)
    {
        auto &property = hubProperty[(unsigned)HubPropertyType::RSSI];
        if (!property.size())
        {
            property.resize(1);
        }
        property[0] = rssi;
        updateHubProperty(HubPropertyType::RSSI);
    }

    void HubEmulation::setHubBatteryLevel(uint8_t batteryLevel)
    {
        auto &property = hubProperty[(unsigned)HubPropertyType::BATTERY_VOLTAGE];
        if (!property.size())
        {
            property.resize(1);
        }
        property[0] = batteryLevel;
        updateHubProperty(HubPropertyType::BATTERY_VOLTAGE);
    }

    void HubEmulation::setHubBatteryType(BatteryType batteryType)
    {
        auto &property = hubProperty[(unsigned)HubPropertyType::BATTERY_TYPE];
        if (!property.size())
        {
            property.resize(1);
        }
        property[0] = (uint8_t)batteryType;
        updateHubProperty(HubPropertyType::BATTERY_TYPE);
    }

    void HubEmulation::setHubName(std::string hubName)
    {
        if (hubName.length() > 14)
        {
            hubName = hubName.substr(0, 14);
        }

        auto &property = hubProperty[(unsigned)HubPropertyType::ADVERTISING_NAME];
        property.resize(hubName.size());
        property.insert(property.end(), hubName.begin(), hubName.end());
        updateHubProperty(HubPropertyType::ADVERTISING_NAME);
    }

    std::string HubEmulation::getHubName()
    {
        auto &hubName = hubProperty[(unsigned)HubPropertyType::ADVERTISING_NAME];
        std::string str;
        str.insert(str.end(), hubName.begin(), hubName.end());
        return str;
    }

    BatteryType HubEmulation::getBatteryType()
    {
        auto &prop = hubProperty[(unsigned)HubPropertyType::BATTERY_TYPE];
        if (!prop.size())
        {
            prop.push_back((uint8_t)BatteryType::NORMAL);
        }
        return (BatteryType)prop[0];
    }

    void HubEmulation::setHubFirmwareVersion(Version version)
    {
        auto v = Utils::packVersion(version);
        hubProperty[(unsigned)HubPropertyType::FW_VERSION] = v;
    }

    void HubEmulation::setHubHardwareVersion(Version version)
    {
        auto v = Utils::packVersion(version);
        hubProperty[(unsigned)HubPropertyType::HW_VERSION] = v;
    }

    void HubEmulation::start()
    {
        destroyBuiltIn();
        reset();
        if (m_useBuiltInDevices)
        {
            initBuiltInPorts();
        }
        LPF2_LOG_D("Starting BLE");

        NimBLEDevice::init(getHubName());
        NimBLEDevice::setPower(ESP_PWR_LVL_N0, NimBLETxPowerType::Advertise); // 0dB, Advertisment

        LPF2_LOG_D("Create server");
        _pServer = NimBLEDevice::createServer();
        _pServer->setCallbacks(new Lpf2HubServerCallbacks(this));

        LPF2_LOG_D("Create service");
        _pService = _pServer->createService(LPF2_UUID);

        // Create a BLE Characteristic
        pCharacteristic = _pService->createCharacteristic(
            NimBLEUUID(LPF2_CHARACHTERISTIC),
            NIMBLE_PROPERTY::READ |
                NIMBLE_PROPERTY::WRITE |
                NIMBLE_PROPERTY::NOTIFY |
                NIMBLE_PROPERTY::WRITE_NR);
        // Create a BLE Descriptor and set the callback
        pCharacteristic->setCallbacks(new Lpf2HubCharacteristicCallbacks(this));

        LPF2_LOG_D("Service start");

        _pService->start();
        _pAdvertising = NimBLEDevice::getAdvertising();

        _pAdvertising->addServiceUUID(LPF2_UUID);
        _pAdvertising->enableScanResponse(true);
        _pAdvertising->setMinInterval(32); // 0.625ms units -> 20ms
        _pAdvertising->setMaxInterval(64); // 0.625ms units -> 40ms

        std::vector<uint8_t> manufacturerData;

        if (m_hubType == HubType::POWERED_UP_HUB)
        {
            LPF2_LOG_D("PoweredUp Hub");
            manufacturerData = {0x97, 0x03, 0x00, 0x41, 0x07, 0x00, 0x63, 0x00};
        }
        else if (m_hubType == HubType::CONTROL_PLUS_HUB)
        {
            LPF2_LOG_D("ControlPlus Hub");
            manufacturerData = {0x97, 0x03, 0x00, 0x80, 0x06, 0x00, 0x41, 0x00};
        }
        NimBLEAdvertisementData advertisementData = NimBLEAdvertisementData();
        // flags must be present to make PoweredUp working on devices with Android >=6
        // (however it seems to be not needed for devices with Android <6)
        advertisementData.setFlags(BLE_HS_ADV_F_DISC_GEN);
        advertisementData.setCompleteServices(NimBLEUUID(LPF2_UUID));
        advertisementData.setManufacturerData(manufacturerData);

        // scan response data is needed because the uuid128 and manufacturer data takes almost all space in the advertisement data
        // the name is therefore stored in the scan response data
        NimBLEAdvertisementData scanResponseData = NimBLEAdvertisementData();

        // set the slave connection interval range to 20-40ms
        uint8_t slaveConnectionIntervalRangeData[6] = {0x05, 0x12, 0x10, 0x00, 0x20, 0x00};
        scanResponseData.addData(slaveConnectionIntervalRangeData, sizeof(slaveConnectionIntervalRangeData));

        // set the power level to 0dB
        uint8_t powerLevelData[3] = {0x02, 0x0A, 0x00};
        scanResponseData.addData(powerLevelData, sizeof(powerLevelData));

        scanResponseData.setName(getHubName());

        LPF2_LOG_D("advertisment data payload(%d): %s", advertisementData.getPayload().size(), Utils::bytes_to_hexString(advertisementData.getPayload()).c_str());
        LPF2_LOG_D("scan response data payload(%d): %s", scanResponseData.getPayload().size(), Utils::bytes_to_hexString(scanResponseData.getPayload()).c_str());

        _pAdvertising->setAdvertisementData(advertisementData);
        _pAdvertising->setScanResponseData(scanResponseData);

        LPF2_LOG_D("Start advertising");
        NimBLEDevice::startAdvertising();
        LPF2_LOG_D("Characteristic defined! Now you can connect with your PoweredUp App!");
        m_firstUpdate = true;
    }
};