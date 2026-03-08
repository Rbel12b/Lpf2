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

#include "Lpf2/Device.hpp"
#include "Lpf2/Devices/EncoderMotor.hpp"
#include "Lpf2/Devices/BasicMotor.hpp"
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/DistanceSensor.hpp"

namespace Lpf2
{
    using namespace Lpf2::Devices;
    void DeviceRegistry::registerDefault()
    {
        // Order matters
        EncoderMotor::registerFactory(DeviceRegistry::instance()); // EncoderMotor must be before BasicMotor, because we prefer EncoderMotors over BasicMotors
        BasicMotor::registerFactory(DeviceRegistry::instance());
        // TODO: Add colordistanceSensor here! (above Color and Distance sensors)
        TechnicColorSensor::registerFactory(DeviceRegistry::instance());
        TechnicDistanceSensor::registerFactory(DeviceRegistry::instance());
    }
};