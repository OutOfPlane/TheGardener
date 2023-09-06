#include "esp32uartPort.hpp"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <cstring>

using namespace gardener;

esp32uartPort::esp32uartPort(const char *name, uartPortNumber portNumber, uint32_t baud, uint8_t TX_PIN, uint8_t RX_PIN)
    : uartPort(name, portNumber, baud)
{
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1 << TX_PIN);
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1 << RX_PIN);
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    uart_config_t conf;
    conf.baud_rate = baud;
    conf.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    conf.data_bits = UART_DATA_8_BITS;
    conf.parity = UART_PARITY_DISABLE;
    conf.stop_bits = UART_STOP_BITS_1;
    conf.source_clk = UART_SCLK_APB;
    conf.rx_flow_ctrl_thresh = 112;

    ESP_ERROR_CHECK(uart_param_config(_port, &conf));
    ESP_ERROR_CHECK(uart_driver_install(_port, 1024, 1024, 0, nullptr, CONFIG_UART_ISR_IN_IRAM));
    ESP_ERROR_CHECK(uart_set_pin(_port, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

g_err esp32uartPort::w(uint8_t *data, uint16_t len)
{
    G_LOGI("writing: %.*s\r\n", len, (char *)data);
    int erg = uart_write_bytes(_port, data, len);
    if (erg < 0)
    {
        return G_ERR_HARDWARE;
    }
    if (len != erg)
    {
        return G_ERR_NOMEM;
    }
    return G_OK;
}

g_err esp32uartPort::r(uint8_t *data, uint16_t &len, uint32_t timeout_ms)
{
    int erg = uart_read_bytes(_port, data, len, timeout_ms / portTICK_PERIOD_MS);
    if (erg < 0)
    {
        len = 0;
        return G_ERR_HARDWARE;
    }
    if (len != erg)
    {
        len = erg;
        return G_ERR_TIMEOUT;
    }
    return G_OK;
}

g_err esp32uartPort::present(bool &isPresent)
{
    isPresent = true;
    return G_OK;
}
