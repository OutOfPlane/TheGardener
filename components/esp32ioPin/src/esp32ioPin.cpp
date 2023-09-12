#include "esp32ioPin.hpp"
#include "driver/gpio.h"
#include "driver/ledc.h"

using namespace gardener;

uint8_t esp32ioPin::_ledc_channel = LEDC_CHANNEL_0;
uint8_t esp32ioPin::_ledc_timer_config_done = 0;

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
    _dir = dir;
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
        io_conf.pin_bit_mask = _pinNumber;
        io_conf.pin_bit_mask = 1ull << io_conf.pin_bit_mask;
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
        io_conf.pin_bit_mask = _pinNumber;
        io_conf.pin_bit_mask = 1ull << io_conf.pin_bit_mask;
        // disable pull-down mode
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        // disable pull-up mode
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        // configure GPIO with the given settings
        return g_err_translate(gpio_config(&io_conf));
    }
    break;

    case PIN_OUTPUT_PWM:
    {
        g_err erg = G_OK;
        // Prepare and then apply the LEDC PWM timer configuration
        if (!_ledc_timer_config_done)
        {
            ledc_timer_config_t ledc_timer = {
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .duty_resolution = LEDC_TIMER_10_BIT,
                .timer_num = LEDC_TIMER_0,
                .freq_hz = 25000, // Set output frequency at 5 kHz
                .clk_cfg = LEDC_AUTO_CLK};
            if ((erg = g_err_translate(ledc_timer_config(&ledc_timer))) != G_OK)
                return erg;
            _ledc_timer_config_done = 1;
        }

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel = {
            .gpio_num = _pinNumber,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = (ledc_channel_t)_ledc_channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0, // Set duty to 0%
            .hpoint = 0,
            .flags = {0}};
        if ((erg = g_err_translate(ledc_channel_config(&ledc_channel))) == G_OK)
        {
            _this_ledc_channel = _ledc_channel;
            _ledc_channel++;
        }

        return erg;
    }

    default:
        return G_ERR_NOT_SUPPORTED;
        break;
    }

    // should not get here
    return G_ERR_INVALID_STATE;
}

g_err esp32ioPin::set(uint8_t value)
{
    _level = value;
    return g_err_translate(gpio_set_level((gpio_num_t)_pinNumber, value));
}

g_err esp32ioPin::get(uint8_t &value)
{
    if(_dir == PIN_INPUT)
    {
        int erg = gpio_get_level((gpio_num_t)_pinNumber);
        if ((erg == 0) || (erg == 1))
        {
            value = erg;
            return G_OK;
        }
        return G_ERR_HARDWARE;
    }
    if(_dir == PIN_OUTPUT)
    {
        value = _level;
        return G_OK;
    }
    if(_dir == PIN_OUTPUT_PWM)
    {
        value = 0;
        if(_level > 0)
        {
            value = 1;
        }
        return G_OK;
    }
    return G_ERR_INVALID_STATE;
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

g_err esp32ioPin::setVoltage(int32_t voltage_mV)
{
    _level = voltage_mV;
    G_LOGI("Set voltage to %d mV", voltage_mV);
    int32_t val = (voltage_mV * 1023) / 3300;
    if (val < 0)
        val = 0;
    if (val > 1023)
        val = 1023;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)_this_ledc_channel, val);
    return g_err_translate(ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)_this_ledc_channel));
}

g_err gardener::esp32ioPin::getVoltage(int32_t &voltage_mV)
{
    if(_dir == PIN_OUTPUT_PWM)
    {
        voltage_mV = _level;
        return G_OK;
    }
    if(_dir == PIN_OUTPUT)
    {
        voltage_mV = 0;
        if(_level > 0)
        {
            voltage_mV = 3300;
        }
        return G_OK;
    }
    //TODO: add case input
    return G_ERR_INVALID_STATE;
}
