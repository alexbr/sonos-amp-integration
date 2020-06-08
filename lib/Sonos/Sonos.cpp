/************************************************************************
 * Sonos v0.1
 ************************************************************************/

#include "Sonos.h"

const char httpVersionP[] PROGMEM = HTTP_VERSION;
const char headerHostP[] PROGMEM = HEADER_HOST;
const char headerContentTypeP[] PROGMEM = HEADER_CONTENT_TYPE;
const char headerContentLengthP[] PROGMEM = HEADER_CONTENT_LENGTH;
const char headerSoapActionP[] PROGMEM = HEADER_SOAP_ACTION;
const char headerConnectionP[] PROGMEM = HEADER_CONNECTION;

const char soapEnvelopeStartP[] PROGMEM = SOAP_ENVELOPE_START;
const char soapEnvelopeEndP[] PROGMEM = SOAP_ENVELOPE_END;
const char soapBodyStartP[] PROGMEM = SOAP_BODY_START;
const char soapBodyEndP[] PROGMEM = SOAP_BODY_END;
const char soapEnvelopeP[] PROGMEM = SOAP_TAG_ENVELOPE;
const char soapBodyP[] PROGMEM = SOAP_TAG_BODY;

const char p_UpnpUrnSchema[] PROGMEM = UPNP_URN_SCHEMA;
const char p_UpnpAvTransportService[] PROGMEM = UPNP_AV_TRANSPORT_SERVICE;
const char p_UpnpAvTransportEndpoint[] PROGMEM = UPNP_AV_TRANSPORT_ENDPOINT;
const char p_UpnpRenderingControlService[] PROGMEM = UPNP_RENDERING_CONTROL_SERVICE;
const char p_UpnpRenderingControlEndpoint[] PROGMEM = UPNP_RENDERING_CONTROL_ENDPOINT;
const char p_UpnpDevicePropertiesService[] PROGMEM = UPNP_DEVICE_PROPERTIES_SERVICE;
const char p_UpnpDevicePropertiesEndpoint[] PROGMEM = UPNP_DEVICE_PROPERTIES_ENDPOINT;

const char p_Play[] PROGMEM = SONOS_TAG_PLAY;
const char p_SourceRinconTemplate[] PROGMEM = SONOS_SOURCE_RINCON_TEMPLATE;
const char p_Stop[] PROGMEM = SONOS_TAG_STOP;
const char p_Pause[] PROGMEM = SONOS_TAG_PAUSE;
const char p_Previous[] PROGMEM = SONOS_TAG_PREVIOUS;
const char p_Next[] PROGMEM = SONOS_TAG_NEXT;
const char p_InstenceId0Tag[] PROGMEM = SONOS_INSTANCE_ID_0_TAG;
const char p_TimeFormatTemplate[] PROGMEM = SONOS_TIME_FORMAT_TEMPLATE;

const char p_GetPositionInfoA[] PROGMEM = SONOS_TAG_GET_POSITION_INFO;
const char p_GetPositionInfoR[] PROGMEM = SONOS_TAG_GET_POSITION_INFO_RESPONSE;
const char p_Track[] PROGMEM = SONOS_TAG_TRACK;
const char p_TrackDuration[] PROGMEM = SONOS_TAG_TRACK_DURATION;
const char p_TrackMetadata[] PROGMEM = SONOS_TAG_TRACK_METADATA;
const char p_TrackURI[] PROGMEM = SONOS_TAG_TRACK_URI;
const char p_RelTime[] PROGMEM = SONOS_TAG_REL_TIME;
const char streamContentStartP[] PROGMEM = SONOS_STREAM_CONTENT;
const char titleStartP[] PROGMEM = SONOS_TITLE_START;
const char creatorStartP[] PROGMEM = SONOS_CREATOR_START;
const char rEndP[] PROGMEM = SONOS_R_END;
const char dcEndP[] PROGMEM = SONOS_DC_END;

const char p_GetTransportInfoA[] PROGMEM = SONOS_TAG_GET_TRANSPORT_INFO;
const char p_GetTransportInfoR[] PROGMEM = SONOS_TAG_GET_TRANSPORT_INFO_RESPONSE;
const char p_CurrentTransportState[] PROGMEM = SONOS_TAG_CURRENT_TRANSPORT_STATE;

Sonos::Sonos(EthernetClient client, void (*ethernetErrCallback)(void)) {
  this->ethClient = client;
  this->ethernetErrCallback = ethernetErrCallback;
}

void Sonos::play(IPAddress host) {
  upnpSet(host, UPNP_AV_TRANSPORT, p_Play, SONOS_TAG_SPEED, "1");
}

void Sonos::stop(IPAddress host) {
  upnpSet(host, UPNP_AV_TRANSPORT, p_Stop);
}

void Sonos::pause(IPAddress host) {
  upnpSet(host, UPNP_AV_TRANSPORT, p_Pause);
}

void Sonos::skip(IPAddress host, uint8_t direction) {
  upnpSet(host, UPNP_AV_TRANSPORT, direction == SONOS_DIRECTION_FORWARD ? p_Next : p_Previous);
}

void Sonos::togglePause(IPAddress host) {
  uint8_t state = getState(host);
  if (state == SONOS_STATE_PLAYING) {
    pause(host);
  }
  else if (state == SONOS_STATE_PAUSED) {
    play(host);
  }
}

uint8_t Sonos::getState(IPAddress host) {
  PGM_P path[] = { soapEnvelopeP, soapBodyP, p_GetTransportInfoR, p_CurrentTransportState };
  char result[sizeof(SONOS_STATE_PAUSED_VALUE)] = "";
  upnpGetString(host, UPNP_AV_TRANSPORT, p_GetTransportInfoA, "", "", path, 4, result, sizeof(result));
  return convertState(result);
}

TrackInfo Sonos::getTrackInfo(
      IPAddress host,
      char *uriBuffer,
      size_t uriBufferSize,
      char *title,
      size_t titleSize,
      char *artist,
      size_t artistSize) {
   TrackInfo trackInfo;
   if (upnpPost(host, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "")) {
    xPath.reset();
    char infoBuffer[20] = "";

    // Track duration
    PGM_P dpath[] = { soapEnvelopeP, soapBodyP, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath(dpath, 4, infoBuffer, sizeof(infoBuffer));
    trackInfo.duration = getTimeInSeconds(infoBuffer);

    // Content (preferred)
    ethClient_stringWithin(streamContentStartP, 23, rEndP, 6, title, titleSize);
    if (strlen(title) == 0) {
       // Title
       ethClient_stringWithin(titleStartP, 16, dcEndP, 7, title, titleSize);
       // Artist
       ethClient_stringWithin(creatorStartP, 18, dcEndP, 7, artist, artistSize);
    }
    // Track URI
    PGM_P upath[] = { soapEnvelopeP, soapBodyP, p_GetPositionInfoR, p_TrackURI };
    ethClient_xPath(upath, 4, uriBuffer, uriBufferSize);
    trackInfo.uri = uriBuffer;

    // Track position
    PGM_P ppath[] = { soapEnvelopeP, soapBodyP, p_GetPositionInfoR, p_RelTime };
    ethClient_xPath(ppath, 4, infoBuffer, sizeof(infoBuffer));
    trackInfo.position = getTimeInSeconds(infoBuffer);
  }
  ethClient_stop();
  return trackInfo;
}

uint8_t Sonos::getSourceFromURI(const char *uri) {
  if (!strncmp(SONOS_SOURCE_FILE_SCHEME, uri, sizeof(SONOS_SOURCE_FILE_SCHEME) - 1)) {
    return SONOS_SOURCE_FILE;
  }
  if (!strncmp(SONOS_SOURCE_HTTP_SCHEME, uri, sizeof(SONOS_SOURCE_HTTP_SCHEME) - 1)) {
    return SONOS_SOURCE_HTTP;
  }
  if (!strncmp(SONOS_SOURCE_RADIO_SCHEME, uri, sizeof(SONOS_SOURCE_RADIO_SCHEME) - 1)) {
    return SONOS_SOURCE_RADIO;
  }
  if (!strncmp(SONOS_SOURCE_RADIO_AAC_SCHEME, uri, sizeof(SONOS_SOURCE_RADIO_AAC_SCHEME) - 1)) {
    return SONOS_SOURCE_RADIO;
  }
  if (!strncmp(SONOS_SOURCE_MASTER_SCHEME, uri, sizeof(SONOS_SOURCE_MASTER_SCHEME) - 1)) {
    return SONOS_SOURCE_MASTER;
  }
  if (!strncmp(SONOS_SOURCE_LINEIN_SCHEME, uri, sizeof(SONOS_SOURCE_LINEIN_SCHEME) - 1)) {
    return SONOS_SOURCE_LINEIN;
  }
  return SONOS_SOURCE_UNKNOWN;
}

void Sonos::upnpSet(IPAddress host, uint8_t upnpMessageType, PGM_P action_P) {
  upnpSet(host, upnpMessageType, action_P, "", "");
}

void Sonos::upnpSet(IPAddress host, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value) {
  upnpSet(host, upnpMessageType, action_P, field, value, "", 0, 0, "");
}

void Sonos::upnpSet(IPAddress host, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue) {
  upnpPost(host, upnpMessageType, action_P, field, valueA, valueB, extraStart_P, extraEnd_P, extraValue);
  ethClient_stop();
}

bool Sonos::upnpPost(
      IPAddress host,
      uint8_t upnpMessageType,
      PGM_P action_P,
      const char *field,
      const char *valueA,
      const char *valueB,
      PGM_P extraStart_P,
      PGM_P extraEnd_P,
      const char *extraValue) {

  if (!ethClient.connect(host, UPNP_PORT)) {
     return false;
  }

  // Get UPnP service name
  PGM_P upnpService = getUpnpService(upnpMessageType);

  // Get HTTP content/body length
  uint16_t contentLength =
    sizeof(SOAP_ENVELOPE_START) - 1 +
    sizeof(SOAP_BODY_START) - 1 +
    SOAP_ACTION_TAG_LEN +
    (strlen_P(action_P) * 2) +
    sizeof(UPNP_URN_SCHEMA) - 1 +
    strlen_P(upnpService) +
    sizeof(SONOS_INSTANCE_ID_0_TAG) - 1 +
    sizeof(SOAP_BODY_END) - 1 +
    sizeof(SOAP_ENVELOPE_END) - 1;

  // Get length of field
  uint8_t fieldLength = strlen(field);
  if (fieldLength) {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength * 2) +
      strlen(valueA) +
      strlen(valueB);
  }

  // Get length of extra field data (e.g. meta data fields)
  if (extraStart_P) {
    contentLength +=
      strlen_P(extraStart_P) +
      strlen(extraValue) +
      strlen_P(extraEnd_P);
  }

  char buffer[50];

  // Write HTTP start
  ethClient_write("POST ");
  ethClient_write_P(getUpnpEndpoint(upnpMessageType), buffer, sizeof(buffer));
  ethClient_write_P(httpVersionP, buffer, sizeof(buffer));

  // Write HTTP header
  sprintf_P(buffer, headerHostP, host[0], host[1], host[2], host[3], UPNP_PORT); // 29 bytes max
  ethClient_write(buffer);
  ethClient_write_P(headerContentTypeP, buffer, sizeof(buffer));
  sprintf_P(buffer, headerContentLengthP, contentLength); // 23 bytes max
  ethClient_write(buffer);
  ethClient_write_P(headerSoapActionP, buffer, sizeof(buffer));
  ethClient_write_P(p_UpnpUrnSchema, buffer, sizeof(buffer));
  ethClient_write_P(upnpService, buffer, sizeof(buffer));
  ethClient_write("#");
  ethClient_write_P(action_P, buffer, sizeof(buffer));
  ethClient_write(HEADER_SOAP_ACTION_END);
  ethClient_write_P(headerConnectionP, buffer, sizeof(buffer));
  ethClient_write("\n");

  // Write HTTP body
  ethClient_write_P(soapEnvelopeStartP, buffer, sizeof(buffer));
  ethClient_write_P(soapBodyStartP, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_START);
  ethClient_write_P(action_P, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_NS);
  ethClient_write_P(p_UpnpUrnSchema, buffer, sizeof(buffer));
  ethClient_write_P(upnpService, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_END);
  ethClient_write_P(p_InstenceId0Tag, buffer, sizeof(buffer));
  if (fieldLength) {
    sprintf(buffer, SOAP_TAG_START, field); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueA);
    ethClient_write(valueB);
    sprintf(buffer, SOAP_TAG_END, field); // 19 bytes
    ethClient_write(buffer);
  }
  if (extraStart_P) {
    ethClient_write_P(extraStart_P, buffer, sizeof(buffer)); // 390 bytes
    ethClient_write(extraValue);
    ethClient_write_P(extraEnd_P, buffer, sizeof(buffer)); // 271 bytes
  }
  ethClient_write(SOAP_ACTION_END_TAG_START);
  ethClient_write_P(action_P, buffer, sizeof(buffer)); // 35 bytes
  ethClient_write(SOAP_ACTION_END_TAG_END);
  ethClient_write_P(soapBodyEndP, buffer, sizeof(buffer)); // 10 bytes
  ethClient_write_P(soapEnvelopeEndP, buffer, sizeof(buffer)); // 14 bytes

  uint32_t start = millis();
  while (!ethClient.available()) {
    if (millis() > (start + UPNP_RESPONSE_TIMEOUT_MS)) {
      if (ethernetErrCallback) ethernetErrCallback();
      return false;
    }
  }
  return true;
}

PGM_P Sonos::getUpnpService(uint8_t upnpMessageType) {
  switch (upnpMessageType) {
    case UPNP_AV_TRANSPORT: return p_UpnpAvTransportService;
    case UPNP_RENDERING_CONTROL: return p_UpnpRenderingControlService;
    case UPNP_DEVICE_PROPERTIES:
    default:
      return p_UpnpDevicePropertiesService;
    }
}

PGM_P Sonos::getUpnpEndpoint(uint8_t upnpMessageType) {
  switch (upnpMessageType) {
    case UPNP_AV_TRANSPORT: return p_UpnpAvTransportEndpoint;
    case UPNP_RENDERING_CONTROL: return p_UpnpRenderingControlEndpoint;
    case UPNP_DEVICE_PROPERTIES:
    default:
      return p_UpnpDevicePropertiesEndpoint;
  }
}

void Sonos::ethClient_write(const char *data) {
  ethClient.print(data);
}

void Sonos::ethClient_write_P(PGM_P data_P, char *buffer, size_t bufferSize) {
  uint16_t dataLen = strlen_P(data_P);
  uint16_t dataPos = 0;
  while (dataLen > dataPos) {
    strlcpy_P(buffer, data_P + dataPos, bufferSize);
    ethClient.print(buffer);
    dataPos += bufferSize - 1;
  }
}

void Sonos::ethClient_stop() {
  if (ethClient) {
    while (ethClient.available()) ethClient.read();
    ethClient.stop();
  }
}

void Sonos::ethClient_stringWithin(const char *beginP, size_t beginSize, const char *endP, size_t endSize, char *resultBuffer, size_t resultBufferSize) {
   uint8_t matchIndex = 0;
   char begin[strlen_P(beginP) + 1];
   char end[strlen_P(endP) + 1];
   strcpy_P(begin, beginP);
   strcpy_P(end, endP);

   while (ethClient.available()) {
      char c = ethClient.read();
      if (c == begin[matchIndex]) {
         matchIndex++;
         if (matchIndex == beginSize) {
            break;
         }
      } else {
         matchIndex = 0;
      }
   }

   if (!ethClient.available()) {
      return;
   }

   uint8_t resultIndex = 0;
   char matchBuffer[10] = "";
   matchIndex = 0;

   const int replacementSize = 6;
   const char replacements[replacementSize][6] = {
       "&amp;",
       "amp;",
       "apos;",
       "lt;",
       "gt;",
       "quot;"};
   const char replaceWiths[replacementSize] = {
       '\0',
       '&',
       '\'',
       '<',
       '>',
       '"'};

   while (ethClient.available() && resultIndex < resultBufferSize) {
      char c = ethClient.read();

      if (c == end[matchIndex]) {
         matchBuffer[matchIndex] = c;
         matchIndex++;
         if (matchIndex == endSize) {
            break;
         }
      } else if (c == '<') {
         // Bail at end of XML
         break;
      } else {
         // No match, so load the data from the matchBuffer into the result
         for (uint8_t i = 0; i < matchIndex; i++) {
            resultBuffer[resultIndex] = matchBuffer[i];
            resultIndex++;
         }

         resultBuffer[resultIndex] = c;
         resultIndex++;
         matchIndex = 0;
      }

      for (uint8_t i = 0; i < replacementSize; i++) {
         const char *replacement = replacements[i];
         int wantMatched = strlen(replacement);

         if (wantMatched <= resultIndex) {
            int matched = 0;
            for (int j = 0; j < wantMatched; j++) {
               if (replacement[wantMatched - j - 1] == resultBuffer[resultIndex - j - 1]) {
                  matched++;
               }
            }

            if (matched == wantMatched) {
               resultIndex = resultIndex - matched;

               char replaceWith = replaceWiths[i];
               if (replaceWith != '\0') {
                  resultBuffer[resultIndex] = replaceWith;
                  resultIndex++;
                  resultBuffer[resultIndex] = '\0';
               }
               break;
            }
         }
      }
   }

   resultBuffer[resultIndex] = '\0';
}

void Sonos::ethClient_xPath(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize) {
  xPath.setPath(path, pathSize);
  while (ethClient.available() && !xPath.getValue(ethClient.read(), resultBuffer, resultBufferSize));
}

void Sonos::upnpGetString(IPAddress host, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value, PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize) {
  if (upnpPost(host, upnpMessageType, action_P, field, value, "", 0, 0, "")) {
    xPath.reset();
    ethClient_xPath(path, pathSize, resultBuffer, resultBufferSize);
  }
  ethClient_stop();
}

uint32_t Sonos::getTimeInSeconds(const char *time) {
  uint8_t len = strlen(time);
  uint32_t seconds = 0;
  uint8_t dPower = 0;
  uint8_t tPower = 0;
  for (int8_t i = len; i > 0; i--) {
    char character = time[i - 1];
    if (character == ':') {
      dPower = 0;
      tPower++;
    }
    else if(character >= '0' && character <= '9') {
      seconds += (character - '0') * uiPow(10, dPower) * uiPow(60, tPower);
      dPower++;
    }
  }
  return seconds;
}

uint32_t Sonos::uiPow(uint16_t base, uint16_t exponent) {
  int result = 1;
  while (exponent) {
    if (exponent & 1) result *= base;
    exponent >>= 1;
    base *= base;
  }
  return result;
}

uint8_t Sonos::convertState(const char *input) {
  if (strcmp(input, SONOS_STATE_PLAYING_VALUE) == 0) return SONOS_STATE_PLAYING;
  if (strcmp(input, SONOS_STATE_PAUSED_VALUE) == 0) return SONOS_STATE_PAUSED;
  return SONOS_STATE_STOPPED;
}
