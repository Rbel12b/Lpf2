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
#include "Lpf2/DeviceFactory.hpp"
#include "Lpf2/Port.hpp"
#include <memory>

namespace Lpf2
{
    class DeviceManager
    {
    public:
        explicit DeviceManager(Port &port)
            : m_port(port) {}

        void init() {}

        void update()
        {
            m_port.update();
            if (!m_port.isDeviceConnected())
            {
                m_device.reset(nullptr);
                return;
            }

            if (!m_device && m_port.isDeviceConnected())
            {
                attachViaFactory();
            }

            if (m_device)
            {
                m_device->update();
            }
        }

        Device *device() const
        {
            if (getDeviceType() == DeviceType::UNKNOWNDEVICE)
                return nullptr;
            return m_device.get();
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
                    m_device.reset(factory->create(m_port));
                    m_device->init();
                    break;
                }
            }
        }

        Port &m_port;
        std::unique_ptr<Device> m_device;
    };
}; // namespace Lpf2