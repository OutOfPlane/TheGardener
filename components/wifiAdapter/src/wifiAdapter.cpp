#include "wifiAdapter.hpp"
#include "esp_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1


using namespace gardener;

wifiAdapter::wifiAdapter(const char *name, int maxRetries)
    : networkAdapter(name), _maxRetries(maxRetries)
{
    memset(&_config, 0, sizeof(_config));
}

g_err wifiAdapter::setSSID(const char *SSID)
{
    // TODO: handle errors
    esp_wifi_get_config(WIFI_IF_STA, &_config);
    strcpy((char *)_config.sta.ssid, SSID);
    esp_wifi_set_config(WIFI_IF_STA, &_config);
    return G_OK;
}

g_err gardener::wifiAdapter::getSSID(char *SSID)
{
    esp_wifi_get_config(WIFI_IF_STA, &_config);
    strcpy(SSID, (char *)_config.sta.ssid);
    return G_OK;
}

g_err gardener::wifiAdapter::setPWD(const char *PWD)
{
    // TODO: handle errors
    esp_wifi_get_config(WIFI_IF_STA, &_config);
    strcpy((char *)_config.sta.password, PWD);
    esp_wifi_set_config(WIFI_IF_STA, &_config);
    return G_OK;
}

g_err gardener::wifiAdapter::startConnect()
{
    // TODO: handle errors
    if (_wifiStarted)
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_connect());
    }else{
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    
    _wifiStarted = true;
    return G_OK;
}

g_err wifiAdapter::waitConnected()
{
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        char ssid[65];
        getSSID(ssid);
        G_LOGI("connected to ap SSID:%s",
               ssid);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        char ssid[65];
        getSSID(ssid);
        G_LOGI("Failed to connect to SSID:%s",
               ssid);
    }
    else
    {
        G_LOGE("UNEXPECTED EVENT");
    }
    // TODO: do correctly
    return G_OK;
}

g_err gardener::wifiAdapter::startSTA(const char *SSID, const char *PWD)
{
    g_err erg = G_ERR_UNK;

    if (_wifiStarted)
    {
        esp_wifi_stop();
    }

    _ap_netif = esp_netif_create_default_wifi_ap();
    assert(_ap_netif);

    esp_netif_ip_info_t ip_info;

    /** NOTE: This is where you set the access point (AP) IP address
        and gateway address. It has to be a class A internet address
        otherwise the captive portal sign-in prompt won't show up	on
        Android when you connect to the access point. */
    IP4_ADDR(&ip_info.ip, 192, 168, 0, 10);
    IP4_ADDR(&ip_info.gw, 192, 168, 0, 10);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    esp_netif_dhcps_stop(_ap_netif);
    esp_netif_set_ip_info(_ap_netif, &ip_info);
    esp_netif_dhcps_start(_ap_netif);

    if (!_wifiInitDone)
    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        erg = g_err_translate(esp_wifi_init(&cfg));
        if (erg != G_OK)
            return erg;
        _wifiInitDone = true;
    }

    memset(&(_config.ap), 0, sizeof(_config.ap));
    memcpy(&(_config.ap.ssid), SSID, strlen(SSID));
    _config.ap.ssid_len = strlen(SSID);
    _config.ap.channel = 1;
    memcpy(&(_config.ap.password), PWD, strlen(PWD));
    _config.ap.max_connection = 4;
    if (strlen(PWD) > 0)
    {
        _config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    }
    else
    {
        _config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &_config));

    G_LOGI("starting WiFi access point: SSID: %s password:%s channel: %d",
           SSID, PWD, 1);

    ESP_ERROR_CHECK(esp_wifi_start());
    _wifiStarted = true;
    return erg;
}

esp_netif_t *gardener::wifiAdapter::getNetIFAP()
{
    return _ap_netif;
}

g_err wifiAdapter::_start()
{
    g_err erg = G_ERR_UNK;

    _event_group = xEventGroupCreate();

    if (!_wifiInitDone)
    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        erg = g_err_translate(esp_wifi_init(&cfg));
        if (erg != G_OK)
            return erg;
        _wifiInitDone = true;
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        _eventHandler,
                                                        this,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        _eventHandler,
                                                        this,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &_config));

    G_LOGI("wifi_init_sta finished.");

    return G_OK;
}

g_err wifiAdapter::createNetif()
{
    _netif = esp_netif_create_default_wifi_sta();
    if (_netif)
    {
        return G_OK;
    }
    return G_ERR_UNK; // API does not return error codes
}

void wifiAdapter::_eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (!arg)
    {
        return;
    }

    wifiAdapter *ctx = (wifiAdapter *)arg;
    const char *_name = ctx->_name;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ctx->_state = G_NETWORK_CONNECTING;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (ctx->_retryNum < ctx->_maxRetries)
        {
            esp_wifi_connect();
            ctx->_retryNum++;
            G_LOGI("retry to connect to the AP");
            ctx->_state = G_NETWORK_RECONNECTING;
        }
        else
        {
            xEventGroupSetBits(ctx->_event_group, WIFI_FAIL_BIT);
            ctx->_state = G_NETWORK_DISCONNECTED;
        }
        G_LOGI("connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        G_LOGI("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ctx->_retryNum = 0;
        ctx->_state = G_NETWORK_CONNECTED;
        xEventGroupSetBits(ctx->_event_group, WIFI_CONNECTED_BIT);
    }
}
