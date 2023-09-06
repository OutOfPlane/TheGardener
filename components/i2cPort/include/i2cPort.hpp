#ifndef G_I2C_PORT_H
#define G_I2C_PORT_H

#include "gardenObject.hpp"

namespace gardener
{
    enum i2cPortNumber
    {
        i2c0 = 0,
        i2c1 = 1
    };

    class i2cPort : public gardenObject
    {
    public:
        i2cPort(const char *name, i2cPortNumber portNumber, uint8_t SCL_PIN, uint8_t SDA_PIN);

        // writing
        g_err w(uint8_t device, uint8_t *data, uint16_t len);
        g_err w8(uint8_t device, uint8_t data);
        g_err w8r8(uint8_t device, uint8_t address, uint8_t data);
        g_err w8r16(uint8_t device, uint16_t address, uint8_t data);
        g_err w8r32(uint8_t device, uint32_t address, uint8_t data);
        g_err w16r8(uint8_t device, uint8_t address, uint16_t data);
        g_err w16r16(uint8_t device, uint16_t address, uint16_t data);
        g_err w16r32(uint8_t device, uint32_t address, uint16_t data);
        g_err w32r8(uint8_t device, uint8_t address, uint32_t data);
        g_err w32r16(uint8_t device, uint16_t address, uint32_t data);
        g_err w32r32(uint8_t device, uint32_t address, uint32_t data);

        // reading
        g_err r(uint8_t device, uint8_t *data, uint16_t len);
        g_err r8(uint8_t device, uint8_t &data);
        g_err r8r8(uint8_t device, uint8_t address, uint8_t &data);
        g_err r8r16(uint8_t device, uint16_t address, uint8_t &data);
        g_err r8r32(uint8_t device, uint32_t address, uint8_t &data);
        g_err r16r8(uint8_t device, uint8_t address, uint16_t &data);
        g_err r16r16(uint8_t device, uint16_t address, uint16_t &data);
        g_err r16r32(uint8_t device, uint32_t address, uint16_t &data);
        g_err r32r8(uint8_t device, uint8_t address, uint32_t &data);
        g_err r32r16(uint8_t device, uint16_t address, uint32_t &data);
        g_err r32r32(uint8_t device, uint32_t address, uint32_t &data);

        g_err present(uint8_t device, bool &isPresent);

    protected:
    private:
        i2cPortNumber _port;
        uint8_t buf[8];
    };

} // namespace gardener

#endif