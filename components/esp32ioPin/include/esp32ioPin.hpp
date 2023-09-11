#ifndef G_ESP32_IO_PIN_H
#define G_ESP32_IO_PIN_H

#include "ioPin.hpp"

namespace gardener
{
    class esp32ioPin : public ioPin
    {
    public:
        esp32ioPin(const char *name, uint8_t pinNumber);
        ~esp32ioPin();
        g_err mode(pinDirection dir);
        g_err set(uint8_t value);
        g_err get(uint8_t &value);
        bool canSet();
        bool canGet();
        g_err setVoltage(int32_t voltage_mV);

    private:
        uint8_t _this_ledc_channel;
        static uint8_t _ledc_channel;
        static uint8_t _ledc_timer_config_done;
    };

} // namespace gardener

#endif