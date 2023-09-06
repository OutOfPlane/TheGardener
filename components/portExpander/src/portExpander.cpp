#include "portExpander.hpp"

using namespace gardener;

gardener::portExpander::portExpander(const char *name)
: gardenObject(name)
{
}

portExpanderPin::portExpanderPin(const char *name, uint8_t pinNumber, portExpander *port)
:ioPin(name, pinNumber), _port(port)
{
    _connected = true;
}

portExpanderPin::~portExpanderPin()
{
}

g_err gardener::portExpanderPin::mode(pinDirection dir)
{
    return _port->setPinDirection(_pinNumber, dir);
}

g_err gardener::portExpanderPin::set(uint8_t value)
{
    return _port->setPin(_pinNumber, value);
}

g_err gardener::portExpanderPin::get(uint8_t &value)
{
    return _port->getPin(_pinNumber, value);
}

bool gardener::portExpanderPin::canSet()
{
    return _port->canGetPin(_pinNumber);
}

bool gardener::portExpanderPin::canGet()
{
    return _port->canGetPin(_pinNumber);
}


