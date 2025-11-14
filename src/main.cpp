// Artnet LED Decoder
// by Miles Punch

// All Rights Reserved 2025
// Licensed under the GNU GPL License.

#define DEBUG 0 // 1: DEBUG at 115200, 0: No DEBUG

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
#define LEDS_PER_UNIVERSE 170
#define CHANNELS_PER_UNIVERSE (LEDS_PER_UNIVERSE * 3)

// Static IP and MAC address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 50); // Set your desired static IP here
ArtnetEtherReceiver artnet;

/// @brief Array to hold the LED data
CRGB leds[NUM_LEDS];

// Tracking for smart refresh
const uint8_t NUM_UNIVERSES = (NUM_LEDS + LEDS_PER_UNIVERSE - 1) / LEDS_PER_UNIVERSE;
uint8_t universesReceived = 0;
unsigned long lastShowTime = 0;
const unsigned long MIN_SHOW_INTERVAL = 8; // ~125fps max

void artnet_callback(const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
  // Calculate the starting LED index for this universe
  uint16_t relativeUniverse = metadata.universe - START_UNIVERSE;
  
  // Ignore universes outside our range
  if (relativeUniverse >= NUM_UNIVERSES) return;
  
  uint16_t startLED = relativeUniverse * LEDS_PER_UNIVERSE;
  uint16_t ledsToUpdate = (size < CHANNELS_PER_UNIVERSE) ? size / 3 : LEDS_PER_UNIVERSE;
  uint16_t endLED = startLED + ledsToUpdate;
  
  // Clamp to actual LED count
  if (endLED > NUM_LEDS) endLED = NUM_LEDS;
  
  // Fast direct memory copy - optimized loop
  const uint8_t *src = data;
  CRGB *dest = &leds[startLED];
  
  for (uint16_t i = startLED; i < endLED; i++) {
    dest->r = *src++;
    dest->g = *src++;
    dest->b = *src++;
    dest++;
  }
  
  // Track universe and only show when all received or after interval
  universesReceived |= (1 << relativeUniverse);
  uint8_t allUniversesMask = (1 << NUM_UNIVERSES) - 1;
  unsigned long now = millis();
  
  if ((universesReceived == allUniversesMask) || (now - lastShowTime >= MIN_SHOW_INTERVAL)) {
    FastLED.show();
    lastShowTime = now;
    universesReceived = 0;
    
    #if DEBUG
    Serial.print("U");
    Serial.print(metadata.universe);
    Serial.print(" SHOW (");
    Serial.print(endLED - startLED);
    Serial.println(" LEDs)");
    #endif
  }
}

void init_leds() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxRefreshRate(0); // Remove artificial frame rate limit
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
