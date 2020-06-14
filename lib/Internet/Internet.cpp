#include "Internet.h"
#include "Dns.h"

static bool InternetClass::useEthernet = true;

static void InternetClass::begin(char *ssid, char *pass) {
    begin(ssid, pass, nullptr);
}

static void InternetClass::begin(char *ssid, char *pass, IPAddress ip) {
   if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("WiFi module failed!");
      return;
   }

   String fv = WiFi.firmwareVersion();
   String latestFv = WIFI_FIRMWARE_LATEST_VERSION;

   if (fv < latestFv) {
      Serial.println("Please upgrade the firmware");
      Serial.println(fv);
      Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
   }

   if (ip != nullptr) {
       WiFi.config(ip);
   }

   int status = WL_IDLE_STATUS;
   while (status != WL_CONNECTED) {
      status = WiFi.begin(ssid, pass);
      delay(5000);
   }

   useEthernet = false;

   Serial.print("Connected to ");
   Serial.println(ssid);
}

static int InternetClass::begin(uint8_t *mac) {
   useEthernet = true;
   return Ethernet.begin(mac);
}

static void InternetClass::begin(uint8_t *mac, IPAddress ip) {
   useEthernet = true;
   Ethernet.begin(mac, ip);
}

static void InternetClass::hostByName(IPAddress &ip, const char *host) {
   if (useEthernet) {
      DNSClient dns;
      dns.begin(Ethernet.dnsServerIP());
      dns.getHostByName(host, ip);
   } else {
      WiFi.hostByName(host, ip);
   }
}

static IPAddress InternetClass::localIP() {
   if (useEthernet) {
      return Ethernet.localIP();
   } else {
      return WiFi.localIP();
   }
}

InternetClass InternetClass;