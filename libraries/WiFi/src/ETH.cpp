/*
 ETH.h - espre ETH PHY support.
 Based on WiFi.h from Arduino WiFi shield library.
 Copyright (c) 2011-2014 Arduino.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ETH.h"
#include "lwip/err.h"
#include "lwip/dns.h"

extern void tcpipInit();

ETHClass::ETHClass() : initialized(false), started(false), staticIP(false)
{
}

ETHClass::~ETHClass()
{
}

bool ETHClass::begin(int timeout)
{
    tcpipInit();
    staticIP = false;
    system_event_t evt;
    evt.event_id = SYSTEM_EVENT_ETH_START;
    esp_event_send(&evt);
    unsigned long start = millis();
    while (!_gotIP && millis() - start < timeout)
    {
        // TODO: make this async, check from main loop dhcp status, then switch to static?
        Serial.println("Waiting for DHCP");
        delay(500);
    }
    initialized = true;
    return _gotIP;
}

bool ETHClass::begin(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1)
{
    if (!initialized)
        tcpipInit();
    esp_err_t err = ESP_OK;
    tcpip_adapter_ip_info_t info;

    if (local_ip != (uint32_t)0x00000000)
    {
        info.ip.addr = static_cast<uint32_t>(local_ip);
        info.gw.addr = static_cast<uint32_t>(gateway);
        info.netmask.addr = static_cast<uint32_t>(subnet);
    }
    else
    {
        info.ip.addr = 0;
        info.gw.addr = 0;
        info.netmask.addr = 0;
    }

    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);

    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
    if (err != ERR_OK)
    {
        log_e("ETH IP could not be configured! Error: %d", err);
        return false;
    }
    staticIP = true;

    ip_addr_t d;
    d.type = IPADDR_TYPE_V4;

    if (dns1 != (uint32_t)0x00000000)
    {
        // Set DNS1-Server
        d.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
        dns_setserver(0, &d);
    }

    system_event_t evt;
    evt.event_id = SYSTEM_EVENT_ETH_START;
    esp_event_send(&evt);
    return true;
}

void ETHClass::end()
{
    system_event_t evt;
    evt.event_id = SYSTEM_EVENT_ETH_START;
    esp_event_send(&evt);
    return;
}

IPAddress ETHClass::localIP()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.ip.addr);
}

IPAddress ETHClass::subnetMask()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.netmask.addr);
}

IPAddress ETHClass::gatewayIP()
{
    tcpip_adapter_ip_info_t ip;
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip))
    {
        return IPAddress();
    }
    return IPAddress(ip.gw.addr);
}

IPAddress ETHClass::dnsIP(uint8_t dns_no)
{
    ip_addr_t dns_ip = dns_getserver(dns_no);
    return IPAddress(dns_ip.u_addr.ip4.addr);
}

const char *ETHClass::getHostname()
{
    const char *hostname;
    if (tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_ETH, &hostname))
    {
        return NULL;
    }
    return hostname;
}

bool ETHClass::setHostname(const char *hostname)
{
    return tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostname) == 0;
}

bool ETHClass::fullDuplex()
{
    return w5500.wizphy_getphyduplex();
}

bool ETHClass::linkUp()
{
    
    return w5500.wizphy_getphylink();
}

uint8_t ETHClass::linkSpeed()
{
    return w5500.wizphy_getphyspeed() ? 100 : 10;
}

bool ETHClass::enableIpV6()
{
    return tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_ETH) == 0;
}

IPv6Address ETHClass::localIPv6()
{
    static ip6_addr_t addr;
    if (tcpip_adapter_get_ip6_linklocal(TCPIP_ADAPTER_IF_ETH, &addr))
    {
        return IPv6Address();
    }
    return IPv6Address(addr.addr);
}

uint8_t *macAddress(uint8_t *mac)
{
    if (!mac)
    {
        return NULL;
    }
    w5500.get_mac(mac);
    return mac;
}

String ETHClass::macAddress(void)
{
    uint8_t mac[6];
    char macStr[18] = {0};
    w5500.get_mac(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

extern void tcp_server_task();

void ETHClass::eventStart()
{
    tcpip_adapter_ip_info_t eth_ip;
    uint8_t eth_mac[6];

    esp_read_mac(eth_mac, ESP_MAC_ETH);

    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &eth_ip);
    tcpip_adapter_eth_start(eth_mac, &eth_ip);
}

static bool has_been_disconnected = false;

void ETHClass::eventConnected()
{
    Serial.println("Ethclass connected");
    if (has_been_disconnected)
    {
        has_been_disconnected = false;
        w5500.restart_socket();
        delay(1);
    }

    tcpip_adapter_dhcp_status_t status;

    tcpip_adapter_up(TCPIP_ADAPTER_IF_ETH);

    tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_ETH, &status);

    if (!staticIP)
    {
        tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
    }
    else if (staticIP || status == TCPIP_ADAPTER_DHCP_STOPPED)
    {
        tcpip_adapter_ip_info_t eth_ip;

        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &eth_ip);

        if (!(ip4_addr_isany_val(eth_ip.ip) || ip4_addr_isany_val(eth_ip.netmask)))
        {
            system_event_t evt;

            //notify event
            evt.event_id = SYSTEM_EVENT_ETH_GOT_IP;
            memcpy(&evt.event_info.got_ip.ip_info, &eth_ip, sizeof(tcpip_adapter_ip_info_t));

            esp_event_send(&evt);
        }
        else
        {
            ESP_LOGE("Ethclass", "invalid static ip");
        }
    }
}

void ETHClass::eventIP()
{
    Serial.println("Ethclass got IP");
    // tcp_server_task();
    tcpip_adapter_ip_info_t eth_ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &eth_ip);

    ESP_LOGI("Ethclass", "eth ip: " IPSTR ", mask: " IPSTR ", gw: " IPSTR,
             IP2STR(&eth_ip.ip),
             IP2STR(&eth_ip.netmask),
             IP2STR(&eth_ip.gw));
    _gotIP = true;
    networkChanged = true;
}

void ETHClass::eventDisconnected()
{
    Serial.println("Ethclass disconnected");
    tcpip_adapter_down(TCPIP_ADAPTER_IF_ETH);
    _gotIP = false;
    networkChanged = true;
    has_been_disconnected = true;
}

void ETHClass::eventStop()
{
    Serial.println("Ethclass stop");
}

ETHClass ETH;
