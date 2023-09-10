#ifndef G_NETWORK_ADAPTER_H
#define G_NETWORK_ADAPTER_H

#include "gardenObject.hpp"
#include "esp_netif.h"

#define G_NETWORK_STATE                         \
    X(G_NETWORK_UNINITIALIZED, "uninitialized") \
    X(G_NETWORK_NO_IF, "no interface present")  \
    X(G_NETWORK_CONNECTING, "connecting")       \
    X(G_NETWORK_RECONNECTING, "reconnecting")       \
    X(G_NETWORK_CONNECTED, "connected")         \
    X(G_NETWORK_DISCONNECTED, "disconnected")

namespace gardener
{

#define X(typeID, name) typeID,
    enum g_network_state : size_t
    {
        G_NETWORK_STATE
    };
#undef X

    class networkAdapter : public gardenObject
    {
    public:
        networkAdapter(const char *name);
        virtual ~networkAdapter() {}

        g_err start();
        esp_netif_t *getNetIF();
        g_network_state getState();
        const char *getStateName();

    protected:
        virtual g_err _start() { return G_ERR_NO_IMPLEMENTATION; }
        virtual g_err createNetif() { return G_ERR_NO_IMPLEMENTATION; }
        esp_netif_t *_netif;
        g_network_state _state;
        esp_netif_config_t _netifCfg;
        

    private:
    };

} // namespace gardener

#endif