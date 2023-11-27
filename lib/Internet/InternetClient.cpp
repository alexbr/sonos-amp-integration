#include "Internet.h"

InternetClient::InternetClient() {}

InternetClient::InternetClient(EthernetClient ethClient) {
  this->ethClient = ethClient;
}

InternetClient::InternetClient(WiFiClient wiFiClient) {
  this->wiFiClient = wiFiClient;
}

int InternetClient::connect(IPAddress ip, uint16_t port) {
  if (Internet.useEthernet) {
    return ethClient.connect(ip, port);
  } else {
    return wiFiClient.connect(ip, port);
  }
}

uint8_t InternetClient::connected() {
  if (Internet.useEthernet) {
    return ethClient.connected();
  } else {
    return wiFiClient.connected();
  }
}

int InternetClient::available() {
  if (Internet.useEthernet) {
    return ethClient.available();
  } else {
    return wiFiClient.available();
  }
}

int InternetClient::read() {
  if (Internet.useEthernet) {
    return ethClient.read();
  } else {
    return wiFiClient.read();
  }
}

void InternetClient::stop() {
  if (Internet.useEthernet) {
    ethClient.stop();
  } else {
    wiFiClient.stop();
  }
}

InternetClient::operator bool() {
  if (Internet.useEthernet) {
    return ethClient ? true : false;
  } else {
    return wiFiClient ? true : false;
  }
}

size_t InternetClient::write(const uint8_t *buf, size_t size) {
  if (Internet.useEthernet) {
    return ethClient.write(buf, size);
  } else {
    return wiFiClient.write(buf, size);
  }
}

size_t InternetClient::write(uint8_t b) {
  if (Internet.useEthernet) {
    return ethClient.write(b);
  } else {
    return wiFiClient.write(b);
  }
}

IPAddress InternetClient::remoteIP() {
  if (Internet.useEthernet) {
    return ethClient.remoteIP();
  } else {
    return wiFiClient.remoteIP();
  }
}