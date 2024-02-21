#ifndef G_HTTP_CLIENT_H
#define G_HTTP_CLIENT_H

#include "gardenObject.hpp"
#include "esp_http_client.h"

namespace gardener
{
    class httpClient : public gardenObject
    {
    public:
        httpClient(const char *name);
        virtual ~httpClient() {}
        g_err get(char *responseBuffer, uint16_t responseBufferSize, const char *requestUrl);
        g_err getPrintf(char *responseBuffer, uint16_t responseBufferSize, const char *requestUrl, ...);
        g_err post(char *body, const char *requestUrl);
        g_err postPrintf(char *body, const char *requestUrl, ...);

    protected:
    private:
        esp_http_client_config_t _config;
        esp_http_client_handle_t _client;
    };

} // namespace gardener

#endif