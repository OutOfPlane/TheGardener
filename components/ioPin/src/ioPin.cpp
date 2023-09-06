#include "ioPin.hpp"

using namespace gardener;

ioPin::ioPin(const char *name, uint8_t pinNumber)
    : gardenObject(name), _pinNumber(pinNumber)
{
    _connected = false;
}

uint8_t ioPin::getNumber()
{
    return _pinNumber;
}

bool gardener::ioPin::isConnected()
{
    return _connected;
}

ioPin* ioPin::_dummyPin = nullptr;

ioPin *ioPin::dummyPin()
{
    if(!_dummyPin)
        _dummyPin = new ioPinDummy("IO_DUMMY");
    return _dummyPin;
}

ioPinDummy::ioPinDummy(const char *name)
: ioPin(name, 0)
{
}

gardener::ioPinDummy::~ioPinDummy()
{
}

g_err gardener::ioPinDummy::mode(pinDirection dir)
{
    return G_OK;
}

g_err gardener::ioPinDummy::set(uint8_t value)
{
    return G_OK;
}

g_err gardener::ioPinDummy::get(uint8_t &value)
{
    value = 1;
    return G_OK;
}

bool gardener::ioPinDummy::canSet()
{
    return true;
}

bool gardener::ioPinDummy::canGet()
{
    return true;
}
