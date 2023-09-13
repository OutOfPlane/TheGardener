#ifndef G_SHT40_H
#define G_SHT40_H

#include "humidityTemperatureSensor.hpp"
#include "i2cPort.hpp"

namespace gardener
{
    class SHT40 : public humidityTemperatureSensor
    {
    public:
        SHT40(const char *name, i2cPort *port);
        ~SHT40();
        g_err getHumidity(int32_t &relhumidity_promill);
        g_err getTemperature(int32_t &temperature_dezideg);        

    protected:
    private:
        i2cPort *_port;
        
    };

} // namespace gardener

#endif