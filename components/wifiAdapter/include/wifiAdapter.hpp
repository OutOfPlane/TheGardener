#ifndef G_WIFI_H
#define G_WIFI_H

#include "networkAdapter.hpp"
#include "freertos/event_groups.h"

namespace gardener
{

    class wifiAdapter : public networkAdapter
    {
    public:
        wifiAdapter(const char *name, int maxRetries);
        virtual ~wifiAdapter() {}        
        g_err setSSID(const char* SSID);
        g_err setPWD(const char* PWD);
        g_err startConnect();
        g_err waitConnected();

    protected:
        g_err _start();
        g_err createNetif();
        static void _eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    private:
        int _retryNum = 0;
        int _maxRetries = 5;
        EventGroupHandle_t _event_group;
        wifi_config_t _config;
    };

} // namespace gardener

#endif