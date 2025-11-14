// Artnet LED Decoder
// by Miles Punch

// All Rights Reserved 2025
// Licensed under the GNU GPL License.

#define DEBUG 1 // 1: DEBUG at 115200, 0: No DEBUG

// Required libraries
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <E131.h>
#include <FastLED.h>

// Defined constants
#define LED_PIN     6
#define NUM_LEDS    300
#define START_UNIVERSE 0 // we may not want to begin on universe 0 (remember that artnet is 0-indexed)

// Static IP and MAC address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 50); // Set your desired static IP here
E131 e131;

/// @brief Array to hold the LED data
CRGB leds[NUM_LEDS];


void init_leds() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void init_networking() {
  e131.begin(mac);
}

void write_leds(int num_channels) {
  // Copy E1.31 data to LED array
  for (int i = 0; i < NUM_LEDS; i++) {
    int channel_index = i * 3;
    if (channel_index + 2 < num_channels) {
      leds[i].r = e131.data[channel_index];
      leds[i].g = e131.data[channel_index + 1];
      leds[i].b = e131.data[channel_index + 2];
    }
  }
  FastLED.show();
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
  // Check for incoming E1.31 packets
  uint16_t num_channels = e131.parsePacket();
  if (num_channels != 0) {
    if (DEBUG) {
      Serial.print(F("Universe "));
      Serial.print(e131.universe);
      Serial.print(F(" / "));
      Serial.print(num_channels);
      Serial.print(F(" Channels | Packet#: "));
      Serial.print(e131.stats.num_packets);
      Serial.print(F(" / Errors: "));
      Serial.print(e131.stats.packet_errors);
      Serial.print(F(" / CH1: "));
      Serial.println(e131.data[0]);
    }
    write_leds(num_channels);
  }
}
