#ifndef G_ESP32_NVS_H
#define G_ESP32_NVS_H

#include "configStorage.hpp"
#include "nvs.h"

namespace gardener
{

    class esp32nvs : public configStorage
    {
    public:
        esp32nvs(const char *name);
        ~esp32nvs();
        g_err openNamespace(const char *name);
        g_err close();
        g_err readUI8(const char *key, uint8_t &value);
        g_err readUI16(const char *key, uint16_t &value);
        g_err readUI32(const char *key, uint32_t &value);
        g_err readUI64(const char *key, uint64_t &value);
        g_err readI8(const char *key, int8_t &value);
        g_err readI16(const char *key, int16_t &value);
        g_err readI32(const char *key, int32_t &value);
        g_err readI64(const char *key, int64_t &value);
        g_err readStr(const char *key, char *value, size_t &length);

        g_err writeUI8(const char *key, uint8_t value);
        g_err writeUI16(const char *key, uint16_t value);
        g_err writeUI32(const char *key, uint32_t value);
        g_err writeUI64(const char *key, uint64_t value);
        g_err writeI8(const char *key, int8_t value);
        g_err writeI16(const char *key, int16_t value);
        g_err writeI32(const char *key, int32_t value);
        g_err writeI64(const char *key, int64_t value);
        g_err writeStr(const char *key, const char *value);

    protected:
    private:
        nvs_handle_t _handle;
        bool _isOpen = false;
    };
} // namespace gardener

#endif