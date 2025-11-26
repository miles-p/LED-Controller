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
#define NUM_LEDS    33
#define ARTNET_PORT 6454
#define START_UNIVERSE 0 // we may not want to begin on universe 0 (remember that artnet is 0-indexed)
#define LEDS_PER_UNIVERSE 170
#define CHANNELS_PER_UNIVERSE (LEDS_PER_UNIVERSE * 3)

// Tracking for smart refresh
const uint8_t NUM_UNIVERSES = (NUM_LEDS + LEDS_PER_UNIVERSE - 1) / LEDS_PER_UNIVERSE;
const uint8_t ALL_UNI_MASK = (1 << NUM_UNIVERSES) - 1;


// Static IP and MAC address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 50); // Set your desired static IP here
ArtnetEtherReceiver artnet;

/// @brief Array to hold the LED data
CRGB leds[NUM_LEDS];
uint8_t universesReceived = 0;
unsigned long lastShowTime = 0;
const unsigned long MIN_SHOW_INTERVAL = 8; // ~125fps max

void artnet_callback(
  const uint8_t *data, uint16_t size,
  const ArtDmxMetadata &metadata,
  const ArtNetRemoteInfo &remote
) {
  uint8_t rel = metadata.universe - START_UNIVERSE;
  if (rel >= NUM_UNIVERSES) return;

  uint16_t start = rel * LEDS_PER_UNIVERSE;
  uint16_t count = (size / 3);
  if (count > LEDS_PER_UNIVERSE) count = LEDS_PER_UNIVERSE;
  if (start + count > NUM_LEDS) count = NUM_LEDS - start;

  memcpy(&leds[start], data, count * 3);

  universesReceived |= (1 << rel);
  unsigned long now = millis();

  #if DEBUG
  Serial.print("Universe: ");
  Serial.print(metadata.universe);
  Serial.print(" | Received: 0x");
  Serial.println(universesReceived, HEX);
  #endif

  if (universesReceived == ALL_UNI_MASK &&
      now - lastShowTime >= MIN_SHOW_INTERVAL) {

    FastLED.show();
    lastShowTime = now;
    universesReceived = 0;
  }
}


void init_leds() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxRefreshRate(0); // Remove artificial frame rate limit
  FastLED.setDither(false);
  FastLED.setCorrection(UncorrectedColor);
  FastLED.clear();
  FastLED.show();
}

void init_networking() {
  Ethernet.begin(mac, ip);
  delay(100);
  artnet.begin(ARTNET_PORT);
  artnet.subscribeArtDmx(artnet_callback);
  
  #if DEBUG
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Universes: ");
  Serial.println(NUM_UNIVERSES);
  #endif
}

void setup() {
  #if DEBUG
  Serial.begin(115200);
  Serial.println("Starting Artnet LED Decoder");
  #endif
  
  init_leds();
  init_networking();
}

void loop() {
  artnet.parse();
}
