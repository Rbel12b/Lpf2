#pragma once
#ifndef _LPF2_DEVICE_H_
#define _LPF2_DEVICE_H_

#include "config.h"
#include "Lpf2Port.h"

class Lpf2Device {
public:
    explicit Lpf2Device(Lpf2Port& port) : port_(port) {}
    virtual ~Lpf2Device() = default;

    virtual bool init() = 0;
    virtual void poll() = 0;
    virtual const char* name() const = 0;

protected:
    Lpf2Port& port_;
};

class Lpf2DeviceFactory {
public:
    virtual ~Lpf2DeviceFactory() = default;

    // Return true if this factory recognizes the connected device
    virtual bool matches(Lpf2Port& port) const = 0;

    // Create the device (DeviceManager takes ownership)
    virtual Lpf2Device* create(Lpf2Port& port) const = 0;
};

class Lpf2DeviceRegistry {
public:
    static Lpf2DeviceRegistry& instance() {
        static Lpf2DeviceRegistry inst;
        return inst;
    }

    void registerFactory(const Lpf2DeviceFactory* factory) {
        factories_[count_++] = factory;
    }

    const Lpf2DeviceFactory* const* factories() const {
        return factories_;
    }

    size_t count() const {
        return count_;
    }

private:
    static constexpr size_t MAX_FACTORIES = 8;

    const Lpf2DeviceFactory* factories_[MAX_FACTORIES];
    size_t count_ = 0;
};

#endif