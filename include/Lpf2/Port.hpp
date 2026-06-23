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
#include "Lpf2/LWPConst.hpp"
#include "Lpf2/Device.hpp"
#include "Lpf2/DeviceDesc.hpp"
#include <memory>

namespace Lpf2
{
    /**
     * @brief Indirect handle to a Port-owned Device.
     *
     * Single allocation per Port, shared with any consumer (e.g. MicroPython
     * binding wrappers). On every device swap, `gen` is incremented so
     * wrappers that cached the previous generation can detect the change and
     * refuse to dereference the (now-deleted) old device.
     */
    struct DeviceSlot
    {
        Device *ptr = nullptr;
        uint32_t gen = 0;
    };

    class Port
    {
    public:
        virtual ~Port() = default;

        /**
         * @brief Poll the port.
         *
         * Non-virtual. Calls the subclass-implemented `_update()` (hardware /
         * transport poll), then `manageDevice()` (factory resolution + child
         * device update). Callers loop on this; do not call `_update()` or
         * `manageDevice()` separately.
         */
        void update();

        /**
         * @brief Optional one-shot initialisation hook.
         * Default no-op; overridden by ports that need setup before first update().
         */
        virtual void init() {}

        /**
         * @brief Run device-factory resolution + child device update.
         * Detaches old device on disconnect/type change, constructs the new
         * typed Device via DeviceRegistry, and forwards update() to it.
         * Called from update(); rarely needs to be called directly.
         */
        void manageDevice();

        /**
         * @brief Current device handle (shared with consumers).
         * Returns an empty shared_ptr if no device has ever attached;
         * the slot stays valid across swaps, but its `ptr` becomes null.
         */
        std::shared_ptr<DeviceSlot> deviceHandle() const { return m_slot; }

        /**
         * @brief Current device, attaching one via factory if needed.
         *
         * If the port reports a connected device but no Device has been
         * constructed yet, this runs `manageDevice()` to resolve a factory
         * and attach the typed wrapper, then returns it. Returns nullptr
         * when nothing is connected.
         */
        Device *device();

        virtual int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) = 0;

        /**
         * @brief set motor power
         * @param pw motor power:
         * -100 - 0 -> CCW, 0 - 100 -> CW
         */
        virtual void startPower(int8_t pw) = 0;

        /**
         * @brief set acceleration time/profile for a motor
         * @param accTime acceleration time in ms
         * @param accProfile acceleration profile to use, ???, use 1
         */
        virtual void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) = 0;

        /**
         * @brief set deceleration time/profile for a motor
         * @param decTime acceleration time in ms
         * @param decProfile deceleration profile to use, ???, use 1
         */
        virtual void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) = 0;

        /**
         * @brief start motor with speed
         * @param speed speed:
         * -100 - 0 -> CCW, 0 - 100 -> CW
         * @param maxPower max motor power, absolute value, 0-100%
         * @param useProfile
            0x0000000? -> Acc.- profile
            0x000000?0 -> Decc.- profile
         */
        virtual void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile = 0) = 0;

        /**
         * @brief start motor with speed for [time] ms
         * @param time time in ms
         * @param speed speed: -100 - 0 -> CCW, 0 - 100 -> CW
         * @param maxPower max motor power
         * @param endState what happens after command is finished
         * @param useProfile
            0x0000000? -> Acc.- profile
            0x000000?0 -> Decc.- profile
         */
        virtual void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) = 0;

        /**
         * @brief start motor with speed for [degrees]°
         * @param degrees degrees to move (positive value, use speed for CCW movement)
         * @param speed speed: -100 - 0 -> CCW, 0 - 100 -> CW
         * @param maxPower max motor power
         * @param endState what happens after command is finished
         * @param useProfile
            0x0000000? -> Acc.- profile
            0x000000?0 -> Decc.- profile
         */
        virtual void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) = 0;

        /**
         * @brief got to absolute position (motor)
         * @param absPos absolute position
         * @param speed speed: 0 - 100
         * @param maxPower max motor power
         * @param endState what happens after command is finished
         * @param useProfile
            0x0000000? -> Acc.- profile
            0x000000?0 -> Decc.- profile
         */
        virtual void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) = 0;

        /**
         * @brief preset encoder, also stops motors
         * @param pos position to set the encoder to
         */
        virtual void presetEncoder(int32_t pos) = 0;

        /**
         * @brief uses writeData with mode 0 to set the color idx
         * (works with an rgb led - hub led)
         */
        void setRgbColorIdx(ColorIDX idx);

        /**
         * @brief uses writeData with mode 1 to set the color
         * (works with an rgb led - hub led)
         */
        void setRgbColor(uint8_t r, uint8_t g, uint8_t b);

        /**
         * @brief Select the mode of the connected device,
         * has no effect when isDeviceConnected() == false
         * @param mode mode number
         * @param delta minimum change threshold for the value-change callback (1 = fire on any int-unit change)
         * @returns 0 if succesful
         */
        virtual int setMode(uint8_t mode, float delta = 1.0f) = 0;

        /**
         * @brief Set the mode combination of the connected device,
         * has no effect when isDeviceConnected() == false
         * @param idx the mode combo index (from getModeCombos())
         * @param deltas per-mode delta thresholds (indexed by position in combo bitmask); empty uses default of 1
         * @returns 0 if succesful
         */
        virtual int setModeCombo(uint8_t idx, const std::vector<float>& deltas = {}) = 0;

        virtual bool isDeviceConnected() = 0;

        static float getValue(const Mode &modeData, const std::vector<uint8_t> &raw, uint8_t dataSet);
        static float getValue(const Mode &modeData, uint8_t dataSet);
        float getValue(uint8_t modeNum, const std::vector<uint8_t> &raw, uint8_t dataSet) const;
        float getValue(uint8_t modeNum, uint8_t dataSet) const;
        static std::string formatValue(float value, const Mode &modeData);
        static std::string getValueStr(const Mode &modeData);
        std::string getValueStr(uint8_t modeNum) const;

        DeviceType getDeviceType() const { return m_deviceType; }
        uint8_t getModeCount() const { return m_modeData.size(); }
        uint8_t getViewCount() const { return m_viewCount; }
        const std::vector<Mode> &getModes() const { return m_modeData; }
        uint8_t getModeComboCount() const { return m_comboNum; }
        const std::vector<uint16_t> &getModeCombos() const { return m_modeCombos; }

        uint16_t getModeCombo(uint8_t combo) const
        {
            if (combo >= m_comboNum || combo >= 16)
                return 0;
            return m_modeCombos[combo];
        }

        /**
         * @returns mode bitmask
         */
        uint16_t getInputModes() const { return m_inModesMask; }
        /**
         * @returns mode bitmask
         */
        uint16_t getOutputModes() const { return m_outModesMask; }
        uint8_t getCapabilities() const
        {
            // Always reflect combinable bit from actual combo count, regardless of descriptor.
            return m_comboNum > 0 ? (m_capabilities | 0x04) : m_capabilities;
        }

        Version getFwVersion() const { return m_fwVersion; }
        Version getHwVersion() const { return m_hwVersion; }

        std::string getInfoStr();

        /**
         * @brief Set the port data from a device descriptor,
         * the function will check if the device type matches before setting the data
         * @param desc Device descriptor to set from
         */
        void setFromDesc(const DeviceDescriptor *desc)
        {
            if (!desc || desc->type != m_deviceType)
                return;
            m_rawDataSizeEnsured = false;
            m_modeData = desc->modes;
            m_modeCount = m_modeData.size();
            m_viewCount = 0; // TODO: add to desc
            m_modeCombos = desc->combos;
            m_comboNum = 0;
            for (auto c : m_modeCombos)
            {
                if (c == 0) break;
                m_comboNum++;
            }
            m_capabilities = desc->caps;
            m_inModesMask = desc->inModesMask;
            m_outModesMask = desc->outModesMask;
            m_fwVersion = desc->fwVersion;
            m_hwVersion = desc->hwVersion;
        }

        /**
         * @brief convert speed to a 8 bit raw value
         * @param speed -100..100
         */
        static uint8_t speedToRaw(int8_t speed)
        {
            uint8_t raw;
            if (speed == 0)
            {
                raw = 127;
            }
            else if (speed > 0)
            {
                raw = Utils::map(speed, 0, 100, 0, 126);
            }
            else
            {
                raw = Utils::map(-speed, 0, 100, 255, 128);
            }
            return raw;
        }

        /**
         * @brief convert 8 bit raw value to speed
         * @param raw raw value
         * @returns speed -100..100
         */
        static int8_t rawToSpeed(uint8_t raw)
        {
            int8_t speed;
            if (raw == 127)
            {
                speed = 0;
            }
            else if (raw < 127)
            {
                speed = Utils::map(raw, 0, 126, 0, 100);
            }
            else
            {
                speed = Utils::map(raw, 255, 128, -100, 0);
            }
            return speed;
        }

        /**
         * @brief populates mode data with zeroes (according to format)
         */
        void ensureRawDataSize();

        /**
         * @brief set the calbback that will be called on incoming mode data from the port
         */
        void setValueChangeCallback(ValueChangeCallback callback)
        {
            m_valueChangeCallback = callback;
        }

    public:
        static uint8_t getDataSize(uint8_t format);
    protected:
        /**
         * @brief Subclass hook implementing one tick of the underlying
         * transport (UART poll, remote message pump, etc.). Called from the
         * non-virtual `update()`.
         */
        virtual void _update() = 0;

        static ModeNum getDefaultMode(DeviceType id);
        static bool deviceIsAbsMotor(DeviceType id);

        /**
         * @brief Store the delta threshold for a single mode.
         * Clears the previous-raw snapshot for that mode so the next update
         * starts a fresh comparison.
         */
        void storeModeDelta(uint8_t modeNum, float delta);

        /**
         * @brief Store per-mode deltas for a combo.
         * @param comboIdx combo index (used to resolve bitmask → mode numbers)
         * @param deltas one entry per set bit in the combo bitmask (ascending mode order)
         */
        void storeComboDeltas(uint8_t comboIdx, const std::vector<float>& deltas);

        /**
         * @brief Call m_valueChangeCallback only when the mode's value changed by >= its stored delta.
         * Falls through unconditionally when delta == 0.
         */
        void fireValueChangeCallback(uint8_t modeNum);

        /// Parse a signed 8-bit integer from raw bytes
        static float parseData8(const uint8_t *ptr);

        /// Parse a signed 16-bit little-endian integer from raw bytes
        static float parseData16(const uint8_t *ptr);

        /// Parse a signed 32-bit little-endian integer from raw bytes
        static float parseData32(const uint8_t *ptr);

        /// Parse a 32-bit IEEE-754 little-endian float
        static float parseDataF(const uint8_t *ptr);

        bool m_rawDataSizeEnsured = false;

        /**
         * @brief Set the port data from a device descriptor if available
         */
        void setFromDesc();

    protected:
        DeviceType m_deviceType = DeviceType::UNKNOWNDEVICE;
        uint8_t m_modeCount = 0, m_viewCount = 0;
        std::vector<uint16_t> m_modeCombos;
        uint8_t m_capabilities = 0;
        uint16_t m_inModesMask = 0, m_outModesMask = 0;
        uint8_t m_comboNum = 0;
        Version m_fwVersion = {}, m_hwVersion = {};

        std::vector<Mode> m_modeData;

        ValueChangeCallback m_valueChangeCallback = nullptr;

        /**
         * @brief Replace the current owned device.
         *
         * (a) Invalidates the existing slot's pointer (any consumer holding
         *     a shared_ptr<DeviceSlot> will now read nullptr).
         * (b) Deletes the previous unique_ptr-owned device.
         * (c) Adopts @p newDev (can be nullptr) under a fresh slot.
         *
         * Called by manageDevice() on connect / disconnect / type change.
         */
        void swapDevice(Device *newDev);

        std::unique_ptr<Device> m_device;
        std::shared_ptr<DeviceSlot> m_slot;

        /// Per-mode delta thresholds (indexed by mode number). 0 = fire on every update.
        std::vector<float> m_deltas;
        /// Per-mode previous raw snapshot used by fireValueChangeCallback for delta comparison.
        std::vector<std::vector<uint8_t>> m_prevRaw;
    };

    class PortDevice : public Lpf2::Device
    {
    public:
        PortDevice(Port &port) : m_port(port) {}

        DeviceType getDeviceType() const override
        {
            return m_port.getDeviceType();
        }

        const std::vector<Mode> &getModes() const override
        {
            return m_port.getModes();
        }

        std::vector<uint16_t> getModeCombos() const override
        {
            return m_port.getModeCombos();
        }

        uint8_t getModeCount() const override
        {
            return m_port.getModeCount();
        }

        Version getFwVersion() const override
        {
            return m_port.getFwVersion();
        }

        Version getHwVersion() const override
        {
            return m_port.getHwVersion();
        }

        /**
         * @returns mode bitmask
         */
        uint16_t getInputModes() const override
        {
            return m_port.getInputModes();
        }

        /**
         * @returns mode bitmask
         */
        uint16_t getOutputModes() const override
        {
            return m_port.getOutputModes();
        }

        uint8_t getCapabilities() const override
        {
            return m_port.getCapabilities();
        }

        int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override
        {
            return m_port.writeData(modeNum, data);
        }

        int setMode(uint8_t mode, float delta = 1.0f) override
        {
            return m_port.setMode(mode, delta);
        }

        int setModeCombo(uint8_t idx, const std::vector<float>& deltas = {}) override
        {
            return m_port.setModeCombo(idx, deltas);
        }

    protected:
        Port &m_port;
    };
}; // namespace Lpf2