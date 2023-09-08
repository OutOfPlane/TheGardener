#ifndef G_ADS7828_H
#define G_ADS7828_H

#include "ADC.hpp"
#include "i2cPort.hpp"

namespace gardener
{
    class ads7828 : public ADC
    {
    public:
        ads7828(const char *name, i2cPort *port);
        g_err selectChannel(uint16_t channel);
        g_err read(int32_t &value_mV);

    protected:
    private:
        i2cPort *_port;
    };

} // namespace gardener

#endif