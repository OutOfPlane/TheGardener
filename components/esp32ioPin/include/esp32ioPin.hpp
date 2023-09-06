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
    };

} // namespace gardener

#endif