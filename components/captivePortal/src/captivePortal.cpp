#include "captivePortal.hpp"
#include "lwip/err.h"
#include "esp_netif.h"

#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_LEN 512

#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_FLAG_QR (1 << 7)
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_FLAG_AA (1 << 2)
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_FLAG_TC (1 << 1)
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_FLAG_RD (1 << 0)

#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_A 1
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_NS 2
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_CNAME 5
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_SOA 6
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_WKS 11
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_PTR 12
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_HINFO 13
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_MINFO 14
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_MX 15
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_TXT 16
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_URI 256

#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QCLASS_IN 1
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QCLASS_ANY 255
#define WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QCLASS_URI 256

typedef struct __attribute__((packed))
{
    uint16_t id;
    uint8_t flags;
    uint8_t rcode;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} DnsHeader;

typedef struct __attribute__((packed))
{
    uint16_t type;
    uint16_t cl;
} DnsQuestionFooter;

typedef struct __attribute__((packed))
{
    uint16_t type;
    uint16_t cl;
    uint32_t ttl;
    uint16_t rdlength;
} DnsResourceFooter;

typedef struct __attribute__((packed))
{
    uint16_t prio;
    uint16_t weight;
} DnsUriHdr;

using namespace gardener;

captivePortal::captivePortal(const char *name, esp_netif_t *netif)
    : gardenObject(name), _netif(netif)
{
}

captivePortal::~captivePortal()
{
}

g_err captivePortal::start()
{
    xTaskCreate(_dns_task, (const char *)"dns_task", 8192, this, 3, NULL);

    g_err erg = G_OK;

    /** HTTP server */
    G_LOGI("Starting HTTP Server...");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = true;

    erg = g_err_translate(httpd_start(&_server, &config));
    if (erg != G_OK)
        return erg;

    G_LOGI("Started HTTP Server.");

    G_LOGI("Registering HTTP server URI handlers...");

    /** use 404 Handler for redirects URI handler */
    httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, _commonHandler);

    G_LOGI("Registered HTTP server URI handlers.");

    return G_OK;
}

void gardener::captivePortal::stop()
{
}

g_err gardener::captivePortal::on(const char *path, esp_err_t (*handler)(httpd_req_t *r))
{
    /** URI handler */
    httpd_uri_t common_get_uri = {
        .uri = path,
        .method = HTTP_GET,
        .handler = handler,
        .user_ctx = this};

    return g_err_translate(httpd_register_uri_handler(_server, &common_get_uri));
}

void gardener::captivePortal::_dns_task(void *args)
{
    captivePortal *prtl = (captivePortal *)args;
    const char *_name = prtl->_name;

    struct sockaddr_in server_addr;
    uint32_t ret;
    struct sockaddr_in from;
    socklen_t fromlen;
    char udp_msg[WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_LEN];

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(53);
    server_addr.sin_len = sizeof(server_addr);

    do
    {
        prtl->_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (prtl->_sock_fd == -1)
        {
            G_LOGE("dns_task failed to create sock!, retrying...");
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while (prtl->_sock_fd == -1);

    do
    {
        ret = bind(prtl->_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret != 0)
        {
            G_LOGE("dns_task failed to bind sock!");
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while (ret != 0);

    G_LOGI("DNS initialized.");

    while (1)
    {
        memset(&from, 0, sizeof(from));
        fromlen = sizeof(struct sockaddr_in);
        ret = recvfrom(prtl->_sock_fd, (uint8_t *)udp_msg, WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_LEN, 0, (struct sockaddr *)&from, (socklen_t *)&fromlen);
        if (ret > 0)
            prtl->_dns_recv(&from, udp_msg, ret);
    }

    close(prtl->_sock_fd);
    vTaskDelete(NULL);
}

esp_err_t gardener::captivePortal::_commonHandler(httpd_req_t *req, httpd_err_code_t error)
{
    // captivePortal *prtl = (captivePortal *)req->user_ctx;
    const char *_name = "404 Handler";

    size_t req_hdr_host_len = httpd_req_get_hdr_value_len(req, "Host");

    char req_hdr_host_val[req_hdr_host_len + 1];

    esp_err_t res = httpd_req_get_hdr_value_str(req, "Host", (char *)&req_hdr_host_val, sizeof(char) * req_hdr_host_len + 1);
    if (res != ESP_OK)
    {
        G_LOGE("failed getting HOST header value: %d", res);

        switch (res)
        {
        case ESP_ERR_NOT_FOUND:
            G_LOGE("failed getting HOST header value: ESP_ERR_NOT_FOUND: Key not found: %d", res);
            break;

        case ESP_ERR_INVALID_ARG:
            G_LOGE("failed getting HOST header value: ESP_ERR_INVALID_ARG: Null arguments: %d", res);
            break;

        case ESP_ERR_HTTPD_INVALID_REQ:
            G_LOGE("failed getting HOST header value: ESP_ERR_HTTPD_INVALID_REQ: Invalid HTTP request pointer: %d", res);
            break;

        case ESP_ERR_HTTPD_RESULT_TRUNC:
            G_LOGE("failed getting HOST header value: ESP_ERR_HTTPD_RESULT_TRUNC: Value string truncated: %d", res);
            break;

        default:
            break;
        }
    }

    G_LOGI("Got HOST header value: %s", req_hdr_host_val);

    const char *correct_host = "gravi.plant";

    if (strncmp(req_hdr_host_val, correct_host, strlen(correct_host)) != 0)
    {
        const char resp[] = "302 Temporary Redirect";

        G_LOGI("Detected redirect trigger HOST: %s", req_hdr_host_val);

        httpd_resp_set_status(req, resp);
        httpd_resp_set_hdr(req, "Location", "http://gravi.plant/status");
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_send(req, "Redirect to captive Portal", HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        G_LOGI("No redirect needed for HOST: %s", req_hdr_host_val);

        
        const char *resp_str = "Hello World";
        httpd_resp_send(req, resp_str, strlen(resp_str));
    }

    return ESP_OK;
}

char *label_to_str(char *packet, char *labelPtr, int packetSz, char *res, int resMaxLen)
{
    int i, j, k;
    char *endPtr = NULL;
    i = 0;
    do
    {
        if ((*labelPtr & 0xC0) == 0)
        {
            j = *labelPtr++; // skip past length
            // Add separator period if there already is data in res
            if (i < resMaxLen && i != 0)
                res[i++] = '.';
            // Copy label to res
            for (k = 0; k < j; k++)
            {
                if ((labelPtr - packet) > packetSz)
                    return NULL;
                if (i < resMaxLen)
                    res[i++] = *labelPtr++;
            }
        }
        else if ((*labelPtr & 0xC0) == 0xC0)
        {
            // // Compressed label pointer
            // endPtr = labelPtr + 2;
            // int offset = ntohs(((uint16_t)labelPtr)) & 0x3FFF;
            // // Check if offset points to somewhere outside of the packet
            // if (offset > packetSz)
            //     return NULL;
            // labelPtr = &packet[offset];
            printf("Landed here\r\n");
            return NULL;
        }
        // check for out-of-bound-ness
        if ((labelPtr - packet) > packetSz)
            return NULL;
    } while (*labelPtr != 0);
    res[i] = 0; // zero-terminate
    if (endPtr == NULL)
        endPtr = labelPtr + 1;
    return endPtr;
}

char *str_to_label(char *str, char *label, int maxLen)
{
    char *len = label;   // ptr to len byte
    char *p = label + 1; // ptr to next label byte to be written
    while (1)
    {
        if (*str == '.' || *str == 0)
        {
            *len = ((p - len) - 1); // write len of label bit
            len = p;                // pos of len for next part
            p++;                    // data ptr is one past len
            if (*str == 0)
                break; // done
            str++;
        }
        else
        {
            *p++ = *str++; // copy byte
                           // if ((p-label)>maxLen) return NULL;	// check out of bounds
        }
    }
    *len = 0;
    return p; // ptr to first free byte in resp
}

g_err gardener::captivePortal::_dns_recv(sockaddr_in *premote_addr, char *pusrdata, unsigned short length)
{
    char buff[WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_LEN];
    char reply[WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_LEN];
    int i;
    char *rend = &reply[length];
    char *p = pusrdata;
    DnsHeader *hdr = (DnsHeader *)p;
    DnsHeader *rhdr = (DnsHeader *)&reply[0];
    p += sizeof(DnsHeader);

    // Some sanity checks:
    if (length > WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_LEN)
        return G_ERR_INTERNAL; // Packet is longer than DNS implementation allows
    if (length < sizeof(DnsHeader))
        return G_ERR_INTERNAL; // Packet is too short
    if (hdr->ancount || hdr->nscount || hdr->arcount)
        return G_ERR_INTERNAL; // this is a reply, don't know what to do with it
    if (hdr->flags & WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_FLAG_TC)
        return G_ERR_INVALID_STATE; // truncated, can't use this
    // Reply is basically the request plus the needed data
    memcpy(reply, pusrdata, length);
    rhdr->flags |= WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_FLAG_QR;

    for (i = 0; i < ntohs(hdr->qdcount); i++)
    {
        // Grab the labels in the q string
        p = label_to_str(pusrdata, p, length, buff, sizeof(buff));
        if (p == NULL)
            return G_ERR_INTERNAL;
        DnsQuestionFooter *qf = (DnsQuestionFooter *)p;
        p += sizeof(DnsQuestionFooter);

        G_LOGI("DNS: WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_Q (type 0x%X cl 0x%X) for %s\n", ntohs(qf->type), ntohs(qf->cl), buff);

        if (ntohs(qf->type) == WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_A)
        {
            // They want to know the IPv4 address of something.
            // Build the response.

            rend = str_to_label(buff, rend, sizeof(reply) - (rend - reply)); // Add the label
            if (rend == NULL)
                return G_ERR_INTERNAL;
            DnsResourceFooter *rf = (DnsResourceFooter *)rend;
            rend += sizeof(DnsResourceFooter);
            rf->type = htons(WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_A);
            rf->cl = htons(WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QCLASS_IN);
            rf->ttl = htonl(0);
            rf->rdlength = htons(4);
            // Grab the current IP of the softap interface

            esp_netif_ip_info_t info;
            esp_netif_get_ip_info(_netif, &info);
            *rend++ = ip4_addr1(&info.ip);
            *rend++ = ip4_addr2(&info.ip);
            *rend++ = ip4_addr3(&info.ip);
            *rend++ = ip4_addr4(&info.ip);
            rhdr->ancount = htons(ntohs(rhdr->ancount) + 1);
        }
        else if (ntohs(qf->type) == WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_NS)
        {
            // Give ns server. Basically can be whatever we want because it'll get resolved to our IP later anyway.
            rend = str_to_label(buff, rend, sizeof(reply) - (rend - reply)); // Add the label
            DnsResourceFooter *rf = (DnsResourceFooter *)rend;
            rend += sizeof(DnsResourceFooter);
            rf->type = htons(WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_NS);
            rf->cl = htons(WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QCLASS_IN);
            rf->ttl = htonl(0);
            rf->rdlength = htons(4);
            *rend++ = 2;
            *rend++ = 'n';
            *rend++ = 's';
            *rend++ = 0;
            rhdr->ancount = htons(ntohs(rhdr->ancount) + 1);
        }
        else if (ntohs(qf->type) == WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_URI)
        {
            // Give uri to us
            rend = str_to_label(buff, rend, sizeof(reply) - (rend - reply)); // Add the label
            DnsResourceFooter *rf = (DnsResourceFooter *)rend;
            rend += sizeof(DnsResourceFooter);
            DnsUriHdr *uh = (DnsUriHdr *)rend;
            rend += sizeof(DnsUriHdr);
            rf->type = htons(WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QTYPE_URI);
            rf->cl = htons(WIFI_CAPTIVE_PORTAL_ESP_IDF_DNS_QCLASS_URI);
            rf->ttl = htonl(0);
            rf->rdlength = htons(4 + 16);
            uh->prio = htons(10);
            uh->weight = htons(1);
            memcpy(rend, "http://esp.nonet", 16);
            rend += 16;
            rhdr->ancount = htons(ntohs(rhdr->ancount) + 1);
        }
    }
    // Send the response
    sendto(_sock_fd, (uint8_t *)reply, rend - reply, 0, (struct sockaddr *)premote_addr, sizeof(struct sockaddr_in));
    return G_OK;
}
