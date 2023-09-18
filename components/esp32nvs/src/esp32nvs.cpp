#include "esp32nvs.hpp"
#include "nvs_flash.h"

using namespace gardener;

esp32nvs::esp32nvs(const char *name)
    : configStorage(name)
{
}

esp32nvs::~esp32nvs()
{
}

g_err gardener::esp32nvs::openNamespace(const char *name)
{
    g_err erg = g_err_translate(nvs_open(name, NVS_READWRITE, &_handle));
    if (erg != G_OK) {
        G_ERROR_DECODE(erg);
        return erg;
    }
    _isOpen = true;
    return G_OK;
}

g_err gardener::esp32nvs::close()
{
    if(!_isOpen)
        return G_ERR_INVALID_STATE;
    g_err erg = g_err_translate(nvs_commit(_handle));
    nvs_close(_handle);
    return erg;
}

g_err gardener::esp32nvs::readUI8(const char *key, uint8_t &value)
{
    if(!_isOpen)
        return G_ERR_INVALID_STATE;
    g_err erg = g_err_translate(nvs_get_u8(_handle, key, &value));
    return erg;
}

g_err gardener::esp32nvs::readStr(const char *key, char *value, size_t &length)
{
    if(!_isOpen)
        return G_ERR_INVALID_STATE;
    g_err erg = g_err_translate(nvs_get_str(_handle, key, value, &length));
    return erg;
}

g_err gardener::esp32nvs::writeUI8(const char *key, uint8_t value)
{
    if(!_isOpen)
        return G_ERR_INVALID_STATE;
    g_err erg = g_err_translate(nvs_set_u8(_handle, key, value));
    if(erg == G_OK)
        erg = g_err_translate(nvs_commit(_handle));
    return erg;
}

g_err gardener::esp32nvs::writeStr(const char *key, const char *value)
{
    if(!_isOpen)
        return G_ERR_INVALID_STATE;
    g_err erg = g_err_translate(nvs_set_str(_handle, key, value));
    if(erg == G_OK)
        erg = g_err_translate(nvs_commit(_handle));
    return erg;
}
