#include "muxedPin.hpp"

using namespace gardener;

gardener::muxedPin::muxedPin(const char *name, ioPin *sharedPin, externalMux *pinMux, uint8_t pinNumber)
: ioPin(name, pinNumber), _sharedPin(sharedPin), _pinMux(pinMux)
{
}

g_err gardener::muxedPin::getVoltage(int32_t &voltage_mV)
{
    g_err erg = G_ERR_OBJECT_LOCKED;
    if(!_pinMux)
        return G_ERR_INVALID_STATE;
    if(_pinMux->lock(*this, 100))
    {
        _pinMux->selectChannel(_pinNumber);
        if(_sharedPin->lock(*this, 100))
        {
            erg = _sharedPin->getVoltage(voltage_mV);
            _sharedPin->unlock();
        }
        _pinMux->unlock();
    }
    return erg;
}
