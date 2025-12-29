#pragma once
#ifndef _LPF2_DEVICE_H_
#define _LPF2_DEVICE_H_

#include "config.h"
#include "Lpf2Port.h"
#include <map>
#include <string>

using CapabilityId = uint16_t;

class Lpf2Device
{
public:
    explicit Lpf2Device(Lpf2Port &port) : port_(port) {}
    virtual ~Lpf2Device() = default;

    virtual bool init() = 0;
    virtual void poll() = 0;
    virtual const char *name() const = 0;

    virtual bool hasCapability(CapabilityId id) const = 0;
    virtual void *getCapability(CapabilityId id) = 0;

    const static CapabilityId CAP;

protected:
    Lpf2Port &port_;
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

class CapabilityRegistry
{
public:
    static CapabilityRegistry &instance()
    {
        static CapabilityRegistry inst;
        return inst;
    }

    CapabilityId registerCapability(const char *name)
    {
        if (name == nullptr)
        {
            return 0;
        }
        auto it = std::find_if(names_.begin(), names_.end(),
                               [name](const auto &pair)
                               { return std::string(pair.second) == name; });
        if (it != names_.end())
        {
            return it->first;
        }
        CapabilityId id = nextId_++;
        names_[id] = name;
        return id;
    }

    const char *name(CapabilityId id) const
    {
        auto it = names_.find(id);
        return it != names_.end() ? it->second : "unknown";
    }

private:
    CapabilityId nextId_ = 1;
    std::map<CapabilityId, const char *> names_;
};

#endif