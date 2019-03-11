#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#define PIR_PIN 13
#define BLUE_LED 2
#define RED_LED 0

WiFiManager wifiManager;
WebSocketsServer webSocket = WebSocketsServer(80);
volatile bool pirChanged = false;
byte pirState;
const uint8_t ping[] = "Ping";
const uint8_t reset[] = "Reset";

/**
 * Interrupt handler for PIR events
 */
void motionEvent () {
        // the main work is done in the loop to keep the ISR short.
        pirChanged = true;
}

/**
 * Listen for "Ping"" from the client, and reply with "Pong".  If the device is
 * reset the list of connected clients is lost.  Clients can periodically attempt
 * to "Ping", and attempt to reconnect if "Pong" is not received.
 *
 * Also listen for "Reset", which will clear the SSID and pwd and run the
 * WiFiManager on reboot.
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        switch(type) {
        case WStype_TEXT:
                if (memcmp(payload, ping, length) == 0) {
                        webSocket.sendTXT(num, "Pong");
                } else if (memcmp(payload, reset, length) == 0) {
                        digitalWrite(RED_LED, HIGH);
                        wifiManager.resetSettings();
                        ESP.reset();
                }
                break;
        default:
                break;
        }
}

void setup() {
        // LEDs
        pinMode(BLUE_LED, OUTPUT);
        pinMode(RED_LED, OUTPUT);
        digitalWrite(RED_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        // PIR
        pinMode(PIR_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionEvent, CHANGE);
        // WiFi
        Serial.begin(9600);
        wifiManager.setDebugOutput(true);
        wifiManager.autoConnect("MotionSensor");  // blocks until configured
        digitalWrite(RED_LED, LOW); // wifi configured, turn on
        // Websockets
        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
}

void loop() {
        webSocket.loop();
        if (pirChanged) {
                pirChanged = false;
                pirState = digitalRead(PIR_PIN); // active high
                digitalWrite(BLUE_LED, pirState ? LOW : HIGH); // active low
                if (pirState) webSocket.broadcastTXT("pirMotion");
        }
}
