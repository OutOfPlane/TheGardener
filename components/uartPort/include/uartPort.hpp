#ifndef G_UART_PORT_H
#define G_UART_PORT_H

#include "gardenObject.hpp"

namespace gardener
{
    enum uartPortNumber
    {
        uart0 = 0,
        uart1 = 1,
        uart2 = 2,
        uartOther = 100
    };

    enum uartNewline{
        NEWLINE_CRLF,
        NEWLINE_CR,
        NEWLINE_LF
    };

    class uartPort : public gardenObject
    {
    public:
        uartPort(const char *name, uartPortNumber portNumber, uint32_t baud);

        // writing
        virtual g_err w(uint8_t *data, uint16_t len){return G_ERR_NO_IMPLEMENTATION;}
        
        //convenience Functions for writing
        g_err write(char *data);
        
        g_err w8(uint8_t data);
        g_err w8r8(uint8_t address, uint8_t data);
        g_err w8r16(uint16_t address, uint8_t data);
        g_err w8r32(uint32_t address, uint8_t data);
        g_err w16r8(uint8_t address, uint16_t data);
        g_err w16r16(uint16_t address, uint16_t data);
        g_err w16r32(uint32_t address, uint16_t data);
        g_err w32r8(uint8_t address, uint32_t data);
        g_err w32r16(uint16_t address, uint32_t data);
        g_err w32r32(uint32_t address, uint32_t data);

        // reading
        virtual g_err r(uint8_t *data, uint16_t &len, uint32_t timeout_ms){return G_ERR_NO_IMPLEMENTATION;}

        //convenience Functions for reading
        g_err read(char *data, uint16_t &maxBufSize, uint32_t timeout_ms);
        g_err readUntil(char *data, uint16_t &maxBufSize, uint32_t timeout_ms, char trigger = '\n');
        
        g_err r8(uint8_t &data, uint32_t timeout_ms);
        g_err r8r8(uint8_t address, uint8_t &data, uint32_t timeout_ms);
        g_err r8r16(uint16_t address, uint8_t &data, uint32_t timeout_ms);
        g_err r8r32(uint32_t address, uint8_t &data, uint32_t timeout_ms);
        g_err r16r8(uint8_t address, uint16_t &data, uint32_t timeout_ms);
        g_err r16r16(uint16_t address, uint16_t &data, uint32_t timeout_ms);
        g_err r16r32(uint32_t address, uint16_t &data, uint32_t timeout_ms);
        g_err r32r8(uint8_t address, uint32_t &data, uint32_t timeout_ms);
        g_err r32r16(uint16_t address, uint32_t &data, uint32_t timeout_ms);
        g_err r32r32(uint32_t address, uint32_t &data, uint32_t timeout_ms);

        //status
        virtual g_err present(bool &isPresent){return G_ERR_NO_IMPLEMENTATION;}

    protected:
        uartPortNumber _port;
    private:
        
        uint8_t buf[8];
    };

} // namespace gardener

#endif