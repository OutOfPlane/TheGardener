#ifndef G_ENC28J60_H
#define G_ENC28J60_H

#include "networkAdapter.hpp"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_eth.h"
#include "ioPin.hpp"

namespace gardener
{

    class enc28j60 : public networkAdapter
    {
    public:
        enc28j60(const char *name, int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int SPI_CLOCK_MHZ, int SPI_HOST, ioPin *RESET_PIN);
        virtual ~enc28j60() {}
        

    protected:
        g_err _start();
        g_err createNetif();

    private:        
        esp_eth_handle_t eth_handle;
        bool started;
        eth_link_t eth_link;

        int _MISO_GPIO;
        int _MOSI_GPIO;
        int _SCLK_GPIO;
        int _CS_GPIO;
        int _SPI_CLOCK_MHZ;
        int _SPI_HOST;
        ioPin *_RESET_PIN;
        
    };

} // namespace gardener

#endif