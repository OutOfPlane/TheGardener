#ifndef G_TCA9534_H
#define G_TCA9534_H

#include "portExpander.hpp"
#include "i2cPort.hpp"

namespace gardener
{
    class TCA9534 : public portExpander
    {
    public:
        TCA9534(const char *name, i2cPort *port);
        ~TCA9534();
        g_err setPinDirection(uint8_t pin, pinDirection dir);
        bool pinDirectionSupported(uint8_t pin, pinDirection dir);
        g_err setPin(uint8_t pin, uint8_t val);
        g_err getPin(uint8_t pin, uint8_t &val);
        bool canSetPin(uint8_t pin);
        bool canGetPin(uint8_t pin);
        ioPin *getIOPin(uint8_t pin);

    protected:
    private:
        i2cPort *_port;
        uint8_t _DDR;
        uint8_t _PIN;
        portExpanderPin *_pins[8];
    };

} // namespace gardener

#endif