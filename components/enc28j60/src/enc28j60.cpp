#include "enc28j60.hpp"
#include "esp_event.h"
#include "esp_eth_phy.h"
#include "esp_eth_mac.h"
#include "esp_eth_com.h"
#if CONFIG_IDF_TARGET_ESP32
#include "soc/emac_ext_struct.h"
#include "soc/rtc.h"
// #include "soc/io_mux_reg.h"
// #include "hal/gpio_hal.h"
#endif

#include "lwip/err.h"
#include "lwip/dns.h"

// extern void tcpipInit();

extern "C"
{
    esp_eth_mac_t *enc28j60_begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPI_CLOCK_MHZ, int SPI_HOST);
#include "enc28j60.h"
}

using namespace gardener;

enc28j60::enc28j60(const char *name, int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int SPI_CLOCK_MHZ, int SPI_HOST, ioPin *RESET_PIN)
    : networkAdapter(name), _MISO_GPIO(MISO_GPIO), _MOSI_GPIO(MOSI_GPIO), _SCLK_GPIO(SCLK_GPIO), _CS_GPIO(CS_GPIO), _SPI_CLOCK_MHZ(SPI_CLOCK_MHZ), _SPI_HOST(SPI_HOST), _RESET_PIN(RESET_PIN)
{
}

g_err gardener::enc28j60::_start()
{
    
    
    // tcpipInit();

    uint8_t ENC28J60_Default_Mac[6] = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};

    esp_efuse_mac_get_default(ENC28J60_Default_Mac);

    esp_base_mac_addr_set(ENC28J60_Default_Mac);

    tcpip_adapter_set_default_eth_handlers();

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);

    esp_eth_mac_t *eth_mac = enc28j60_begin(_MISO_GPIO, _MOSI_GPIO, _SCLK_GPIO, _CS_GPIO, -1, _SPI_CLOCK_MHZ, _SPI_HOST);

    if (eth_mac == NULL)
    {
        G_LOGE("esp_eth_mac_new_esp32 failed");
        return G_ERR_INTERNAL;
    }

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.autonego_timeout_ms = 0; // ENC28J60 doesn't support auto-negotiation
    phy_config.reset_gpio_num = -1;     // ENC28J60 doesn't have a pin to reset internal PHY
    esp_eth_phy_t *eth_phy = esp_eth_phy_new_enc28j60(&phy_config);

    if (eth_phy == NULL)
    {
        G_LOGE("esp_eth_phy_new failed");
        return G_ERR_INTERNAL;
    }

    eth_handle = NULL;
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(eth_mac, eth_phy);
    // eth_config.on_lowlevel_init_done = on_lowlevel_init_done;
    // eth_config.on_lowlevel_deinit_done = on_lowlevel_deinit_done;
    if (esp_eth_driver_install(&eth_config, &eth_handle) != ESP_OK || eth_handle == NULL)
    {
        G_LOGE("esp_eth_driver_install failed");
        return G_ERR_INTERNAL;
    }

    /* ENC28J60 doesn't burn any factory MAC address, we need to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
    */
    eth_mac->set_addr(eth_mac, ENC28J60_Default_Mac);

    // ENC28J60 Errata #1 check
    if (emac_enc28j60_get_chip_info(eth_mac) < ENC28J60_REV_B5 && _SPI_CLOCK_MHZ < 8)
    {
        G_LOGE("SPI frequency must be at least 8 MHz for chip revision less than 5");
        return G_ERR_INVALID_ARGS;
    }

    /* attach Ethernet driver to TCP/IP stack */
    if (esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)) != ESP_OK)
    {
        G_LOGE("esp_netif_attach failed");
        return G_ERR_INTERNAL;
    }

    if (esp_eth_start(eth_handle) != ESP_OK)
    {
        G_LOGE("esp_eth_start failed");
        return G_ERR_INTERNAL;
    }

    // holds a few microseconds to let DHCP start and enter into a good state
    // FIX ME -- adresses issue https://github.com/espressif/arduino-esp32/issues/5733
    vTaskDelay(50 / portTICK_RATE_MS);

    return G_OK;
}

g_err gardener::enc28j60::createNetif()
{
    return G_OK;
}

// bool ENC28J60Class::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
// {
//     esp_err_t err = ESP_OK;
//     tcpip_adapter_ip_info_t info;

//     if (static_cast<uint32_t>(local_ip) != 0)
//     {
//         info.ip.addr = static_cast<uint32_t>(local_ip);
//         info.gw.addr = static_cast<uint32_t>(gateway);
//         info.netmask.addr = static_cast<uint32_t>(subnet);
//     }
//     else
//     {
//         info.ip.addr = 0;
//         info.gw.addr = 0;
//         info.netmask.addr = 0;
//     }

//     err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
//     if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED)
//     {
//         log_e("DHCP could not be stopped! Error: %d", err);
//         return false;
//     }

//     err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
//     if (err != ERR_OK)
//     {
//         log_e("STA IP could not be configured! Error: %d", err);
//         return false;
//     }

//     if (info.ip.addr)
//     {
//         staticIP = true;
//     }
//     else
//     {
//         err = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
//         if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED)
//         {
//             log_w("DHCP could not be started! Error: %d", err);
//             return false;
//         }
//         staticIP = false;
//     }

//     ip_addr_t d;
//     d.type = IPADDR_TYPE_V4;

//     if (static_cast<uint32_t>(dns1) != 0)
//     {
//         // Set DNS1-Server
//         d.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
//         dns_setserver(0, &d);
//     }

//     if (static_cast<uint32_t>(dns2) != 0)
//     {
//         // Set DNS2-Server
//         d.u_addr.ip4.addr = static_cast<uint32_t>(dns2);
//         dns_setserver(1, &d);
//     }

//     return true;
// }

// IPAddress ENC28J60Class::localIP()
// {
//     tcpip_adapter_ip_info_t ip;
//     if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
//     {
//         return IPAddress();
//     }
//     return IPAddress(ip.ip.addr);
// }

// IPAddress ENC28J60Class::subnetMask()
// {
//     tcpip_adapter_ip_info_t ip;
//     if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
//     {
//         return IPAddress();
//     }
//     return IPAddress(ip.netmask.addr);
// }

// IPAddress ENC28J60Class::gatewayIP()
// {
//     tcpip_adapter_ip_info_t ip;
//     if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
//     {
//         return IPAddress();
//     }
//     return IPAddress(ip.gw.addr);
// }

// IPAddress ENC28J60Class::dnsIP(uint8_t dns_no)
// {
//     const ip_addr_t *dns_ip = dns_getserver(dns_no);
//     return IPAddress(dns_ip->u_addr.ip4.addr);
// }

// IPAddress ENC28J60Class::broadcastIP()
// {
//     tcpip_adapter_ip_info_t ip;
//     if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
//     {
//         return IPAddress();
//     }
//     return WiFiGenericClass::calculateBroadcast(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
// }

// IPAddress ENC28J60Class::networkID()
// {
//     tcpip_adapter_ip_info_t ip;
//     if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
//     {
//         return IPAddress();
//     }
//     return WiFiGenericClass::calculateNetworkID(IPAddress(ip.gw.addr), IPAddress(ip.netmask.addr));
// }

// uint8_t ENC28J60Class::subnetCIDR()
// {
//     tcpip_adapter_ip_info_t ip;
//     if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
//     {
//         return (uint8_t)0;
//     }
//     return WiFiGenericClass::calculateSubnetCIDR(IPAddress(ip.netmask.addr));
// }

// const char *ENC28J60Class::getHostname()
// {
//     const char *hostname;
//     if (tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_ETH, &hostname))
//     {
//         return NULL;
//     }
//     return hostname;
// }

// bool ENC28J60Class::setHostname(const char *hostname)
// {
//     return tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostname) == 0;
// }

// bool ENC28J60Class::fullDuplex()
// {
//     return true; // todo: do not see an API for this
// }

// bool ENC28J60Class::linkUp()
// {
//     return eth_link == ETH_LINK_UP;
// }

// uint8_t ENC28J60Class::linkSpeed()
// {
//     eth_speed_t link_speed;
//     esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, &link_speed);
//     return (link_speed == ETH_SPEED_10M) ? 10 : 100;
// }

// bool ENC28J60Class::enableIpV6()
// {
//     return tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_ETH) == 0;
// }

// IPv6Address ENC28J60Class::localIPv6()
// {
//     static ip6_addr_t addr;
//     if (tcpip_adapter_get_ip6_linklocal(TCPIP_ADAPTER_IF_ETH, &addr))
//     {
//         return IPv6Address();
//     }
//     return IPv6Address(addr.addr);
// }

// uint8_t *ENC28J60Class::macAddress(uint8_t *mac)
// {
//     if (!mac)
//     {
//         return NULL;
//     }
//     esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac);
//     return mac;
// }

// String ENC28J60Class::macAddress(void)
// {
//     uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
//     char macStr[18] = {0};
//     macAddress(mac);
//     sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//     return String(macStr);
// }
