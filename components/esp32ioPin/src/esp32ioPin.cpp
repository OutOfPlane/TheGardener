#include "esp32ioPin.hpp"
#include "driver/gpio.h"

using namespace gardener;

esp32ioPin::esp32ioPin(const char *name, uint8_t pinNumber)
    : ioPin(name, pinNumber)
{
    _connected = true;
    if (pinNumber >= GPIO_NUM_MAX)
    {
        _pinNumber = GPIO_NUM_NC;
        _connected = false;
    }
}

esp32ioPin::~esp32ioPin()
{
}

g_err esp32ioPin::mode(pinDirection dir)
{
    switch (dir)
    {
    case PIN_OUTPUT:
    {
        gpio_config_t io_conf = {};
        // disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        // set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        // bit mask of the pins that you want to set,e.g.GPIO18/19
        io_conf.pin_bit_mask = (1 << _pinNumber);
        // disable pull-down mode
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        // disable pull-up mode
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        // configure GPIO with the given settings
        return g_err_translate(gpio_config(&io_conf));
    }

    case PIN_INPUT:
    {
        gpio_config_t io_conf = {};
        // disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        // set as output mode
        io_conf.mode = GPIO_MODE_INPUT;
        // bit mask of the pins that you want to set,e.g.GPIO18/19
        io_conf.pin_bit_mask = (1 << _pinNumber);
        // disable pull-down mode
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        // disable pull-up mode
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        // configure GPIO with the given settings
        return g_err_translate(gpio_config(&io_conf));
    }
    break;

    default:
        return G_ERR_NOT_SUPPORTED;
        break;
    }

    // should not get here
    return G_ERR_INVALID_STATE;
}

g_err esp32ioPin::set(uint8_t value)
{
    return g_err_translate(gpio_set_level((gpio_num_t)_pinNumber, value));
}

g_err esp32ioPin::get(uint8_t &value)
{
    int erg = gpio_get_level((gpio_num_t)_pinNumber);
    if ((erg == 0) || (erg == 1))
    {
        return G_OK;
    }
    return G_ERR_HARDWARE;
}

// TODO: respect input only pins here
bool esp32ioPin::canSet()
{
    return true;
}

bool esp32ioPin::canGet()
{
    return true;
}
