#include "httpClient.hpp"
#include "esp_netif.h"
#include "esp_crt_bundle.h"

const char *cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----)EOF";

using namespace gardener;

gardener::httpClient::httpClient(const char *name)
    : gardenObject(name)
{
    memset(&_config, 0, sizeof(_config));
    _config.url = "http://www.google.de";
    // _config.crt_bundle_attach = esp_crt_bundle_attach;
    _config.cert_pem = cert;
    _client = esp_http_client_init(&_config);
}

g_err gardener::httpClient::get(char *responseBuffer, uint16_t responseBufferSize, const char *requestUrl)
{
    int content_length = 0;
    esp_http_client_set_url(_client, requestUrl);
    esp_http_client_set_method(_client, HTTP_METHOD_GET);

    esp_err_t err = esp_http_client_open(_client, 0);
    if (err != ESP_OK)
    {
        G_LOGE("Failed to open HTTP connection: %s", esp_err_to_name(err));
        return G_ERR_NETWORK;
    }
    else
    {
        content_length = esp_http_client_fetch_headers(_client);
        if (content_length < 0)
        {
            G_LOGE("HTTP client fetch headers failed");
            esp_http_client_close(_client);
            return G_ERR_INVALID_RESPONSE;
        }
        else
        {
            int data_read = esp_http_client_read_response(_client, responseBuffer, responseBufferSize);
            if (data_read >= 0)
            {
                responseBuffer[data_read] = 0;
                G_LOGI("HTTP GET Status = %d, content_length = %d",
                       esp_http_client_get_status_code(_client),
                       esp_http_client_get_content_length(_client));
            }
            else
            {
                responseBuffer[0] = 0;
                G_LOGE("Failed to read response");
                esp_http_client_close(_client);
                return G_ERR_NO_DATA;
            }
        }
    }
    esp_http_client_close(_client);

    return G_OK;
}

const char *hexChars = "0123456789ABCDEF";

g_err gardener::httpClient::getPrintf(char *responseBuffer, uint16_t responseBufferSize, const char *requestUrl, ...)
{
    char tmp[256];
    va_list args;
    va_start(args, requestUrl);
    vsprintf(tmp, requestUrl, args);
    va_end(args);
    // urlencode the getPARAMs
    char tmpENC[256];
    char *tmpPtr = tmp;
    char *tmpENCPtr = tmpENC;
    uint8_t paramStart = 0;
    while (*tmpPtr)
    {
        if(*tmpPtr == 0x1B)
            break;
        *tmpENCPtr = *tmpPtr;
        if (paramStart)
        {
            if(*tmpPtr == '=')
                *tmpENCPtr = *tmpPtr;
            else if(*tmpPtr == '&')
                *tmpENCPtr = *tmpPtr;
            else if(isalnum(*tmpPtr) == 0)
            {
                *tmpENCPtr = '%';
                tmpENCPtr++;
                *tmpENCPtr = hexChars[((*tmpPtr) >> 4) & 0xF];
                tmpENCPtr++;
                *tmpENCPtr = hexChars[((*tmpPtr) >> 0) & 0xF];
            }
        }
        if (*tmpPtr == '?')
            paramStart = 1;
        tmpPtr++;
        tmpENCPtr++;
    }
    *tmpENCPtr = '\0';
    G_LOGI("Executing request: %s", tmpENC);
    return get(responseBuffer, responseBufferSize, tmpENC);
}


g_err gardener::httpClient::post(char *body, const char *requestUrl)
{
    int content_length = 0;
    esp_http_client_set_url(_client, requestUrl);
    esp_http_client_set_method(_client, HTTP_METHOD_POST);
    uint16_t dataLen = strlen(body);
    esp_err_t err = esp_http_client_open(_client, dataLen);
    if (err != ESP_OK)
    {
        G_LOGE("Failed to open HTTP connection: %s", esp_err_to_name(err));
        return G_ERR_NETWORK;
    }
    else
    {
        if(dataLen != esp_http_client_write(_client, body, dataLen))
        {
            G_LOGE("HTTP POST transfer failed");
            esp_http_client_close(_client);
            return G_ERR_TRANSFER_INTERRUPTED;
        }

        content_length = esp_http_client_fetch_headers(_client);
        if (content_length < 0)
        {
            G_LOGE("HTTP client fetch headers failed");
            esp_http_client_close(_client);
            return G_ERR_INVALID_RESPONSE;
        }
        else
        {
            char tmp[512];
            int data_read = esp_http_client_read_response(_client, tmp, 512);
            if (data_read >= 0)
            {
                tmp[data_read] = 0;
                G_LOGI("HTTP GET Status = %d, content_length = %d",
                       esp_http_client_get_status_code(_client),
                       esp_http_client_get_content_length(_client));
                G_LOGI("data: %s", tmp);
            }
            else
            {
                tmp[0] = 0;
                G_LOGE("Failed to read response");
                esp_http_client_close(_client);
                return G_ERR_NO_DATA;
            }
        }
    }
    esp_http_client_close(_client);

    return G_OK;
}

g_err gardener::httpClient::postPrintf(char *body, const char *requestUrl, ...)
{
    char tmp[256];
    va_list args;
    va_start(args, requestUrl);
    vsprintf(tmp, requestUrl, args);
    va_end(args);
    // urlencode the getPARAMs
    char tmpENC[256];
    char *tmpPtr = tmp;
    char *tmpENCPtr = tmpENC;
    uint8_t paramStart = 0;
    while (*tmpPtr)
    {
        if(*tmpPtr == 0x1B)
            break;
        *tmpENCPtr = *tmpPtr;
        if (paramStart)
        {
            if(*tmpPtr == '=')
                *tmpENCPtr = *tmpPtr;
            else if(*tmpPtr == '&')
                *tmpENCPtr = *tmpPtr;
            else if(isalnum(*tmpPtr) == 0)
            {
                *tmpENCPtr = '%';
                tmpENCPtr++;
                *tmpENCPtr = hexChars[((*tmpPtr) >> 4) & 0xF];
                tmpENCPtr++;
                *tmpENCPtr = hexChars[((*tmpPtr) >> 0) & 0xF];
            }
        }
        if (*tmpPtr == '?')
            paramStart = 1;
        tmpPtr++;
        tmpENCPtr++;
    }
    *tmpENCPtr = '\0';
    G_LOGI("Executing request: %s", tmpENC);
    return post(body, tmpENC);
}
