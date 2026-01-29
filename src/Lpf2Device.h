#pragma once
#ifndef _LPF2_DEVICE_H_
#define _LPF2_DEVICE_H_

#include "config.h"
#include "Lpf2Port.h"
#include <map>
#include <string>

using Lpf2DeviceCapabilityId = uint32_t;

class Lpf2Device
{
public:
    explicit Lpf2Device(Lpf2Port &port) : m_port(port) {}
    virtual ~Lpf2Device() = default;

    virtual bool init() = 0;
    virtual void poll() = 0;
    virtual const char *name() const = 0;

    virtual bool hasCapability(Lpf2DeviceCapabilityId id) const = 0;
    virtual void *getCapability(Lpf2DeviceCapabilityId id) = 0;

    const static Lpf2DeviceCapabilityId CAP;

protected:
    Lpf2Port &m_port;
};

class Lpf2DeviceFactory
{
public:
    virtual ~Lpf2DeviceFactory() = default;

    // Return true if this factory recognizes the connected device
    virtual bool matches(Lpf2Port &port) const = 0;

    // Create the device (Lpf2DeviceManager takes ownership)
    virtual Lpf2Device *create(Lpf2Port &port) const = 0;

    virtual const char *name() const = 0;
};

class Lpf2DeviceRegistry
{
public:
    static Lpf2DeviceRegistry &instance()
    {
        static Lpf2DeviceRegistry inst;
        return inst;
    }

    static void registerDefaultFactories();

    void registerFactory(const Lpf2DeviceFactory *factory)
    {
        if (count_ >= MAX_FACTORIES)
        {
            assert(false && "Exceeded maximum number of Lpf2 device factories");
            return;
        }

        factories_[count_++] = factory;
    }

    const Lpf2DeviceFactory *const *factories() const
    {
        return factories_;
    }

    size_t count() const
    {
        return count_;
    }

private:
    static constexpr size_t MAX_FACTORIES = 32;

    const Lpf2DeviceFactory *factories_[MAX_FACTORIES];
    size_t count_ = 0;
};

class Lpf2CapabilityRegistry
{
public:
    static constexpr uint32_t fnv1a(const char *s, uint32_t h = 2166136261u)
    {
        return *s ? fnv1a(s + 1, (h ^ uint32_t(*s)) * 16777619u) : h;
    }

    static constexpr Lpf2DeviceCapabilityId registerCapability(const char *const name)
    {
        return fnv1a(name);
    }
};

#endif