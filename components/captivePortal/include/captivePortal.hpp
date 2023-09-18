#ifndef G_CAPTIVE_PORTAL_H
#define G_CAPTIVE_PORTAL_H

#include "gardenObject.hpp"
#include "lwip/sockets.h"
#include "esp_netif.h"
#include "esp_http_server.h"

namespace gardener
{
    class captivePortal : public gardenObject
    {
    public:
        captivePortal(const char *name, esp_netif_t *netif);
        virtual ~captivePortal();
        virtual g_err start();
        virtual void stop();
        virtual g_err onGet(const char *path, esp_err_t (*handler)(httpd_req_t *r));
        virtual g_err onPost(const char *path, esp_err_t (*handler)(httpd_req_t *r));

    protected:
    private:
        static void _dns_task(void *args);
        static esp_err_t _commonHandler(httpd_req_t *req, httpd_err_code_t error);
        g_err _dns_recv(struct sockaddr_in *premote_addr, char *pusrdata, unsigned short length);
        int _sock_fd;
        esp_netif_t *_netif;
        httpd_handle_t _server = NULL;
    };

} // namespace gardener

#endif