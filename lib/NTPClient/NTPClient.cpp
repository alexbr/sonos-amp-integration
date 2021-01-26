/**
 * UDP NTP Client
 */

#include "NTPClient.h"
#include <Arduino.h>

// time.nist.gov NTP server
IPAddress timeServer(129, 6, 15, 28);

// buffer to hold incoming and outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];

NTPClient::NTPClient(UDP &udp) {
   this->udp = &udp;
   this->port = DEFAULT_UDP_PORT;
}

void NTPClient::begin() { udp->begin(port); }

Date NTPClient::getDate() {
   Date d;
   d.setTime(getTime());
   return d;
}

unsigned long NTPClient::getTime() {
   // Discard previous replies
   while (udp->parsePacket());

   sendNTPpacket(timeServer);

   unsigned long epoch = 0;
   unsigned long beginWait = millis();

   // Loop for a little to see if a reply is available
   while (millis() - beginWait < 1000) {
      if (udp->parsePacket() >= NTP_PACKET_SIZE) {
         udp->read(packetBuffer, NTP_PACKET_SIZE);

         // The timestamp starts at byte 40 of the received packet and is four
         // bytes, or two words, long. First, esxtract the two words:
         unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
         unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

         // Combine the four bytes (two words) into a long integer
         // this is NTP time (seconds since Jan 1 1900):
         unsigned long secsSince1900 = highWord << 16 | lowWord;

         // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
         const unsigned long seventyYears = 2208988800UL;
         epoch = secsSince1900 - seventyYears;
         epoch = epoch - (7 * 3600);
         Serial.println(epoch);
      }
   }

   return epoch;
}

// send an NTP request to the time server at the given address
unsigned long NTPClient::sendNTPpacket(IPAddress &address) {
   // set all bytes in the buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   // (see URL above for details on the packets)
   packetBuffer[0] = 0b11100011; // LI, Version, Mode
   packetBuffer[1] = 0;          // Stratum, or type of clock
   packetBuffer[2] = 6;          // Polling Interval
   packetBuffer[3] = 0xEC;       // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12] = 49;
   packetBuffer[13] = 0x4E;
   packetBuffer[14] = 49;
   packetBuffer[15] = 52;

   // all NTP fields have been given values, now
   // you can send a packet requesting a timestamp:
   udp->beginPacket(address, 123); // NTP requests are to port 123
   udp->write(packetBuffer, NTP_PACKET_SIZE);
   udp->endPacket();
}