#ifndef G_NETWORK_ADAPTER_H
#define G_NETWORK_ADAPTER_H

#include "gardenObject.hpp"
#include "esp_netif.h"

namespace gardener
{

    class networkAdapter : public gardenObject
    {
    public:
        networkAdapter(const char *name);
        virtual ~networkAdapter() {}

        g_err start();
        esp_netif_t *getNetIF();
        

    protected:
        virtual g_err _start() { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err createNetif(){return G_ERR_NO_IMPLEMENTATION;}
        esp_netif_t *_netif;
        esp_netif_config_t _netifCfg;

    private:
    };

} // namespace gardener

#endif