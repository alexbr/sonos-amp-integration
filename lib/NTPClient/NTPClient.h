#include <WiFiUdp.h>

// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48
#define DEFAULT_UDP_PORT 2390 
#define SEVENTY_YEARS 2208988800UL

struct Date {
   unsigned long time = 0;

   void setTime(unsigned long time) { 
      this->time = time;
   }
   
   unsigned long getTime() {
      return time;
   }

   uint8_t getHours() {
      return ((time % 86400L) / 3600) - 7; // PST, no daylight savings for now!!!
   }

   uint8_t getMinutes() { return (time % 3600) / 60; }

   int8_t getSeconds() { return time % 60; }
};

class NTPClient {
 private:
   UDP *udp;
   int port;
   unsigned long sendNTPpacket(IPAddress &address);

 public:
   NTPClient(UDP &udp);
   void begin();
   Date getDate();
   unsigned long getTime();
};