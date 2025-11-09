#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    300
#define ARTNET_PORT 6454
#define START_UNIVERSE 0

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 50);

EthernetUDP Udp;
CRGB leds[NUM_LEDS];

// Multi-universe frame sync tracking
const uint16_t NUM_UNIVERSES = ((NUM_LEDS * 3) + 509) / 510; // Calculate universes needed (170 LEDs per universe)
unsigned long lastShowTime = 0;
const unsigned long MIN_SHOW_INTERVAL = 5; // Minimum 5ms between shows to prevent overload

void setup() {
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  Udp.begin(ARTNET_PORT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();
  
  Serial.println("\n=== ArtNet LED Controller ===");
  Serial.print("NUM_LEDS: "); Serial.println(NUM_LEDS);
  Serial.print("NUM_UNIVERSES: "); Serial.println(NUM_UNIVERSES);
  Serial.print("IP: "); Serial.println(Ethernet.localIP());
  Serial.print("Listening on port: "); Serial.println(ARTNET_PORT);
}

void loop() {
  uint8_t packet[530];
  int packetSize = Udp.parsePacket();
  
  if (packetSize > 0) {
    Udp.read(packet, packetSize);

    // Validate Art-Net packet: "Art-Net\0" + OpCode 0x5000 (ArtDMX)
    if (packetSize >= 18 && 
        memcmp(packet, "Art-Net", 8) == 0 && 
        packet[8] == 0x00 && 
        packet[9] == 0x50) {
      
      // Parse ArtNet header
      uint8_t sequence = packet[12];
      uint8_t physical = packet[13];
      uint16_t universe = packet[14] | (packet[15] << 8);  // SubUni | (Net << 8)
      uint16_t dmxLength = (packet[16] << 8) | packet[17]; // Length is big-endian
      
      // Clamp DMX length to valid range
      if (dmxLength > 512) dmxLength = 512;
      if (dmxLength > (packetSize - 18)) dmxLength = packetSize - 18;
      
      // Calculate which universe this is relative to our start
      int16_t relativeUniverse = universe - START_UNIVERSE;
      
      // Only process universes we care about
      if (relativeUniverse >= 0 && relativeUniverse < NUM_UNIVERSES && relativeUniverse < 8) {
        
        // Map DMX data to LEDs
        // Each universe can hold 170 RGB LEDs (510 channels)
        uint16_t startLED = relativeUniverse * 170;
        uint16_t numLEDs = min(170, NUM_LEDS - startLED);
        
        for (uint16_t led = 0; led < numLEDs && (led * 3 + 2) < dmxLength; led++) {
          uint16_t ledIndex = startLED + led;
          uint16_t dmxIndex = led * 3;
          
          if (ledIndex < NUM_LEDS) {
            leds[ledIndex].r = packet[18 + dmxIndex];
            leds[ledIndex].g = packet[18 + dmxIndex + 1];
            leds[ledIndex].b = packet[18 + dmxIndex + 2];
          }
        }
        
        // Debug output
        unsigned long now = millis();
        Serial.print("U"); Serial.print(universe);
        Serial.print(" ("); Serial.print(relativeUniverse);
        Serial.print(") len="); Serial.print(dmxLength);
        Serial.print(" LEDs="); Serial.print(startLED); Serial.print("-"); Serial.print(startLED + numLEDs - 1);
        
        // Show after every universe update with minimal throttling
        if (now - lastShowTime >= MIN_SHOW_INTERVAL) {
          FastLED.show();
          Serial.print(" -> SHOW ("); Serial.print(now - lastShowTime); Serial.println("ms)");
          lastShowTime = now;
        } else {
          Serial.println(" (throttled)");
        }
      }
    }
  }
}
