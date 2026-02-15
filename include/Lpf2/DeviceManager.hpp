#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Device.hpp"
#include "Lpf2/Port.hpp"
#include <memory>

namespace Lpf2
{
    class DeviceManager
    {
    public:
        explicit DeviceManager(Port &port)
            : m_port(port) {}

        void init()
        {
#if defined(LPF2_USE_FREERTOS)
            xTaskCreate(
                &DeviceManager::taskEntryPoint, // Static entry point
                "DeviceManager",                // Task name
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

        Device *device() const
        {
            if (getDeviceType() == DeviceType::UNKNOWNDEVICE)
                return nullptr;
            return device_.get();
        }

        DeviceType getDeviceType() const
        {
            return m_port.getDeviceType();
        }

        const Port &getPort() const
        {
            return m_port;
        }

    private:
        void attachViaFactory()
        {
            auto &reg = DeviceRegistry::instance();

            for (size_t i = 0; i < reg.count(); ++i)
            {
                const DeviceFactory *factory = reg.factories()[i];

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

        Port &m_port;
        std::unique_ptr<Device> device_;
    };
}; // namespace Lpf2