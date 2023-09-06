#include "i2cPort.hpp"
#include "driver/i2c.h"
#include <cstring>

using namespace gardener;

i2cPort::i2cPort(const char *name, i2cPortNumber portNumber, uint8_t SCL_PIN, uint8_t SDA_PIN) : gardenObject(name)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN; // select SDA GPIO specific to your project
    conf.scl_io_num = SCL_PIN; // select SCL GPIO specific to your project
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = 100000; // select frequency specific to your project
    conf.clk_flags = 0;             // optional; you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here

    _port = portNumber;

    i2c_param_config(portNumber, &conf);
    i2c_driver_install(portNumber, I2C_MODE_MASTER, 0, 0, 0);
}

g_err gardener::i2cPort::w(uint8_t device, uint8_t *data, uint16_t len)
{
    esp_err_t erg = i2c_master_write_to_device(_port, device, data, len, 10);
    if (erg == ESP_OK)
        return G_OK;
    if (erg == ESP_FAIL)
        return G_ERR_HARDWARE;
    if (erg == ESP_ERR_TIMEOUT)
        return G_ERR_TIMEOUT;
    if (erg == ESP_ERR_INVALID_ARG)
        return G_ERR_INVALID_ARGS;
    if (erg == ESP_ERR_INVALID_STATE)
        return G_ERR_INVALID_STATE;
    return G_ERR_UNK;
}

g_err gardener::i2cPort::w8(uint8_t device, uint8_t data)
{
    esp_err_t erg = i2c_master_write_to_device(_port, device, &data, 1, 10);
    if (erg == ESP_OK)
        return G_OK;
    if (erg == ESP_FAIL)
        return G_ERR_HARDWARE;
    if (erg == ESP_ERR_TIMEOUT)
        return G_ERR_TIMEOUT;
    if (erg == ESP_ERR_INVALID_ARG)
        return G_ERR_INVALID_ARGS;
    if (erg == ESP_ERR_INVALID_STATE)
        return G_ERR_INVALID_STATE;
    return G_ERR_UNK;
}

g_err gardener::i2cPort::w8r8(uint8_t device, uint8_t address, uint8_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w8r16(uint8_t device, uint16_t address, uint8_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w8r32(uint8_t device, uint32_t address, uint8_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w16r8(uint8_t device, uint8_t address, uint16_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w16r16(uint8_t device, uint16_t address, uint16_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w16r32(uint8_t device, uint32_t address, uint16_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w32r8(uint8_t device, uint8_t address, uint32_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w32r16(uint8_t device, uint16_t address, uint32_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::w32r32(uint8_t device, uint32_t address, uint32_t data)
{
    std::memcpy(buf, &address, sizeof(address));
    std::memcpy(buf + sizeof(address), &data, sizeof(data));
    return w(device, buf, sizeof(address) + sizeof(data));
}

g_err gardener::i2cPort::present(uint8_t device, bool &isPresent)
{
    // Source: https://gist.github.com/herzig/8d4c13d8b81a77ac86481c6c1306bb12
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_stop(cmd);
    esp_err_t erg = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    isPresent = (erg == ESP_OK);

    return g_err_translate(erg);
}

g_err gardener::i2cPort::r(uint8_t device, uint8_t *data, uint16_t len)
{
    return g_err_translate(
        i2c_master_read_from_device(_port, device, data, len, 10));
}

g_err gardener::i2cPort::r8(uint8_t device, uint8_t &data)
{
    return g_err_translate(
        i2c_master_read_from_device(_port, device, &data, sizeof(data), 10));
}

g_err gardener::i2cPort::r8r8(uint8_t device, uint8_t address, uint8_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r8r16(uint8_t device, uint16_t address, uint8_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r8r32(uint8_t device, uint32_t address, uint8_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r16r8(uint8_t device, uint8_t address, uint16_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r16r16(uint8_t device, uint16_t address, uint16_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r16r32(uint8_t device, uint32_t address, uint16_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r32r8(uint8_t device, uint8_t address, uint32_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r32r16(uint8_t device, uint16_t address, uint32_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}

g_err gardener::i2cPort::r32r32(uint8_t device, uint32_t address, uint32_t &data)
{
    return g_err_translate(
        i2c_master_write_read_device(_port, device, (uint8_t*)&address, sizeof(address), (uint8_t*)&data, sizeof(data), 10));
}
