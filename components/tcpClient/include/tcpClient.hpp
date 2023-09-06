#ifndef G_TCP_CLIENT_H
#define G_TCP_CLIENT_H

#include <gardenObject.hpp>
#include <sys/socket.h>

namespace gardener
{
    class tcpClient : public gardenObject
    {
    public:
        tcpClient(const char *name, const char *IP, uint16_t port, int addr_family = AF_INET);
        ~tcpClient();
        g_err openSocket();
        void closeSocket();
        g_err write(const char *data);

    protected:
    private:
        int _addr_family;
        char *_IP;
        struct sockaddr_in _dest_addr;
        int _sock;
    };

} // namespace gardener

#endif