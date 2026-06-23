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
#include "Lpf2/Port.hpp"

namespace Lpf2
{
    /**
     * @brief Deprecated thin wrapper around Port's built-in device management.
     *
     * As of v2.3.0, Port owns its device and exposes init()/update()/device()
     * directly. This class remains as a compatibility shim — every call
     * forwards to the wrapped Port. New code should call Port methods
     * directly.
     */
    class [[deprecated("Use Port::init/update/device directly")]] DeviceManager
    {
    public:
        explicit DeviceManager(Port &port)
            : m_port(port) {}

        void init() { m_port.init(); }

        void update()
        {
            m_port.update();
            m_port.manageDevice();
        }

        Device *device() const { return const_cast<Port &>(m_port).device(); }

        DeviceType getDeviceType() const { return m_port.getDeviceType(); }

        const Port &getPort() const { return m_port; }

#if defined(LPF2_USE_FREERTOS)
        static void taskEntryPoint(void *pvParameters);
        void loopTask();
#endif

    private:
        Port &m_port;
    };
}; // namespace Lpf2
