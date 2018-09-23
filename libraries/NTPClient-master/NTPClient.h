#pragma once

#include "Arduino.h"

#include <EthernetUdp2.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

#define NTP_TIMEOUT 1000

class NTPClient {
  private:
    EthernetUDP*          _udp = 0;
    bool          _udpSetup       = false;

    const char*   _poolServerName = "time.nist.gov"; // Default time server
    int           _port           = NTP_DEFAULT_LOCAL_PORT;
    int           _timeOffset     = 0;

    unsigned int  _updateInterval = 60000;  // In ms

    unsigned long _currentEpoc    = 0;      // In s
    unsigned long _lastUpdate     = 0;      // In ms

    byte          _packetBuffer[NTP_PACKET_SIZE] = {0};

    int packetSent = 0;

    void          sendNTPPacket();
    bool          parsePacket();

  public:
    NTPClient();
    NTPClient(EthernetUDP& udp);
    NTPClient(EthernetUDP& udp, int timeOffset);
    NTPClient(EthernetUDP& udp, const char* poolServerName);
    NTPClient(EthernetUDP& udp, const char* poolServerName, int timeOffset);
    NTPClient(EthernetUDP& udp, const char* poolServerName, int timeOffset, int updateInterval);

    /**
     * Starts the underlying UDP client with the default local port
     */
    void begin();

    void begin(EthernetUDP* udp);

    /**
     * Starts the underlying UDP client with the specified local port
     */
    void begin(EthernetUDP* udp, int port);

    /**
     * This should be called in the main loop of your application. By default an update from the NTP Server is only
     * made every 60 seconds. This can be configured in the NTPClient constructor.
     *
     * @return true on success, false on failure
     */
    bool update();

    /**
     * This will force the update from the NTP Server.
     *
     * @return true on success, false on failure
     */
    bool forceUpdate();

    unsigned long get();
    int getDay();
    int getHours();
    int getMinutes();
    int getSeconds();

    /**
     * Changes the time offset. Useful for changing timezones dynamically
     */
    void setTimeOffset(int timeOffset);

    /**
     * Set the update interval to another frequency. E.g. useful when the
     * timeOffset should not be set in the constructor
     */
    void setUpdateInterval(int updateInterval);

    /**
     * @return time formatted like `hh:mm:ss`
     */
    String getFormattedTime();

    /**
     * @return time in seconds since Jan. 1, 1970
     */
    unsigned long getEpochTime();

    /**
     * Stops the underlying UDP client
     */
    void end();

    void setPoolServerName(const char* name){
      this->_poolServerName = name;
    }

    const char* getPoolServerName(){
      return this->_poolServerName;
    }
};
