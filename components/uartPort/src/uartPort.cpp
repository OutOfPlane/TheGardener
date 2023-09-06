#include "uartPort.hpp"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <cstring>

using namespace gardener;

uartPort::uartPort(const char *name, uartPortNumber portNumber, uint32_t baud)
    : gardenObject(name), _port(portNumber)
{
}

g_err gardener::uartPort::write(char *data)
{
    return w((uint8_t *)data, strlen(data));
}

g_err gardener::uartPort::w8(uint8_t data)
{
    uint16_t len = 1;
    return w(&data, len);
}

g_err gardener::uartPort::w8r8(uint8_t address, uint8_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w8r16(uint16_t address, uint8_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w8r32(uint32_t address, uint8_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w16r8(uint8_t address, uint16_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w16r16(uint16_t address, uint16_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w16r32(uint32_t address, uint16_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w32r8(uint8_t address, uint32_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w32r16(uint16_t address, uint32_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::w32r32(uint32_t address, uint32_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(buf, sizeof(address) + sizeof(data));
}

g_err gardener::uartPort::read(char *data, uint16_t &maxBufSize, uint32_t timeout_ms)
{
    assert(data);
    if (maxBufSize == 0)
        return G_ERR_INVALID_ARGS;

    maxBufSize--;
    g_err erg = r((uint8_t *)data, maxBufSize, timeout_ms);
    if ((erg == G_ERR_TIMEOUT) || (erg == G_OK))
    {
        data[maxBufSize] = '\0';
    }
    return erg;
}

g_err gardener::uartPort::readUntil(char *data, uint16_t &maxBufSize, uint32_t timeout_ms, char trigger)
{
    assert(data);
    if (maxBufSize == 0)
        return G_ERR_INVALID_ARGS;

    uint16_t idx = 0;
    uint8_t tmp = 0;
    while ((r8(tmp, timeout_ms) == G_OK) &&
           (idx < (maxBufSize - 1)) &&
           (tmp != trigger))
    {
        data[idx] = tmp;
        idx++;
    }
    data[idx] = '\0';
    maxBufSize = idx;
    return G_OK;
}

g_err gardener::uartPort::r8(uint8_t &data, uint32_t timeout_ms)
{
    uint16_t len = 1;
    return r(&data, len, timeout_ms);
}

g_err gardener::uartPort::r8r8(uint8_t address, uint8_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r8r16(uint16_t address, uint8_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r8r32(uint32_t address, uint8_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r16r8(uint8_t address, uint16_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r16r16(uint16_t address, uint16_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r16r32(uint32_t address, uint16_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r32r8(uint8_t address, uint32_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r32r16(uint16_t address, uint32_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}

g_err gardener::uartPort::r32r32(uint32_t address, uint32_t &data, uint32_t timeout_ms)
{
    uint16_t len = sizeof(data);
    g_err erg = w((uint8_t *)&address, len);
    if (erg)
    {
        return erg;
    }
    return r((uint8_t *)&data, len, timeout_ms);
}
