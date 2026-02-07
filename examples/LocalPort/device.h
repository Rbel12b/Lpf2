#pragma once
#include "config.h"
#include "UART/UART.h"
#include "Lpf2Local/IO/IO.h"

#ifdef ESP32

#include <Arduino.h>
#include <HardwareSerial.h>

class Esp32s3Uart : public Lpf2Uart
{
public:
    explicit Esp32s3Uart(int uart_num)
        : serial_(uart_num)
    {
    }

    bool begin(uint32_t baudrate,
               uint32_t config = SERIAL_8N1,
               int rx_pin = -1,
               int tx_pin = -1,
               int id1_pin = -1,
               int id2_pin = -1) override
    {
        rx_pin_ = rx_pin;
        tx_pin_ = tx_pin;
        id1_pin_ = id1_pin;
        id2_pin_ = id2_pin;
        config_ = config;
        baud_ = baudrate;

        // Configure ADC pins if provided
        if (id1_pin_ >= 0)
        {
            pinMode(id1_pin_, INPUT);
        }
        if (id2_pin_ >= 0)
        {
            pinMode(id2_pin_, INPUT);
        }

        uartPinsOff();

        return true;
    }

    void end() override
    {
        serial_.end();
        uartPinsOff();
    }

    void setBaudrate(uint32_t baudrate) override
    {
        baud_ = baudrate;
        serial_.updateBaudRate(baudrate);
    }

    size_t write(const uint8_t *data, size_t length) override
    {
        return serial_.write(data, length);
    }

    int read() override
    {
        return serial_.read();
    }

    int available() override
    {
        return serial_.available();
    }

    void flush() override
    {
        serial_.flush();
    }

    void setUartPinsState(bool highZ) override
    {
        LPF2_LOG_D("Setting uart pins state: highZ=%s", highZ ? "true" : "false");

        if (highZ)
        {
            // Detach UART pins → high impedance
            serial_.end();
            if (rx_pin_ >= 0)
                pinMode(rx_pin_, INPUT);
            if (tx_pin_ >= 0)
                pinMode(tx_pin_, INPUT);
        }
        else
        {
            // Reattach UART pins
            if (rx_pin_ >= 0 || tx_pin_ >= 0)
            {
                if (rx_pin_ >= 0)
                    pinMode(rx_pin_, INPUT);
                if (tx_pin_ >= 0)
                    pinMode(tx_pin_, INPUT);
                serial_.end();
                serial_.begin(baud_, config_, rx_pin_, tx_pin_);
            }
        }
    }

    float readCh(uint8_t ch) override
    {
        int pin = -1;

        if (ch == 0)
        {
            pin = id1_pin_;
        }
        else
        {
            pin = id2_pin_;
        }

        if (pin < 0)
        {
            return 0.0f;
        }

        // ESP32 ADC: 12-bit by default (0–4095), reference ~3.3V
        constexpr float VREF = 3.3f;
        int raw = analogRead(pin);
        return (raw / 4095.0f) * VREF;
    }

private:
    HardwareSerial serial_;

    int rx_pin_ = -1;
    int tx_pin_ = -1;
    int id1_pin_ = -1;
    int id2_pin_ = -1;

    uint32_t baud_ = 115200;
    uint32_t config_ = SERIAL_8N1;
};

#include "driver/mcpwm.h"

class Esp32s3MotorPWM : public Lpf2PWM
{
public:
    explicit Esp32s3MotorPWM() = default;

    int init(int pin1,
             int pin2,
             mcpwm_unit_t unit,
             mcpwm_timer_t timer,
             uint32_t freq = 1000)
    {
        pin1_ = pin1;
        pin2_ = pin2;
        unit_ = unit;
        timer_ = timer;
        freq_ = freq;

        // Attach GPIOs
        mcpwm_gpio_init(unit_, mcpwm_io_signals_t(MCPWM0A + timer_ * 2), pin1_);
        mcpwm_gpio_init(unit_, mcpwm_io_signals_t(MCPWM0B + timer_ * 2), pin2_);

        mcpwm_config_t cfg = {};
        cfg.frequency = freq_;
        cfg.cmpr_a = 0;
        cfg.cmpr_b = 0;
        cfg.counter_mode = MCPWM_UP_COUNTER;
        cfg.duty_mode = MCPWM_DUTY_MODE_0;

        mcpwm_init(unit_, timer_, &cfg);

        return 0;
    }

    void out(uint8_t ch1, uint8_t ch2) override
    {
        // Convert 0–255 → percentage
        float dutyA = (ch1 * 100.0f) / 255.0f;
        float dutyB = (ch2 * 100.0f) / 255.0f;

        mcpwm_set_duty(unit_, timer_, MCPWM_OPR_A, dutyA);
        mcpwm_set_duty(unit_, timer_, MCPWM_OPR_B, dutyB);

        mcpwm_set_duty_type(unit_, timer_, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
        mcpwm_set_duty_type(unit_, timer_, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    }

private:
    int pin1_ = -1;
    int pin2_ = -1;
    mcpwm_unit_t unit_;
    mcpwm_timer_t timer_;
    uint32_t freq_ = 1000;
};

class Esp32IO : public Lpf2IO
{
public:
    Esp32IO(int uartNum) : m_uart(uartNum) {}

    Lpf2PWM *getPWM() override
    {
        return &m_pwm;
    }

    Lpf2Uart *getUart() override
    {
        return &m_uart;
    }

    /**
     * @brief Initialize the IO ports.
     * @param id1_pin The analog input connected to the TX pin, external 47k pull-up is required. (Can be the same as tx_pin)
     * @param id2_pin The analog input connected to the RX pin, external 47k pull-up is required. (Can be the same as rx_pin)
     * @param pwm_pin1 The first PWM output pin. Use -1 for no PWM.
     * @param pwm_pin2 The second PWM output pin. Use -1 for no PWM.
     * @param freq The PWM frequency in Hz.
     * @param resolution The PWM resolution in bits.
     * @param channel1 The PWM channel for the first pin.
     * @param channel2 The PWM channel for the second pin.
     * @return 0 if initialization was successful, -1 otherwise.
     * @note Must be called before any IO operations. (Before calling init on Lpf2Port or update on Lpf2DeviceManager)
     */
    int init(int id1_pin = -1, int id2_pin = -1, int pwm_pin1 = -1, int pwm_pin2 = -1, mcpwm_unit_t unit = mcpwm_unit_t(0), mcpwm_timer_t timer = mcpwm_timer_t(0), uint32_t freq = 1000)
    {
        if (!m_uart.begin(115200, SERIAL_8N1, id2_pin, id1_pin, id1_pin, id2_pin))
        {
            return -1;
        }
        if (m_pwm.init(pwm_pin1, pwm_pin2, unit, timer, freq) != 0)
        {
            return -1;
        }
        m_inited = true;
        return 0;
    }

    bool ready() const override
    {
        return m_inited;
    }

private:
    Esp32s3Uart m_uart;
    Esp32s3MotorPWM m_pwm;
    bool m_inited = false;
};

#endif // ESP32