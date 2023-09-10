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
        virtual g_err on(const char *path, void *cb);

    protected:
    private:
        static void _dns_task(void *args);
        static esp_err_t _commonHandler(httpd_req_t *req);
        g_err _dns_recv(struct sockaddr_in *premote_addr, char *pusrdata, unsigned short length);
        int _sock_fd;
        esp_netif_t *_netif;
    };

} // namespace gardener

#endif