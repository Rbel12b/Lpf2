#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Port.hpp"
#include <map>
#include <string>

using DeviceCapabilityId = uint32_t;

namespace Lpf2
{
    class Device
    {
    public:
        explicit Device(Port &port) : m_port(port) {}
        virtual ~Device() = default;

        virtual bool init() = 0;
        virtual void poll() = 0;
        virtual const char *name() const = 0;

        virtual bool hasCapability(DeviceCapabilityId id) const = 0;
        virtual void *getCapability(DeviceCapabilityId id) = 0;

        const static DeviceCapabilityId CAP;

    protected:
        Port &m_port;
    };

    class DeviceFactory
    {
    public:
        virtual ~DeviceFactory() = default;

        // Return true if this factory recognizes the connected device
        virtual bool matches(Port &port) const = 0;

        // Create the device (DeviceManager takes ownership)
        virtual Device *create(Port &port) const = 0;

        virtual const char *name() const = 0;
    };

    class DeviceRegistry
    {
    public:
        static DeviceRegistry &instance()
        {
            static DeviceRegistry inst;
            return inst;
        }

        static void registerDefault();

        void registerFactory(const DeviceFactory *factory)
        {
            if (count_ >= MAX_FACTORIES)
            {
                assert(false && "Exceeded maximum number of Lpf2 device factories");
                return;
            }

            factories_[count_++] = factory;
        }

        const DeviceFactory *const *factories() const
        {
            return factories_;
        }

        size_t count() const
        {
            return count_;
        }

    private:
        static constexpr size_t MAX_FACTORIES = 32;

        const DeviceFactory *factories_[MAX_FACTORIES];
        size_t count_ = 0;
    };

    class Lpf2CapabilityRegistry
    {
    public:
        static constexpr uint32_t fnv1a(const char *s, uint32_t h = 2166136261u)
        {
            return *s ? fnv1a(s + 1, (h ^ uint32_t(*s)) * 16777619u) : h;
        }

        static constexpr DeviceCapabilityId registerCapability(const char *const name)
        {
            return fnv1a(name);
        }
    };
}; // namespace Lpf2