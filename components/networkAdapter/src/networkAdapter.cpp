#include "networkAdapter.hpp"

using namespace gardener;

#define X(typeID, name) name,
const char *_NetworkToString[]{
    G_NETWORK_STATE};
#undef X

networkAdapter::networkAdapter(const char *name)
    : gardenObject(name), _netif(nullptr), _state(G_NETWORK_UNINITIALIZED)
{
}

g_err networkAdapter::start()
{
    if (createNetif() == G_OK)
    {
        G_LOGI("Create Network Interface Success");
    }
    else
    {
        G_LOGI("Create Network Interface Fail");
    }

    G_LOGI("Starting networkAdapter");
    g_err erg = _start();
    if (erg == G_OK)
    {
        G_LOGI("Adapter Start Success");
    }

    return erg;
}

esp_netif_t *networkAdapter::getNetIF()
{
    return _netif;
}

g_network_state gardener::networkAdapter::getState()
{
    return _state;
}

const char *gardener::networkAdapter::getStateName()
{
    return _NetworkToString[_state];
}
