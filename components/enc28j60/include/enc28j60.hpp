#ifndef G_ENC28J60_H
#define G_ENC28J60_H

#include "networkAdapter.hpp"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_eth.h"

namespace gardener
{

    class enc28j60 : public networkAdapter
    {
    public:
        enc28j60(const char *name);
        virtual ~enc28j60() {}
        

    protected:
        g_err _start();
        g_err createNetif();
        spiInterface *spiIF;
        static void _eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    private:        
        EventGroupHandle_t _event_group;
        esp_netif_t *_ap_netif;
        esp_eth_handle_t eth_handle;
        bool started;
        eth_link_t eth_link;
        
    };

} // namespace gardener

#endif