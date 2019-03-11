# WiFi PIR Motion Sensor

A Wifi motion detector - Sends a message to websocket clients when motion is detected.

## Hardware

Components (similar devices will likely work too):

* Adafruit Huzzah ESP8266.  
* Adafruit PIR motion sensor.
* FTDI cable or 5V, 500mA power supply.

Connect power using the V+ and GND pins on the FTDI header on the Huzzah.  The
input voltage range of the Huzzah is 4-6V, and the input voltage of the PIR is
5-12V, however the PIR will likely run as low as 4V if needed.  There is no
sleep mode or power-saving features of this implementation - it is designed to
be run from a continuous power supply.

The sensor only triggers a websocket message when motion is first detected, so the
PIR will work best in re-triggering mode and the timeout set to minimum.

The PIR sensor is connected as follows:

|--------|-----|
| Huzzah | PIR |
|--------|-----|
| V+     | +5V |
| GND    | GND |
| 13     | OUT |
|--------|-----|



## Setup

1. Power on the motion sensor.
2. Using another device, find and connect to the "MotionSensor" wifi network.
3. Browse to http://192.168.4.1
4. Select and join a wifi network.
5. Restart the motion sensor (pull the plug, or press the Reset button)
6. Device should now be visible on the network.

You can connect an FTDI cable to and view the console over serial at 9600 baud.

## Listen for Motion

The device runs a websocket server at ws://<ip-address>:80.  When motion is detected,
it broadcasts a "pirMotion" message to all clients.

If the device is powered off or reset, it will lose the list of connected clients.
To ensure clients continue to receive motion events, the client can periodically
send a "Ping" message to the server.  If the client is connected, the server will
respond with "Pong".  If the client is not connected, steps should be taken to
re-connect.

## Other Messages

To reset the wifi connection settings, send a "Reset" message to the websocket,
then restart the device and follow the Setup instructions above.

## LED Indicators

* The RED led will turn on when the device has successfully connected to WiFi.
* The BLUE led will turn on whenever motion is detected.

On power-up, the BLUE led will be on for a few seconds until the PIR stabilizes.
If there is motion during power-up, no event will be triggered.  

## Node Red Client

Here's how to listen for motion events using Node Red.  First, setup the
heartbeat / re-connect:

1. Add an Inject node.  Enable "inject once" and set the repeat interval to 1 minute.
2. Add a websocket-out node.  Set type="connect to" and URL="ws://<ip-address-of-sensor>"
3. Connect the Inject node to the websocket-out node.

Next, setup a listener for motion events:

1. Add a websocket-in node.  Set type="connect to" and URL the same as above.
2. Add a debounce node.  Set the time to 100ms, and connect the input to the websocket-in.
3. Add a switch node.  Create one output for the string, "pirMotion".  Create another
   output for the string, "Pong".

The "Pong" message can be ignored.  If the interval "Ping" message fails to send, the
websocket will try to reconnect automatically.  Attach your own nodes to the "pirMotion"
output to react whenever motion is detected.
