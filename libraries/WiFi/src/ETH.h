/*
 ETH.h - espre ETH PHY support.
 Based on WiFi.h from Ardiono WiFi shield library.
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

#ifndef _ETH_H_
#define _ETH_H_

#include "WiFi.h"
#include "w5500.h"


class ETHClass {
    private:
        bool initialized;
        bool started;
        bool staticIP;
        
    public:
        ETHClass();
        ~ETHClass();

        bool begin(int timeout = 30000);

        bool begin(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000);

        void end();
        const char * getHostname();
        bool setHostname(const char * hostname);

        bool fullDuplex();
        bool linkUp();
        uint8_t linkSpeed();

        bool enableIpV6();
        IPv6Address localIPv6();

        IPAddress localIP();
        IPAddress subnetMask();
        IPAddress gatewayIP();
        IPAddress dnsIP(uint8_t dns_no = 0);

        uint8_t * macAddress(uint8_t* mac);
        String macAddress();

        bool networkChanged = false;

        void eventStart();
        void eventIP();
        void eventConnected();
        void eventDisconnected();
        void eventStop();

        bool _gotIP = false;

        friend class WiFiClient;
        friend class WiFiServer;
};

extern ETHClass ETH;

#endif /* _ETH_H_ */
