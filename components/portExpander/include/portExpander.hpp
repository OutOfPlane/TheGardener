#ifndef G_PORTEXPANDER_H
#define G_PORTEXPANDER_H

#include "ioPin.hpp"

namespace gardener
{
    class portExpander : public gardenObject
    {
    public:
        portExpander(const char *name);
        virtual ~portExpander() {}
        virtual g_err setPinDirection(uint8_t pin, pinDirection dir) { return G_ERR_NO_IMPLEMENTATION; }
        virtual bool pinDirectionSupported(uint8_t pin, pinDirection dir) { return false; }
        virtual g_err setPin(uint8_t pin, uint8_t val) { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err getPin(uint8_t pin, uint8_t &val) { return G_ERR_NO_IMPLEMENTATION; }
        virtual bool canSetPin(uint8_t pin) { return false; }
        virtual bool canGetPin(uint8_t pin) { return false; }
        virtual ioPin *getIOPin(uint8_t pin) { return nullptr; }

    protected:
    private:
    };

    class portExpanderPin : public ioPin
    {
    public:
        portExpanderPin(const char *name, uint8_t pinNumber, portExpander *port);
        ~portExpanderPin();
        g_err mode(pinDirection dir);
        g_err set(uint8_t value);
        g_err get(uint8_t &value);
        bool canSet();
        bool canGet();
    protected:
        portExpander* _port;

    };
} // namespace gardener

#endif