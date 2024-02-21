#ifndef G_ESP32_FREQ_COUNTER_H
#define G_ESP32_FREQ_COUNTER_H

#include "ioPin.hpp"

namespace gardener
{
    class esp32freqCounter : public gardenObject
    {
    public:
        esp32freqCounter(const char *name, uint8_t pinNumber);
        ~esp32freqCounter();
        g_err getFrequency(int32_t &freqHz);
        g_err setSamplingPeriod(uint32_t periodMillis);

    private:
        int64_t _lastUpdate = 0;
        
    };

} // namespace gardener

#endif