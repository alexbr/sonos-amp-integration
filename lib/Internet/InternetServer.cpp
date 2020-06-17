#include "Internet.h"
#include <WiFiNINA.h>

void InternetServer::begin() {
    if (Internet.useEthernet) {
        ethServer.begin();
    } else {
        Serial.print("beginning wifi server on ");
        Serial.println(port);
        wiFiServer.begin();
        Serial.println("server started");
    }
}

InternetClient InternetServer::available() {
    if (Internet.useEthernet) {
       EthernetClient client = ethServer.available();
       return InternetClient(client);
    } else {
       WiFiClient client = wiFiServer.available();
       return InternetClient(client);
    }
}