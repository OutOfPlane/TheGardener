#include "ioPin.hpp"
#include "adc.hpp"

using namespace gardener;

gardener::ADC::ADC(const char *name, uint16_t numChannels)
: gardenObject(name), _numChannels(numChannels)
{
}

gardener::adcPin::adcPin(const char *name, uint8_t pinNumber, ADC *adc, float scale)
: ioPin(name, pinNumber), _adc(adc)
{
    _fastScale = scale * 1000;
}

g_err gardener::adcPin::getVoltage(int32_t &voltage_mV)
{
    if(_adc->lock(*this, 100))
    {
        g_err erg = G_OK;
        if((erg = _adc->selectChannel(_pinNumber)) == G_OK)
            erg = _adc->read(voltage_mV);
        _adc->unlock();

        voltage_mV *= _fastScale;
        voltage_mV /= 1000;

        return erg;
    }
    return G_ERR_OBJECT_LOCKED;
}
