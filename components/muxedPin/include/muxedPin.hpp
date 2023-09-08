#ifndef G_MUXED_PIN_H
#define G_MUXED_PIN_H

#include "gardenObject.hpp"
#include "ioPin.hpp"
#include "externalMux.hpp"

namespace gardener
{
    class muxedPin : public ioPin
    {
    public:
        muxedPin(const char *name, ioPin *sharedPin, externalMux *pinMux, uint8_t pinNumber);
        g_err getVoltage(int32_t &voltage_mV);

    private:
        ioPin *_sharedPin;
        externalMux *_pinMux;
    };

} // namespace gardener

#endif