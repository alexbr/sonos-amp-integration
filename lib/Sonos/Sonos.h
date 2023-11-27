/************************************************************************
 * Sonos access
 ************************************************************************/

#ifndef sonos_h_
#define sonos_h_

#include <Arduino.h>
#include <MicroXPath_P.h>
// WTF platformio?
#include <WiFiNINA.h>
#if WIFI
#include <WiFiNINA.h>
#else
#include <Internet.h>
#endif

// UPnP config
#define UPNP_PORT 1400
#define UPNP_RESPONSE_TIMEOUT_MS 5000

// HTTP
#define HTTP_VERSION " HTTP/1.1\n"
#define HEADER_HOST "Host: %d.%d.%d.%d:%d\n"
#define HEADER_CONTENT_TYPE "Content-Type: text/xml; charset=\"utf-8\"\n"
#define HEADER_CONTENT_LENGTH "Content-Length: %d\n"
#define HEADER_SOAP_ACTION "SOAPAction: \"urn:"
#define HEADER_SOAP_ACTION_END "\"\n"
#define HEADER_CONNECTION "Connection: close\n"

// SOAP tag data:
#define SOAP_ENVELOPE_START                                                    \
  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "         \
  "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
#define SOAP_ENVELOPE_END "</s:Envelope>"
#define SOAP_BODY_START "<s:Body>"
#define SOAP_BODY_END "</s:Body>"
#define SOAP_TAG_START "<%s>"
#define SOAP_TAG_END "</%s>"
#define SOAP_TAG_LEN 5
#define SOAP_TAG_ENVELOPE "s:Envelope"
#define SOAP_TAG_BODY "s:Body"

// UPnP tag data:
#define SOAP_ACTION_START_TAG_START "<u:"
#define SOAP_ACTION_START_TAG_NS " xmlns:u=\"urn:"
#define SOAP_ACTION_START_TAG_END "\">"
#define SOAP_ACTION_END_TAG_START "</u:"
#define SOAP_ACTION_END_TAG_END ">"
#define SOAP_ACTION_TAG_LEN 24

// UPnP service data:
#define UPNP_URN_SCHEMA "schemas-upnp-org:service:"
#define UPNP_AV_TRANSPORT 1
#define UPNP_AV_TRANSPORT_SERVICE "AVTransport:1"
#define UPNP_AV_TRANSPORT_ENDPOINT "/MediaRenderer/AVTransport/Control"
#define UPNP_RENDERING_CONTROL 2
#define UPNP_RENDERING_CONTROL_SERVICE "RenderingControl:1"
#define UPNP_RENDERING_CONTROL_ENDPOINT                                        \
  "/MediaRenderer/RenderingControl/Control"
#define UPNP_DEVICE_PROPERTIES 3
#define UPNP_DEVICE_PROPERTIES_SERVICE "DeviceProperties:1"
#define UPNP_DEVICE_PROPERTIES_ENDPOINT "/DeviceProperties/Control"

// GetPositionInfo
/*
<u:GetPositionInfoResponse>
  <Track>1</Track>
  <TrackDuration>0:03:21</TrackDuration>
  <TrackMetaData>[Meta data in DIDL-Lite]</TrackMetaData>
  <TrackURI></TrackURI>
  <RelTime>0:01:23</RelTime>
  <AbsTime>NOT_IMPLEMENTED</AbsTime>
  <RelCount>2147483647</RelCount>
  <AbsCount>2147483647</AbsCount>
</u:GetPositionInfoResponse>
*/
#define GETPOSITIONINFO_REQUEST                                                \
  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "         \
  "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/"                \
  "\"><s:Body><u:GetPositionInfo "                                             \
  "xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</"     \
  "InstanceID></u:GetPositionInfo></s:Body></s:Envelope>"

#define SONOS_TAG_GET_POSITION_INFO "GetPositionInfo"
#define SONOS_TAG_GET_POSITION_INFO_RESPONSE "u:GetPositionInfoResponse"
#define SONOS_TAG_TRACK "Track"
#define SONOS_TAG_TRACK_DURATION "TrackDuration"
#define SONOS_TAG_TRACK_URI "TrackURI"
#define SONOS_TAG_TRACK_METADATA "TrackMetaData"
#define SONOS_TAG_REL_TIME "RelTime"
#define SONOS_SOURCE_UNKNOWN 0
#define SONOS_SOURCE_FILE 1
#define SONOS_SOURCE_HTTP 2
#define SONOS_SOURCE_RADIO 3
#define SONOS_SOURCE_LINEIN 4
#define SONOS_SOURCE_MASTER 5
#define SONOS_SOURCE_FILE_SCHEME "x-file-cifs:"
#define SONOS_SOURCE_HTTP_SCHEME "x-sonos-http:"
#define SONOS_SOURCE_RADIO_SCHEME "x-rincon-mp3radio:"
#define SONOS_SOURCE_RADIO_AAC_SCHEME "aac:"
#define SONOS_SOURCE_LINEIN_SCHEME "x-rincon-stream:"
#define SONOS_SOURCE_MASTER_SCHEME "x-rincon:"
#define SONOS_SOURCE_QUEUE_SCHEME "x-rincon-queue:"
#define SONOS_STREAM_CONTENT "&lt;r:streamContent&gt;"
#define SONOS_TITLE_START "&lt;dc:title&gt;"
#define SONOS_CREATOR_START "&lt;dc:creator&gt;"
#define SONOS_R_END "&lt;/r"
#define SONOS_DC_END "&lt;/dc"

// State
/*
<u:GetTransportInfoResponse>
  <CurrentTransportState>[PLAYING/PAUSED_PLAYBACK/STOPPED]</CurrentTransportState>
  <CurrentTransportStatus>[OK/ERROR]</CurrentTransportStatus>
  <CurrentSpeed>1</CurrentSpeed>
</u:GetTransportInfoResponse>
*/
#define GETTRANSPORTINFO_REQUEST                                               \
  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "         \
  "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/"                \
  "\"><s:Body><u:GetTransportInfo "                                            \
  "xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</"     \
  "InstanceID></u:GetTransportInfo></s:Body></s:Envelope>"
#define SONOS_TAG_GET_TRANSPORT_INFO "GetTransportInfo"
#define SONOS_TAG_GET_TRANSPORT_INFO_RESPONSE "u:GetTransportInfoResponse"
#define SONOS_TAG_CURRENT_TRANSPORT_STATE "CurrentTransportState"
#define SONOS_STATE_PLAYING 1
#define SONOS_STATE_PLAYING_VALUE "PLAYING"
#define SONOS_STATE_PAUSED 2
#define SONOS_STATE_PAUSED_VALUE "PAUSED_PLAYBACK"
#define SONOS_STATE_STOPPED 3
#define SONOS_STATE_STOPPED_VALUE "STOPPED"
#define SONOS_STATE_UNKNOWN 4
#define SONOS_STATE_UNKNOWN_VALUE "UNKNOWN"

// Sonos speaker state control:
/*
<u:Play>
  <InstanceID>0</InstanceID>
  <Speed>1</Speed>
</u:Play>
<u:Seek>
  <InstanceID>0</InstanceID>
  <Unit>REL_TIME</Unit>
  <Target>0:01:02</Target>
</u:Seek>
*/
#define SONOS_TAG_PLAY "Play"
#define SONOS_SOURCE_RINCON_TEMPLATE "RINCON_%s0%d%s"
#define SONOS_TAG_SPEED "Speed"
#define SONOS_TAG_STOP "Stop"
#define SONOS_TAG_PAUSE "Pause"
#define SONOS_TAG_PREVIOUS "Previous"
#define SONOS_TAG_NEXT "Next"
#define SONOS_DIRECTION_BACKWARD 0
#define SONOS_DIRECTION_FORWARD 1
#define SONOS_INSTANCE_ID_0_TAG "<InstanceID>0</InstanceID>"

#define SONOS_TIME_FORMAT_TEMPLATE "%d:%02d:%02d"

struct TrackInfo {
  uint32_t duration;
  uint32_t position;
  char *uri;
};

class Sonos {

public:
#if WIFI
  Sonos(WiFiClient client, void (*internetErrCallback)(void));
#else
  Sonos(InternetClient client, void (*internetErrCallback)(void));
#endif

  void play(IPAddress host);
  void stop(IPAddress host);
  void pause(IPAddress host);
  void togglePause(IPAddress host);
  void skip(IPAddress host, uint8_t direction);
  uint8_t getState(IPAddress host);
  uint8_t getSourceFromURI(const char *uri);

  TrackInfo getTrackInfo(IPAddress host, char *uriBuffer, size_t uriBufferSize,
                         char *titleBuffer, size_t titleBufferSize,
                         char *artist, size_t artistSize);

private:
#if WIFI
  WiFiClient client;
#else
  InternetClient client;
#endif

  void (*internetErrCallback)(void);
  void upnpSet(IPAddress host, uint8_t upnpMessageType, PGM_P action_P);
  void upnpSet(IPAddress host, uint8_t upnpMessageType, PGM_P action_P,
               const char *field, const char *value);
  void upnpSet(IPAddress host, uint8_t upnpMessageType, PGM_P action_P,
               const char *field, const char *valueA, const char *valueB,
               PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue);
  bool upnpPost(IPAddress host, uint8_t upnpMessageType, PGM_P action_P,
                const char *field, const char *valueA, const char *valueB,
                PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue);
  const char *getUpnpService(uint8_t upnpMessageType);
  const char *getUpnpEndpoint(uint8_t upnpMessageType);
  void client_write(const char *data);
  void client_write_P(PGM_P data_P, char *buffer, size_t bufferSize);
  void client_stop();

  MicroXPath_P xPath;
  void client_xPath(PGM_P *path, uint8_t pathSize, char *resultBuffer,
                    size_t resultBufferSize);
  void client_stringWithin(const char *begin, size_t beginSize, const char *end,
                           size_t endSize, char *resultBuffer,
                           size_t resultBufferSize);
  void upnpGetString(IPAddress host, uint8_t upnpMessageType, PGM_P action_P,
                     const char *field, const char *value, PGM_P *path,
                     uint8_t pathSize, char *resultBuffer,
                     size_t resultBufferSize);
  uint32_t getTimeInSeconds(const char *time);
  uint32_t uiPow(uint16_t base, uint16_t exp);
  uint8_t convertState(const char *input);
};

#endif