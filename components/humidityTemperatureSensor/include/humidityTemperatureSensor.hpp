#ifndef G_HUMIDITY_TEMPERATURE_H
#define G_HUMIDITY_TEMPERATURE_H

#include "gardenObject.hpp"

namespace gardener
{
    class humidityTemperatureSensor : public gardenObject
    {
    public:
        humidityTemperatureSensor(const char *name);
        virtual ~humidityTemperatureSensor() {}
        virtual g_err getHumidity(int32_t &relhumidity_promill) { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err getTemperature(int32_t &temperature_dezideg) { return G_ERR_NO_IMPLEMENTATION; }

    protected:
    private:
    };
} // namespace gardener

#endif