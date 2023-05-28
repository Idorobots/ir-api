#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <WiFiClient.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);
const uint16_t irLed = 3;
const uint16_t recvPin = 2;
#endif

#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);
const uint16_t irLed = 22;
const uint16_t recvPin = 21;
#endif

const uint16_t frequency = 38000;
const uint16_t bufSize = 2048;
const uint8_t timeout = 40;

const char* kSsid = "...";
const char* kPassword = "...";

IRsend irsend(irLed);
IRrecv irrecv(recvPin, bufSize, timeout);

void handleIrSend() {
  Serial.println("Handling /ir/send.");
  Serial.print("Free memory: ");
  Serial.println(system_get_free_heap_size());

  String protocol = "NEC";
  uint16_t freq = frequency;
  String code = "";
  uint8_t repeat = kNoRepeat;

  for (uint8_t i = 0; i < server.args(); i++) {
    String a = server.argName(i);

    if (a == "code") {
      code = server.arg(i);
    } else if (a == "protocol") {
      protocol = server.arg(i);
    } else if (a == "frequency") {
      freq = strtoul(server.arg(i).c_str(), NULL, DEC);
    } else if (a == "repeat") {
      repeat = strtoul(server.arg(i).c_str(), NULL, DEC);
    }
  }

  decode_type_t type = strToDecodeType(protocol.c_str());

  Serial.print("Sent " + protocol + " (");
  Serial.print(type);
  Serial.print("): ");

  if (protocol == "UNKNOWN") {
    // Length is count of nibbles, each 4 bits.
    if (code.length() >= bufSize * 4) {
      Serial.println("Buffer overflow!");
      server.send(400, "text/plain", "Buffer overflow!");
    } else {
      uint16_t length = code.length() / 4;
      uint16_t *raw = new uint16_t[length];

      for(uint16_t i = 0; i < length; i++) {
        raw[i] = (nibbleToValue(code[i * 4 + 0]) << 12)
               + (nibbleToValue(code[i * 4 + 1]) << 8)
               + (nibbleToValue(code[i * 4 + 2]) << 4)
               + (nibbleToValue(code[i * 4 + 3]) << 0);
      }

      irsend.sendRaw(raw, length, freq);

      delete [] raw;

      Serial.println(code);
      server.send(200, "application/json", irJson(protocol, code));
    }
  } else {
    if (code.length() >= bufSize * 2) {
      Serial.println("Buffer overflow!");
      server.send(400, "text/plain", "Buffer overflow!");
    } else if (code.length() > 16) {
      uint16_t length = code.length() / 2;
      uint8_t *state = new uint8_t[length];

      for (uint16_t i = 0; i < length; i++) {
        state[i] = (nibbleToValue(code[i * 2 + 0]) << 4)
                 + (nibbleToValue(code[i * 2 + 1]) << 0);
      }

      bool ok = irsend.send(type, state, length);

      delete [] state;

      Serial.println(code);
      server.send(ok ? 200 : 400, "application/json", irJson(protocol, code));
    } else {
      bool ok = irsend.send(type, strtoul(code.c_str(), NULL, HEX), code.length() * 4, repeat);

      Serial.println(code);
      server.send(ok ? 200 : 400, "application/json", irJson(protocol, code));
    }
  }
}

void handleIrScan() {
  Serial.println("Handling /ir/scan.");
  Serial.print("Free memory: ");
  Serial.println(system_get_free_heap_size());

  decode_results result;

  if (irrecv.decode(&result)) {
    if (!result.overflow) {
      String protocol = typeToString(result.decode_type);
      String code = "";

      if (protocol == "UNKNOWN") {
        uint16_t length = getCorrectedRawLength(&result);
        uint16_t *raw = resultToRawArray(&result);

        // NOTE handleIrSend() expects 4 nibbles for each timing value.
        for (uint16_t i = 0; i < length; ++i) {
          String val = String(raw[i], HEX);
          switch(val.length()) {
            case 1:
              code += "0";
            case 2:
              code += "0";
            case 3:
              code += "0";
            case 4:
            default:
              break;
          }
          code += val;
        }

        delete [] raw;
      } else {
        code = resultToHexidecimal(&result);
      }


      Serial.print("Got " + protocol + " (");
      Serial.print(result.decode_type);
      Serial.print("): ");
      Serial.println(code);

      Serial.print(resultToHumanReadableBasic(&result));
      Serial.println(resultToSourceCode(&result));
      server.send(200, "application/json", irJson(protocol, code));
    } else {
      Serial.println("Buffer overflow!");
      server.send(400, "text/plain", "buffer overflow");
    }
    irrecv.resume();
  } else {
    server.send(404, "text/plain", "not found");
  }

}

String irJson(String protocol, String code) {
  return String("{\"protocol\":\"") + protocol + "\",\"code\":\"" + code + "\"}";
}

uint8_t nibbleToValue(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  return 0; // FIXME Handle unrecognized garbage.
}

void handleNotFound() {
  Serial.println("Handling not_found.");
  Serial.print("Free memory: ");
  Serial.println(system_get_free_heap_size());

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  server.send(404, "text/plain", message);
}

void setup(void) {
  irrecv.enableIRIn();
  irsend.enableIROut(frequency);
  irsend.begin();

#if defined(ESP32)
  Serial.begin(115200);
#else
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#endif
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP().toString());

  server.on("/ir/send", handleIrSend);
  server.on("/ir/scan", handleIrScan);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.print("Free memory: ");
  Serial.println(system_get_free_heap_size());
}

void loop(void) {
  server.handleClient();
}
