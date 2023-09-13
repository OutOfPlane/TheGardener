#include "sht40.hpp"

using namespace gardener;

#define DEV_ADDR 0x44

gardener::SHT40::SHT40(const char *name, i2cPort *port)
    : humidityTemperatureSensor(name), _port(port)
{
}

gardener::SHT40::~SHT40()
{
}

g_err gardener::SHT40::getHumidity(int32_t &relhumidity_promill)
{
    g_err erg = G_ERR_OBJECT_LOCKED;
    if (_port->lock(*this, 100))
    {
        _port->w8(DEV_ADDR, 0xFD);
        vTaskDelay(50 / portTICK_RATE_MS);
        uint8_t tmp[6];
        _port->r(DEV_ADDR, tmp, sizeof(tmp));
        _port->unlock();

        uint16_t tmprel = (tmp[3] << 8) | tmp[4];
        relhumidity_promill = tmprel;
        relhumidity_promill = ((relhumidity_promill * 1250) - (60 * 65535)) / 65535;
    }
    return erg;
}

g_err gardener::SHT40::getTemperature(int32_t &temperature_dezideg)
{
    g_err erg = G_ERR_OBJECT_LOCKED;
    if (_port->lock(*this, 100))
    {
        _port->w8(DEV_ADDR, 0xFD);
        vTaskDelay(50 / portTICK_RATE_MS);
        uint8_t tmp[6];
        _port->r(DEV_ADDR, tmp, sizeof(tmp));
        _port->unlock();

        uint16_t tmprel = (tmp[0] << 8) | tmp[1];
        temperature_dezideg = tmprel;
        temperature_dezideg = ((temperature_dezideg * 1750) - (450 * 65535)) / 65535;
    }
    return erg;
}
