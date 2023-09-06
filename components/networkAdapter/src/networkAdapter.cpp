#include "networkAdapter.hpp"

using namespace gardener;

networkAdapter::networkAdapter(const char *name)
    : gardenObject(name), _netif(nullptr)
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
