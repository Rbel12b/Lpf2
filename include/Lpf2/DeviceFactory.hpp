#pragma once
#include "Lpf2/config.hpp"
#include "Lpf2/Port.hpp"
#include <map>

namespace Lpf2
{
    class DeviceFactory
    {
    public:
        virtual ~DeviceFactory() = default;

        // Return true if this factory recognizes the connected device
        virtual bool matches(const Port &port) const = 0;

        // Create the device (DeviceManager takes ownership)
        virtual PortDevice *create(Port &port) const = 0;

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
            if (m_factoryCount >= MAX_FACTORIES)
            {
                assert(false && "Exceeded maximum number of Lpf2 device factories");
                return;
            }

            m_factories[m_factoryCount++] = factory;
        }

        const DeviceFactory *const *factories() const
        {
            return m_factories;
        }

        size_t count() const
        {
            return m_factoryCount;
        }

    private:
        static constexpr size_t MAX_FACTORIES = 32;

        const DeviceFactory *m_factories[MAX_FACTORIES];
        size_t m_factoryCount = 0;
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
} // namespace Lpf2
