#include "tca9534.hpp"

using namespace gardener;

#define REG_INPUT_PORT 0x00
#define REG_OUTPUT_PORT 0x01
#define REG_INVERT_PORT 0x02
#define REG_CONFIG_PORT 0x03
#define DEV_ADDR 0x23
#define DEV_SPEED 100000

gardener::TCA9534::TCA9534(const char *name, i2cPort *port)
    : portExpander(name), _port(port), _DDR(0xFF), _PIN(0x00)
{
    assert(port);
    _pins[0] = new portExpanderPin("PE0", 0, this);
    _pins[1] = new portExpanderPin("PE1", 1, this);
    _pins[2] = new portExpanderPin("PE2", 2, this);
    _pins[3] = new portExpanderPin("PE3", 3, this);
    _pins[4] = new portExpanderPin("PE4", 4, this);
    _pins[5] = new portExpanderPin("PE5", 5, this);
    _pins[6] = new portExpanderPin("PE6", 6, this);
    _pins[7] = new portExpanderPin("PE7", 7, this);    
}

gardener::TCA9534::~TCA9534()
{
}

g_err gardener::TCA9534::setPinDirection(uint8_t pin, pinDirection dir)
{
    g_err erg = G_ERR_OBJECT_LOCKED;
    if (_port->lock(*this, 10))
    {
        if (dir == PIN_INPUT)
        {
            _DDR |= (1 << pin);
        }
        if (dir == PIN_OUTPUT)
        {
            _DDR &= ~(1 << pin);
        }
        erg = _port->w8r8(DEV_ADDR, REG_CONFIG_PORT, _DDR);
        _port->unlock();
    }
    return erg;
}

bool gardener::TCA9534::pinDirectionSupported(uint8_t pin, pinDirection dir)
{
    return ((pin < 8) && ((dir == PIN_INPUT) || (dir == PIN_OUTPUT)));
}

g_err gardener::TCA9534::setPin(uint8_t pin, uint8_t val)
{
    g_err erg = G_ERR_TIMEOUT;
    if (_port->lock(*this, 10))
    {
        if (val)
        {
            _PIN |= (1 << pin);
        }
        else
        {
            _PIN &= ~(1 << pin);
        }
        erg = _port->w8r8(DEV_ADDR, REG_OUTPUT_PORT, _PIN);
        _port->unlock();
    }
    return erg;
}

g_err gardener::TCA9534::getPin(uint8_t pin, uint8_t &val)
{
    g_err erg = G_ERR_TIMEOUT;
    if (_port->lock(*this, 10))
    {
        erg = _port->r8r8(DEV_ADDR, REG_INPUT_PORT, val);
        _port->unlock();
        val = (val>>pin) & 0x01;
    }
    return erg;
}

bool gardener::TCA9534::canSetPin(uint8_t pin)
{
    return (pin < 8);
}

bool gardener::TCA9534::canGetPin(uint8_t pin)
{
    return (pin < 8);
}

ioPin *gardener::TCA9534::getIOPin(uint8_t pin)
{
    if (pin < 8)
    {
        return _pins[pin];
    }
    return nullptr;
}
