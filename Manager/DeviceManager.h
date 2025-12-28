#pragma once
#ifndef _LPF2_DEVICE_MANAGER_H_
#define _LPF2_DEVICE_MANAGER_H_

#include "../config.h"
#include "../Lpf2Device.h"
#include "../Lpf2Port.h"
#include <memory>

#include "../Lpf2Devices/BasicMotor.h"

class DeviceManager {
public:
    explicit DeviceManager(Lpf2IO* io)
        : port_(io) {}

    void update() {
        if (!port_.deviceConnected()) {
            device_.reset();
            return;
        }

        if (!device_) {
            attachViaFactory();
        }

        if (device_) {
            device_->poll();
        }
    }

    Lpf2Device* device() {
        return device_.get();
    }

private:
    void attachViaFactory() {
        auto& reg = Lpf2DeviceRegistry::instance();

        for (size_t i = 0; i < reg.count(); ++i) {
            const Lpf2DeviceFactory* factory = reg.factories()[i];

            if (factory->matches(port_)) {
                device_.reset(factory->create(port_));
                device_->init();
                break;
            }
        }
    }

    Lpf2Port port_;
    std::unique_ptr<Lpf2Device> device_;
};

#endif
