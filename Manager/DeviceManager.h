#pragma once
#ifndef _LPF2_DEVICE_MANAGER_H_
#define _LPF2_DEVICE_MANAGER_H_

#include "../config.h"
#include "../Lpf2Device.h"
#include "../Lpf2Port.h"
#include <memory>

class Lpf2DeviceManager
{
public:
    explicit Lpf2DeviceManager(Lpf2IO *io)
        : port_(io), io(io) {}

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
        if (!io->ready())
        {
            return;
        }
        if (!inited)
        {
            #if defined(LPF2_USE_FREERTOS)
            port_.init(false);
            #else
            port_.init();
            #endif
            inited = true;
        }
    }

    void update()
    {
        if (!io->ready())
        {
            return;
        }
        if (!inited)
        {
            #if defined(LPF2_USE_FREERTOS)
            port_.init(false);
            #else
            port_.init();
            #endif
            inited = true;
        }
        port_.update();
        if (!port_.deviceConnected())
        {
            device_.reset();
            return;
        }

        if (!device_ && port_.deviceConnected())
        {
            attachViaFactory();
        }

        if (device_)
        {
            device_->poll();
        }
    }

    Lpf2Device *device()
    {
        if (getDeviceType() == Lpf2DeviceType::UNKNOWNDEVICE)
            return nullptr;
        return device_.get();
    }

    Lpf2DeviceType getDeviceType()
    {
        return port_.getDeviceType();
    }

private:
    void attachViaFactory()
    {
        auto &reg = Lpf2DeviceRegistry::instance();

        for (size_t i = 0; i < reg.count(); ++i)
        {
            const Lpf2DeviceFactory *factory = reg.factories()[i];

            if (factory->matches(port_))
            {
                device_.reset(factory->create(port_));
                device_->init();
                break;
            }
        }
    }
#if defined(LPF2_USE_FREERTOS)
    static void taskEntryPoint(void *pvParameters);
    void loopTask();
#endif

    Lpf2Port port_;
    Lpf2IO *io;
    bool inited = false;
    std::unique_ptr<Lpf2Device> device_;
};

#endif
