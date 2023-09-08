#ifndef G_ADC_H
#define G_ADC_H

#include "gardenObject.hpp"
#include "ioPin.hpp"

namespace gardener
{
    class ADC : public gardenObject
    {
    public:
        ADC(const char *name, uint16_t numChannels);
        virtual ~ADC() {}
        virtual g_err selectChannel(uint16_t channel) { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err read(int32_t &value_mV) { return G_ERR_NO_IMPLEMENTATION; }

    protected:
        uint16_t _numChannels = 0;

    private:
    };

    class adcPin : public ioPin
    {
    public:
        adcPin(const char *name, uint8_t pinNumber, ADC *adc, float scale);
        g_err getVoltage(int32_t &voltage_mV);

    protected:
        int32_t _fastScale;
        ADC *_adc;

    private:
    };

} // namespace gardener

#endif