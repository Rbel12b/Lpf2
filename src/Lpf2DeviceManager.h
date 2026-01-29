#pragma once
#ifndef _LPF2_DEVICE_MANAGER_H_
#define _LPF2_DEVICE_MANAGER_H_

#include "config.h"
#include "Lpf2Device.h"
#include "Lpf2Port.h"
#include <memory>

class Lpf2DeviceManager
{
public:
    explicit Lpf2DeviceManager(Lpf2Port &port)
        : m_port(port) {}

    void init()
    {
#if defined(LPF2_USE_FREERTOS)
        xTaskCreate(
            &Lpf2DeviceManager::taskEntryPoint, // Static entry point
            "Lpf2DeviceManager",                // Task name
            4096,
            this, // Pass this pointer
            5,
            nullptr);
#endif
    }

    void update()
    {
        m_port.update();
        if (!m_port.deviceConnected())
        {
            m_port.setPower(0, 0);
            device_.reset(nullptr);
            return;
        }

        if (!device_ && m_port.deviceConnected())
        {
            attachViaFactory();
        }

        if (device_)
        {
            device_->poll();
        }
    }

    Lpf2Device *device() const
    {
        if (getDeviceType() == Lpf2DeviceType::UNKNOWNDEVICE)
            return nullptr;
        return device_.get();
    }

    Lpf2DeviceType getDeviceType() const
    {
        return m_port.getDeviceType();
    }

    const Lpf2Port& getPort() const
    {
        return m_port;
    }

private:
    void attachViaFactory()
    {
        auto &reg = Lpf2DeviceRegistry::instance();

        for (size_t i = 0; i < reg.count(); ++i)
        {
            const Lpf2DeviceFactory *factory = reg.factories()[i];

            if (factory->matches(m_port))
            {
                device_.reset(factory->create(m_port));
                device_->init();
                break;
            }
        }
    }
#if defined(LPF2_USE_FREERTOS)
    static void taskEntryPoint(void *pvParameters);
    void loopTask();
#endif

    Lpf2Port& m_port;
    std::unique_ptr<Lpf2Device> device_;
};

#endif
