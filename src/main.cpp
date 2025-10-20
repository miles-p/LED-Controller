// libraries
#include <Arduino.h>
#include <FastLED.h>
#include <ArtnetEther.h>

// instance objects
ArtnetEtherReceiver artnetReceiver;

// constants
const int LED_PIN = 6;
const int NUM_LEDS = 300;

const int START_ADDRESS = 0; // DMX start address
const int UNIVERSE = 0;

const IPAddress IP(192, 168, 1, 50); // Set your desired static IP
const byte MAC[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB}; // Set your desired MAC address

// global variables
CRGB leds[NUM_LEDS];

// callback functions
void callback(const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
    // you can also use pre-defined callbacks

}

void setup() {
    // LED configuration!
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.show();

    Ethernet.begin(MAC, IP);
    delay(500); // Give the Ethernet shield a second to initialize
    artnetReceiver.begin();
    artnetReceiver.subscribeArtDmxUniverse(UNIVERSE, callback);
}

void loop() {
    // put your main code here, to run repeatedly:
    artnetReceiver.parse();
}