#ifndef G_ESP32_UART_PORT_H
#define G_ESP32_UART_PORT_H

#include "uartPort.hpp"

namespace gardener
{
    class esp32uartPort : public uartPort
    {
    public:
        esp32uartPort(const char *name, uartPortNumber portNumber, uint32_t baud, uint8_t TX_PIN, uint8_t RX_PIN);

        // writing
        g_err w(uint8_t *data, uint16_t len);
        // reading
        g_err r(uint8_t *data, uint16_t &len, uint32_t timeout_ms);

        //status
        g_err present(bool &isPresent);

    protected:
    private:
    };

} // namespace gardener

#endif