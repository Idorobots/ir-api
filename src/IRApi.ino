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
const uint16_t irLed = 0;
const uint16_t recvPin = 2;
#endif

#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);
const uint16_t irLed = 22;
const uint16_t recvPin = 21;
#endif

#define MAX_BUF 64

const char* kSsid = "...";
const char* kPassword = "...";

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

void handleIrSend() {
  String arg = "";
  String protocol = "NEC";

  for (uint8_t i = 0; i < server.args(); i++) {
    String a = server.argName(i);

    if (a == "code") {
      arg = server.arg(i);
    } else if (a == "protocol") {
      protocol = server.arg(i);
    }
  }

  decode_type_t type = strToDecodeType(protocol.c_str());

  Serial.print("Sent " + protocol + " (");
  Serial.print(type);
  Serial.print("): ");

  // Length is count of nibbles, each 4 bits.
  if (arg.length() >= MAX_BUF * 2) {
    Serial.println("Buffer overflow!");
    server.send(400, "text/plain", "Buffer overflow!");
  } else if (arg.length() > 16) {
    uint8_t state[MAX_BUF];
    for(uint8_t i = 0; i < arg.length(); i += 2) {
      state[i/2] = nibbleToValue(arg[i]) * 16 + nibbleToValue(arg[i+1]);
    }

    bool ok = irsend.send(type, state, arg.length() / 2);

    Serial.print("[");
    for (uint8_t i = 0; i < arg.length() /2; i++) {
      Serial.print(state[i], 16);
      Serial.print(", ");
    }
    Serial.println("]");
    server.send(ok ? 200 : 400, "text/plain", arg);
  } else {
    uint64_t code = strtoul(arg.c_str(), NULL, 16);

    bool ok = irsend.send(type, code, arg.length() * 4);

    Serial.println(code);
    server.send(ok ? 200 : 400, "text/plain", arg);
  }
}

void handleIrScan() {
  decode_results result;

  if (irrecv.decode(&result)) {
    if (!result.overflow) {
      String protocol = typeToString(result.decode_type);
      String code = resultToHexidecimal(&result);

      Serial.print("Got " + protocol + " (");
      Serial.print(result.decode_type);
      Serial.print("): ");
      Serial.println(code);

      Serial.print(resultToHumanReadableBasic(&result));
      Serial.println(resultToSourceCode(&result));
      server.send(200, "text/plain", code);
    } else {
      Serial.println("Buffer overflow!");
      server.send(400, "text/plain", "buffer overflow");
    }
    irrecv.resume();
  } else {
    server.send(404, "text/plain", "not found");
  }
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
  irsend.begin();

  Serial.begin(115200);
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
}

void loop(void) {
  server.handleClient();
}
