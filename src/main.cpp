// Artnet LED Decoder
// by Miles Punch

// All Rights Reserved 2025
// Licensed under the GNU GPL License.

#define DEBUG 1 // 1: DEBUG at 115200, 0: No DEBUG

// Required libraries
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ArtnetEther.h>
#include <FastLED.h>

// Defined constants
#define LED_PIN     6
#define NUM_LEDS    300
#define ARTNET_PORT 6454
#define START_UNIVERSE 0 // we may not want to begin on universe 0 (remember that artnet is 0-indexed)

// Static IP and MAC address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 50); // Set your desired static IP here
ArtnetEtherReceiver artnet;

/// @brief Array to hold the LED data
CRGB leds[NUM_LEDS];

void artnet_callback(const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
  if (DEBUG) {
    Serial.print("Received ArtDmx packet: Universe ");
    Serial.print(metadata.universe);
    Serial.print(", Size ");
    Serial.print(size);
    Serial.print(", [");
    Serial.print(data[0]);
    Serial.print(", ");
    Serial.print(data[1]);
    Serial.print(", ");
    Serial.print(data[2]);
    Serial.println("...]");
  }
  // Calculate the starting LED index for this universe
  int universeOffset = (metadata.universe - START_UNIVERSE) * 170;
  
  // Update LEDs for this universe (170 LEDs per universe, 3 bytes per LED)
  int ledsInThisPacket = min(size / 3, 170);
  for (int i = 0; i < ledsInThisPacket; i++) {
    int ledIndex = universeOffset + i;
    if (ledIndex < NUM_LEDS) {
      leds[ledIndex].r = data[i * 3 + 0];
      leds[ledIndex].g = data[i * 3 + 1];
      leds[ledIndex].b = data[i * 3 + 2];
    }
  }
  FastLED.show();
}

void init_leds() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void init_networking() {
  Ethernet.begin(mac, ip);
  delay(100); // Give the Ethernet shield a second to initialize
  artnet.begin(ARTNET_PORT);
  artnet.subscribeArtDmx(artnet_callback);
}

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
    Serial.println("Starting Artnet LED Decoder");
  }
  init_leds();
  init_networking();
}

void loop() {
  artnet.parse();
}
