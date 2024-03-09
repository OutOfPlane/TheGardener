#include "ads7828.hpp"

using namespace gardener;

#define BIT_MODE_SELECT 7
#define BIT_CHANNEL_SELECT 4
#define BIT_POWER_SELECT 2
#define DEV_ADDR 0x48
#define DEV_SPEED 100000

gardener::ads7828::ads7828(const char *name, i2cPort *port)
    : ADC(name, 8), _port(port)
{
}

g_err gardener::ads7828::selectChannel(uint16_t channel)
{
    if (_port->lock(*this))
    {
        uint8_t cmd = (1 << BIT_MODE_SELECT) | (channel << BIT_CHANNEL_SELECT) | (0x03 << BIT_POWER_SELECT);
        g_err erg = _port->w8(DEV_ADDR, cmd);
        _port->unlock();
        return erg;
    }
    return G_ERR_OBJECT_LOCKED;
}

g_err gardener::ads7828::read(int32_t &value_mV)
{
    if (_port->lock(*this, 200))
    {
        uint8_t buf[2];
        g_err erg = _port->r(DEV_ADDR, buf, sizeof(buf));
        _port->unlock();
        value_mV = (buf[0] << 8) | (buf[1]);
        value_mV *= 2500;
        value_mV /= 0xFFF;
        return erg;
    }
    return G_ERR_OBJECT_LOCKED;
}
