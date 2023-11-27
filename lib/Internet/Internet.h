#ifndef internet_h_
#define internet_h_

#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>
#include <WiFiNINA.h>

class InternetClient;
class InternetServer;

class InternetClass {
  // private:
public:
  static bool useEthernet;
  static bool initialized;

  static void begin(char *ssid, char *pass);
  static void begin(char *ssid, char *pass, IPAddress ip);
  static int begin(uint8_t *mac);
  static void begin(uint8_t *mac, IPAddress ip);
  static bool connected();
  static void hostByName(IPAddress &ip, const char *host);
  static IPAddress localIP();

  friend class InternetClient;
  friend class InternetServer;
};

extern InternetClass Internet;

class InternetClient : public Print {
private:
  EthernetClient ethClient;
  WiFiClient wiFiClient;

public:
  InternetClient();
  InternetClient(EthernetClient ethClient);
  InternetClient(WiFiClient wiFiClient);
  int connect(IPAddress ip, uint16_t port);
  uint8_t connected();
  int available();
  size_t write(uint8_t);
  size_t write(const uint8_t *buf, size_t size);
  int read();
  void stop();
  IPAddress remoteIP();
  operator bool();

  using Print::write;

  friend class EthernetServer;
};

class InternetServer {
private:
  uint16_t port;
  EthernetServer ethServer;
  WiFiServer wiFiServer;

public:
  InternetServer(uint16_t port)
      : port(port), ethServer(port), wiFiServer(port) {}
  InternetClient available();
  void begin();
};

#endif // internet_h_