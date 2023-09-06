#ifndef G_IO_PIN_H
#define G_IO_PIN_H

#include "gardenObject.hpp"

namespace gardener
{
    enum pinDirection
    {
        PIN_INPUT,
        PIN_INPUT_PULLUP,
        PIN_INPUT_PULLDOWN,
        PIN_OUTPUT,
        PIN_OUTPUT_OPEN_DRAIN,
    };

    class ioPin : public gardenObject
    {
    public:
        ioPin(const char *name, uint8_t pinNumber);
        virtual ~ioPin() {}
        uint8_t getNumber();
        bool isConnected();
        virtual g_err mode(pinDirection dir) { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err set(uint8_t value) { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err get(uint8_t &value) { return G_ERR_NO_IMPLEMENTATION; }
        virtual bool canSet() { return false; }
        virtual bool canGet() { return false; }
        static ioPin *dummyPin();

    protected:
        uint8_t _pinNumber;        
        bool _connected;

    private:
        static ioPin * _dummyPin;
    };

    class ioPinDummy : public ioPin
    {
    public:
        ioPinDummy(const char *name);
        ~ioPinDummy();
        g_err mode(pinDirection dir);
        g_err set(uint8_t value);
        g_err get(uint8_t &value);
        bool canSet();
        bool canGet();

    protected:
        uint8_t _pinNumber;        

    private:
        static ioPin * _dummyPin;
    };

} // namespace gardener

#endif