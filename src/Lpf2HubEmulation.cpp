#include "./log/log.h"
#include "Lpf2Hub.h"
#include "Lpf2HubEmulation.h"
#include "Util/hexUtils.h"
#include "Lpf2Port.h"

class Lpf2HubServerCallbacks : public NimBLEServerCallbacks
{

    Lpf2HubEmulation *_lpf2HubEmulation;

public:
    Lpf2HubServerCallbacks(Lpf2HubEmulation *lpf2HubEmulation) : NimBLEServerCallbacks()
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
    Lpf2HubEmulation *_lpf2HubEmulation;

public:
    Lpf2HubCharacteristicCallbacks(Lpf2HubEmulation *lpf2HubEmulation) : NimBLECharacteristicCallbacks()
    {
        _lpf2HubEmulation = lpf2HubEmulation;
    }

    void onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, uint16_t subValue) override
    {
        LPF2_LOG_D("Client subscription status: %s (%d)",
                   subValue == 0 ? "Un-Subscribed" : subValue == 1 ? "Notifications"
                                                 : subValue == 2   ? "Indications"
                                                 : subValue == 3   ? "Notifications and Indications"
                                                                   : "unknown subscription dstatus",
                   subValue);

        _lpf2HubEmulation->isSubscribed = subValue != 0;
    }

    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        std::vector<uint8_t> msgReceived = pCharacteristic->getValue();

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

void Lpf2HubEmulation::onMessageReceived(std::vector<uint8_t> message)
{
    Lpf2MessageType type = (Lpf2MessageType)message[(byte)Lpf2MessageHeader::MESSAGE_TYPE];
    LPF2_DEBUG_EXPR_D(
        std::string hexMessage = bytes_to_hexString(message);
        LPF2_LOG_D("message received (%d): %s", message.size(), hexMessage.c_str()););
    LPF2_LOG_D("message type: %d", (byte)type);

    switch (type)
    {
    case Lpf2MessageType::HUB_PROPERTIES:
        handleHubPropertyMessage(message);
        break;
    case Lpf2MessageType::HUB_ACTIONS:
        LPF2_LOG_W("HUB_ACTIONS not implemented yet");
        break;
    case Lpf2MessageType::HUB_ALERTS:
        handleHubAlertsMessage(message);
        break;
    case Lpf2MessageType::HW_NETWORK_COMMANDS:
        LPF2_LOG_W("HW_NETWORK_COMMANDSnot implemented yet");
        break;
    case Lpf2MessageType::FW_UPDATE_GO_INTO_BOOT_MODE:
        LPF2_LOG_W("FW_UPDATE_GO_INTO_BOOT_MODE not implemented yet");
        break;
    case Lpf2MessageType::FW_UPDATE_LOCK_MEMORY:
        LPF2_LOG_W("FW_UPDATE_LOCK_MEMORY not implemented yet");
        break;
    case Lpf2MessageType::FW_UPDATE_LOCK_STATUS_REQUEST:
        LPF2_LOG_W("FW_UPDATE_LOCK_STATUS_REQUEST not implemented yet");
        break;
    case Lpf2MessageType::PORT_INFORMATION_REQUEST:
        handlePortInformationRequestMessage(message);
        break;
    case Lpf2MessageType::PORT_MODE_INFORMATION_REQUEST:
        handlePortModeInformationRequestMessage(message);
        break;
    case Lpf2MessageType::PORT_INPUT_FORMAT_SETUP_COMBINEDMODE:
        LPF2_LOG_W("PORT_INPUT_FORMAT_SETUP_COMBINEDMODE not implemented yet");
        break;
    case Lpf2MessageType::VIRTUAL_PORT_SETUP:
        LPF2_LOG_W("VIRTUAL_PORT_SETUP not implemented yet");
        break;
    case Lpf2MessageType::PORT_OUTPUT_COMMAND:
        LPF2_LOG_W("PORT_OUTPUT_COMMAND not implemented yet");
        break;

    default:
        goto unimplemented;
    }

    //   // It's a port out command:
    //   // execute and send feedback to the App
    //   if (msgReceived[(byte)Lpf2MessageHeader::MESSAGE_TYPE] == (char)MessageType::PORT_OUTPUT_COMMAND)
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

    //   if (msgReceived[(byte)Lpf2MessageHeader::MESSAGE_TYPE] == (byte)MessageType::HUB_ACTIONS && msgReceived[3] == (byte)ActionType::SWITCH_OFF_HUB)
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

void Lpf2HubEmulation::updateHubProperty(Lpf2HubPropertyReference propRef)
{
    if (!updateHubPropertyEnabled[(uint8_t)propRef])
        return;
    sendHubPropertyUpdate(propRef);
}

void Lpf2HubEmulation::sendHubPropertyUpdate(Lpf2HubPropertyReference propRef)
{
    auto &prop = hubProperty[(uint8_t)propRef];
    std::vector<uint8_t> payload;
    payload.push_back((uint8_t)propRef);
    payload.push_back((uint8_t)Lpf2HubPropertyOperation::UPDATE_UPSTREAM);
    payload.insert(payload.end(), prop.begin(), prop.end());
    writeValue(Lpf2MessageType::HUB_PROPERTIES, payload);
}

void Lpf2HubEmulation::resetHubProperty(Lpf2HubPropertyReference propRef)
{
    if (propRef >= Lpf2HubPropertyReference::END)
    {
        LPF2_LOG_E("Invalid HUB property requested.");
        return;
    }
    auto &prop = hubProperty[(uint8_t)propRef];
    switch (propRef)
    {
    case Lpf2HubPropertyReference::ADVERTISING_NAME:
    {
        prop.resize(0);
        std::string name = "Hub";
        prop.insert(prop.end(), name.begin(), name.end());
    }
    case Lpf2HubPropertyReference::BATTERY_TYPE:
    {
        prop.resize(1);
        prop[0] = (uint8_t)Lpf2BatteryType::NORMAL;
    }
    case Lpf2HubPropertyReference::BATTERY_VOLTAGE:
    {
        prop.resize(1);
        prop[0] = 0x64;
    }
    case Lpf2HubPropertyReference::BUTTON:
    {
        prop.resize(1);
        prop[0] = (uint8_t)Lpf2ButtonState::RELEASED;
    }
    case Lpf2HubPropertyReference::FW_VERSION:
    {
        Lpf2Version version;
        version.Major = 0;
        version.Minor = 0;
        version.Bugfix = 0;
        version.Build = 529;
        prop = packVersion(version);
    }
    case Lpf2HubPropertyReference::HARDWARE_NETWORK_FAMILY:
    {
        prop.resize(1);
        prop[0] = 0x00;
    }
    case Lpf2HubPropertyReference::HW_NETWORK_ID:
    {
        prop.resize(1);
        prop[0] = 0x01;
    }
    case Lpf2HubPropertyReference::HW_VERSION:
    {
        Lpf2Version version;
        version.Major = 0;
        version.Minor = 0;
        version.Bugfix = 0;
        version.Build = 1;
        prop = packVersion(version);
    }
    case Lpf2HubPropertyReference::LEGO_WIRELESS_PROTOCOL_VERSION:
    {
        prop.resize(2);
        prop[0] = 0x00;
        prop[1] = 0x03;
    }
    case Lpf2HubPropertyReference::MANUFACTURER_NAME:
    {
        prop.resize(0);
        std::string str = "LEGO System A/S";
        prop.insert(prop.end(), str.begin(), str.end());
    }
    case Lpf2HubPropertyReference::PRIMARY_MAC_ADDRESS:
    {
        prop.resize(0);
        auto mac = ESP.getEfuseMac();
        prop.push_back((char)((mac >> 40) & 0xFF));
        prop.push_back((char)((mac >> 32) & 0xFF));
        prop.push_back((char)((mac >> 24) & 0xFF));
        prop.push_back((char)((mac >> 16) & 0xFF));
        prop.push_back((char)((mac >> 8) & 0xFF));
        prop.push_back((char)(mac & 0xFF));
    }
    case Lpf2HubPropertyReference::RADIO_FIRMWARE_VERSION:
    {
        prop.resize(0);
        std::string str = "2_02_01";
        prop.insert(prop.end(), str.begin(), str.end());
    }
    case Lpf2HubPropertyReference::RSSI:
    {
        prop.resize(1);
        prop[0] = 0xC8;
    }
    case Lpf2HubPropertyReference::SECONDARY_MAC_ADDRESS:
    {
        prop.resize(0);
        auto mac = ESP.getEfuseMac();
        prop.push_back((char)((mac >> 40) & 0xFF));
        prop.push_back((char)((mac >> 32) & 0xFF));
        prop.push_back((char)((mac >> 24) & 0xFF));
        prop.push_back((char)((mac >> 16) & 0xFF));
        prop.push_back((char)((mac >> 8) & 0xFF));
        prop.push_back((char)(mac & 0xFF));
    }
    case Lpf2HubPropertyReference::SYSTEM_TYPE_ID:
    {
        prop.resize(1);
        prop[0] = 0x00;
    }
    break;

    default:
        break;
    }
}

void Lpf2HubEmulation::updateHubAlert(Lpf2HubAlertType alert, bool on)
{
    if (alert >= Lpf2HubAlertType::END)
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

void Lpf2HubEmulation::sendHubAlertUpdate(Lpf2HubAlertType alert)
{
    std::vector<uint8_t> payload;
    payload.push_back((uint8_t)alert);
    payload.push_back((uint8_t)Lpf2HubAlertOperation::UPDATE_UPSTREAM);
    payload.push_back(hubAlert[(uint8_t)alert] ? 255 : 0);
    writeValue(Lpf2MessageType::HUB_ALERTS, payload);
}

void Lpf2HubEmulation::resetHubAlerts()
{
    for (uint8_t i = 0; i < (uint8_t)Lpf2HubAlertType::END; i++)
    {
        hubAlertEnabled[i] = false;
        hubAlert[i] = false;
    }
}

void Lpf2HubEmulation::handleHubAlertsMessage(std::vector<uint8_t> message)
{
    if (message.size() < 5)
    {
        LPF2_LOG_E("Unexpected message length: %i", message.size());
        return;
    }
    Lpf2HubAlertType alertType = (Lpf2HubAlertType)message[(byte)Lpf2MessageByte::PROPERTY];
    Lpf2HubAlertOperation alertOperation = (Lpf2HubAlertOperation)message[(byte)Lpf2MessageByte::OPERATION];
    if (alertType >= Lpf2HubAlertType::END)
    {
        LPF2_LOG_E("Invalid HUB alert type requested.");
        return;
    }
    switch (alertOperation)
    {
    case Lpf2HubAlertOperation::ENABLE_UPDATES_DOWNSTREAM:
    {
        hubAlertEnabled[(uint8_t)alertType] = true;
        break;
    }
    case Lpf2HubAlertOperation::DISABLE_UPDATES_DOWNSTREAM:
    {
        hubAlertEnabled[(uint8_t)alertType] = false;
        break;
    }
    case Lpf2HubAlertOperation::REQUEST_UPDATE_DOWNSTREAM:
    {
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

void Lpf2HubEmulation::handlePortInformationRequestMessage(std::vector<uint8_t> message)
{
    Lpf2PortNum portNum = (Lpf2PortNum)message[(uint8_t)Lpf2MessageByte::PORT_ID];
    if (attachedPorts.find(portNum) == attachedPorts.end())
    {
        LPF2_LOG_W("Port information request for unattached port %d", portNum);
        return;
    }
    Lpf2Port* port = attachedPorts[portNum];
    Lpf2DeviceType deviceType = port->getDeviceType();
    uint8_t informationType = message[(uint8_t)Lpf2MessageByte::OPERATION];

    std::vector<uint8_t> payload;
    payload.push_back((uint8_t)portNum);
    payload.push_back((uint8_t)informationType);

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

    writeValue(Lpf2MessageType::PORT_INFORMATION, payload);
}

void Lpf2HubEmulation::handlePortModeInformationRequestMessage(std::vector<uint8_t> message)
{
    Lpf2PortNum portNum = (Lpf2PortNum)message[(uint8_t)Lpf2MessageByte::PORT_ID];
    if (attachedPorts.find(portNum) == attachedPorts.end())
    {
        LPF2_LOG_W("Port information request for unattached port %d", portNum);
        return;
    }
    Lpf2Port* port = attachedPorts[portNum];
    Lpf2DeviceType deviceType = port->getDeviceType();
    uint8_t modeNum = message[(uint8_t)Lpf2MessageByte::OPERATION];
    Lpf2ModeInfoType modeInfoType = (Lpf2ModeInfoType)message[(uint8_t)Lpf2MessageByte::SUB_COMMAND];

    if (modeNum >= port->getModes().size())
    {
        LPF2_LOG_E("Invalid mode number requested: %i", modeNum);
        return;
    }
    auto &mode = port->getModes()[modeNum];

    std::vector<uint8_t> payload;
    payload.push_back((uint8_t)portNum);
    payload.push_back(modeNum);
    payload.push_back((uint8_t)modeInfoType);

    switch (modeInfoType)
    {
    case Lpf2ModeInfoType::NAME:
        // Mode name, without null termination
        payload.insert(payload.end(), mode.name.begin(), mode.name.end());
        break;

    case Lpf2ModeInfoType::RAW:
        payload.resize(3 + 8);
        std::memcpy(&payload[3], &mode.min, 4);
        std::memcpy(&payload[7], &mode.max, 4);
        break;

    case Lpf2ModeInfoType::PCT:
        payload.resize(3 + 8);
        std::memcpy(&payload[3], &mode.PCTmin, 4);
        std::memcpy(&payload[7], &mode.PCTmax, 4);
        break;

    case Lpf2ModeInfoType::SI:
        payload.resize(3 + 8);
        std::memcpy(&payload[3], &mode.SImin, 4);
        std::memcpy(&payload[7], &mode.SImax, 4);
        break;

    case Lpf2ModeInfoType::SYMBOL:
        if (mode.unit.length() == 0)
        {
            payload.push_back('\0');
        }
        else
        {
            payload.insert(payload.end(), mode.unit.begin(), mode.unit.end());
        }

    case Lpf2ModeInfoType::MAPPING:
        payload.push_back(mode.in.val);
        payload.push_back(mode.out.val);
        break;

    case Lpf2ModeInfoType::MOTOR_BIAS:
        payload.push_back(0);
        break;

    case Lpf2ModeInfoType::CAPS:
        payload.push_back(mode.flags.bytes[5]);
        payload.push_back(mode.flags.bytes[4]);
        payload.push_back(mode.flags.bytes[3]);
        payload.push_back(mode.flags.bytes[2]);
        payload.push_back(mode.flags.bytes[1]);
        payload.push_back(mode.flags.bytes[0]);
        break;

    case Lpf2ModeInfoType::VALUE:
        payload.push_back(mode.data_sets);
        payload.push_back(mode.format);
        payload.push_back(mode.figures);
        payload.push_back(mode.decimals);
        break;

    default:
        LPF2_LOG_E("Invalid Port Mode Info requested.");
        break;
    }

    writeValue(Lpf2MessageType::PORT_MODE_INFORMATION, payload);
}

std::vector<uint8_t> Lpf2HubEmulation::packVersion(Lpf2Version version)
{
    std::vector<uint8_t> v;
    v.push_back(version.Build);
    v.push_back(version.Build >> 8);
    v.push_back(version.Bugfix);
    v.push_back(version.Major << 4 | version.Minor);
    return v;
}

Lpf2Version Lpf2HubEmulation::unPackVersion(std::vector<uint8_t> version)
{
    if (version.size() < 4)
        return Lpf2Version();

    Lpf2Version v;
    v.Build = version[0] | (version[1] << 8);
    v.Bugfix = version[2];
    v.Minor = version[3] & 0x0F;
    v.Major = (version[3] >> 4) & 0x0F;
    return v;
}

void Lpf2HubEmulation::checkPort(Lpf2Port *port)
{
    Lpf2PortNum portNum = port->portNum;
    if (connectedDevices[portNum] == port->deviceConnected())
        return;
    connectedDevices[portNum] = port->deviceConnected();

    if (connectedDevices[portNum])
    {
        LPF2_LOG_I("Device connected to port %d", portNum);
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)portNum);
        payload.push_back((uint8_t)Lpf2Event::ATTACHED_IO);
        payload.push_back((uint8_t)port->getDeviceType());
        payload.push_back(0x00);
        payload.insert(payload.end(), {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10}); // version numbers
        writeValue(Lpf2MessageType::HUB_ATTACHED_IO, payload);
    }
    else
    {
        LPF2_LOG_I("Device disconnected from port %d", portNum);
        std::vector<uint8_t> payload;
        payload.push_back((char)portNum);
        payload.push_back((char)Lpf2Event::DETACHED_IO);
        writeValue(Lpf2MessageType::HUB_ATTACHED_IO, payload);
    }
}

void Lpf2HubEmulation::handleHubPropertyMessage(std::vector<uint8_t> message)
{
    if (message.size() < 5)
    {
        LPF2_LOG_E("Unexpected message length: %i", message.size());
        return;
    }
    Lpf2HubPropertyOperation op = (Lpf2HubPropertyOperation)message[(byte)Lpf2MessageByte::OPERATION];
    Lpf2HubPropertyReference propId = (Lpf2HubPropertyReference)message[(byte)Lpf2MessageByte::PROPERTY];
    if (propId >= Lpf2HubPropertyReference::END)
    {
        LPF2_LOG_E("Invalid HUB property requested.");
        return;
    }
    switch (op)
    {
    case Lpf2HubPropertyOperation::REQUEST_UPDATE_DOWNSTREAM:
    {
        sendHubPropertyUpdate(propId);
        break;
    }
    case Lpf2HubPropertyOperation::SET_DOWNSTREAM:
    {
        std::vector<uint8_t> val;
        val.insert(val.end(), message.begin() + 5, message.end()); // Okay, since we checked the lenght previously.
        hubProperty[(uint8_t)propId] = val;
        break;
    }
    case Lpf2HubPropertyOperation::DISABLE_UPDATES_DOWNSTREAM:
    {
        updateHubPropertyEnabled[(uint8_t)propId] = false;
        break;
    }
    case Lpf2HubPropertyOperation::ENABLE_UPDATES_DOWNSTREAM:
    {
        updateHubPropertyEnabled[(uint8_t)propId] = true;
        break;
    }
    case Lpf2HubPropertyOperation::RESET_DOWNSTREAM:
    {
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

Lpf2HubEmulation::Lpf2HubEmulation() {};

Lpf2HubEmulation::Lpf2HubEmulation(std::string hubName, Lpf2HubType hubType)
{
    setHubName(hubName);
    _hubType = hubType;
}

void Lpf2HubEmulation::reset()
{
    LPF2_LOG_D("Resetting props.");
    for (uint8_t i = 0; i < (uint8_t)Lpf2HubPropertyReference::END; i++)
    {
        resetHubProperty((Lpf2HubPropertyReference)i);
        updateHubPropertyEnabled[i] = false;
    }
    resetHubAlerts();
}

void Lpf2HubEmulation::attachPort(Lpf2PortNum portNum, Lpf2Port *port)
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
    port->portNum = portNum;
}

void Lpf2HubEmulation::writeValue(Lpf2MessageType messageType, std::vector<uint8_t> payload, bool notify)
{
    if (!isConnected || !pCharacteristic)
        return;
    std::vector<uint8_t> message;
    message.push_back((char)(payload.size() + 3)); // length of message
    message.push_back(0x00);                       // hub id (not used)
    message.push_back((char)messageType);          // message type
    message.insert(message.end(), payload.begin(), payload.end());
    pCharacteristic->setValue(message);

    if (notify)
    {
        pCharacteristic->notify();
    }

    LPF2_DEBUG_EXPR_D(
        std::string hexMessage = bytes_to_hexString(message);
        LPF2_LOG_D("write message (%d): %s", message.size(), hexMessage.c_str()););
}

void Lpf2HubEmulation::setHubButton(bool pressed)
{
    auto &property = hubProperty[(unsigned)Lpf2HubPropertyReference::BUTTON];
    if (!property.size())
    {
        property.resize(1);
    }
    property[0] = uint8_t(pressed ? Lpf2ButtonState::PRESSED : Lpf2ButtonState::RELEASED);
    updateHubProperty(Lpf2HubPropertyReference::BUTTON);
}

void Lpf2HubEmulation::update()
{
    if (!isSubscribed)
        return;
    std::for_each(attachedPorts.begin(), attachedPorts.end(),
                  [this](auto &pair)
                  {
                      this->checkPort(pair.second);
                  });
}

void Lpf2HubEmulation::setHubRssi(int8_t rssi)
{
    auto &property = hubProperty[(unsigned)Lpf2HubPropertyReference::RSSI];
    if (!property.size())
    {
        property.resize(1);
    }
    property[0] = rssi;
    updateHubProperty(Lpf2HubPropertyReference::RSSI);
}

void Lpf2HubEmulation::setHubBatteryLevel(uint8_t batteryLevel)
{
    auto &property = hubProperty[(unsigned)Lpf2HubPropertyReference::BATTERY_VOLTAGE];
    if (!property.size())
    {
        property.resize(1);
    }
    property[0] = batteryLevel;
    updateHubProperty(Lpf2HubPropertyReference::BATTERY_VOLTAGE);
}

void Lpf2HubEmulation::setHubBatteryType(Lpf2BatteryType batteryType)
{
    auto &property = hubProperty[(unsigned)Lpf2HubPropertyReference::BATTERY_TYPE];
    if (!property.size())
    {
        property.resize(1);
    }
    property[0] = (uint8_t)batteryType;
    updateHubProperty(Lpf2HubPropertyReference::BATTERY_TYPE);
}

void Lpf2HubEmulation::setHubName(std::string hubName)
{
    if (hubName.length() > 14)
    {
        hubName = hubName.substr(0, 14);
    }

    auto &property = hubProperty[(unsigned)Lpf2HubPropertyReference::ADVERTISING_NAME];
    property.resize(hubName.size());
    property.insert(property.end(), hubName.begin(), hubName.end());
    updateHubProperty(Lpf2HubPropertyReference::ADVERTISING_NAME);
}

std::string Lpf2HubEmulation::getHubName()
{
    auto &hubName = hubProperty[(unsigned)Lpf2HubPropertyReference::ADVERTISING_NAME];
    std::string str;
    str.insert(str.end(), hubName.begin(), hubName.end());
    return str;
}

Lpf2BatteryType Lpf2HubEmulation::getBatteryType()
{
    auto &prop = hubProperty[(unsigned)Lpf2HubPropertyReference::BATTERY_TYPE];
    if (!prop.size())
    {
        prop.push_back((uint8_t)Lpf2BatteryType::NORMAL);
    }
    return (Lpf2BatteryType)prop[0];
}

void Lpf2HubEmulation::setHubFirmwareVersion(Lpf2Version version)
{
    auto v = packVersion(version);
    hubProperty[(unsigned)Lpf2HubPropertyReference::FW_VERSION] = v;
}

void Lpf2HubEmulation::setHubHardwareVersion(Lpf2Version version)
{
    auto v = packVersion(version);
    hubProperty[(unsigned)Lpf2HubPropertyReference::HW_VERSION] = v;
}

void Lpf2HubEmulation::start()
{
    reset();
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

    std::string manufacturerData;
    if (_hubType == Lpf2HubType::POWERED_UP_HUB)
    {
        LPF2_LOG_D("PoweredUp Hub");
        // this is the minimal change that makes PoweredUp working on devices with Android <6
        // https://lego.github.io/lego-ble-wireless-protocol-docs/index.html#last-network-id
        // set Last Network ID to UNKNOWN (0x00)
        const char poweredUpHub[8] = {0x97, 0x03, 0x00, 0x41, 0x07, 0x00, 0x63, 0x00};
        manufacturerData = std::string(poweredUpHub, sizeof(poweredUpHub));
    }
    else if (_hubType == Lpf2HubType::CONTROL_PLUS_HUB)
    {
        LPF2_LOG_D("ControlPlus Hub");
        const char controlPlusHub[8] = {0x97, 0x03, 0x00, 0x80, 0x06, 0x00, 0x41, 0x00};
        manufacturerData = std::string(controlPlusHub, sizeof(controlPlusHub));
    }
    NimBLEAdvertisementData advertisementData = NimBLEAdvertisementData();
    // flags must be present to make PoweredUp working on devices with Android >=6
    // (however it seems to be not needed for devices with Android <6)
    advertisementData.setFlags(BLE_HS_ADV_F_DISC_GEN);
    advertisementData.setManufacturerData(manufacturerData);
    advertisementData.setCompleteServices(NimBLEUUID(LPF2_UUID));
    // scan response data is needed because the uuid128 and manufacturer data takes almost all space in the advertisement data
    // the name is therefore stored in the scan response data
    NimBLEAdvertisementData scanResponseData = NimBLEAdvertisementData();
    scanResponseData.setName(getHubName());
    // set the advertisment flags to 0x06
    scanResponseData.setFlags(BLE_HS_ADV_F_DISC_GEN);
    // set the power level to 0dB
    uint8_t powerLevelData[3] = {0x02, 0x0A, 0x00};
    scanResponseData.addData(powerLevelData, sizeof(powerLevelData));
    // set the slave connection interval range to 20-40ms
    uint8_t slaveConnectionIntervalRangeData[6] = {0x05, 0x12, 0x10, 0x00, 0x20, 0x00};
    scanResponseData.addData(slaveConnectionIntervalRangeData, sizeof(slaveConnectionIntervalRangeData));

    LPF2_LOG_D("advertisment data payload(%d): %s", advertisementData.getPayload().size(), bytes_to_hexString(advertisementData.getPayload()).c_str());
    LPF2_LOG_D("scan response data payload(%d): %s", scanResponseData.getPayload().size(), bytes_to_hexString(scanResponseData.getPayload()).c_str());

    _pAdvertising->setAdvertisementData(advertisementData);
    _pAdvertising->setScanResponseData(scanResponseData);

    LPF2_LOG_D("Start advertising");
    NimBLEDevice::startAdvertising();
    LPF2_LOG_D("Characteristic defined! Now you can connect with your PoweredUp App!");
}
