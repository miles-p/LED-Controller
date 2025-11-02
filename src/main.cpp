#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    300
#define ARTNET_PORT 6454

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 50);

EthernetUDP Udp;
CRGB leds[NUM_LEDS];

// Add at top of file with other variables
unsigned long lastShowTime = 0;
const unsigned long SHOW_INTERVAL = 16; // ~60fps (adjust as needed)

void setup() {
  Ethernet.begin(mac, ip);
  Udp.begin(ARTNET_PORT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  uint8_t packet[530];
  int len = Udp.parsePacket();
  if (len > 0) {
    Udp.read(packet, len);

    // verify Art-Net and OpCode = ArtDMX (0x5000)
    // ensure we have enough bytes for the Art-Net/ArtDMX header (at least 18 bytes)
    if (len >= 18 && memcmp(packet, "Art-Net", 7) == 0 && packet[8] == 0x00 && packet[9] == 0x50) {
      uint16_t dmxLen = (packet[16] << 8) | packet[17];
      const uint16_t start = 18;
      
      // clamp to actual received UDP length (len from Udp.parsePacket()) to avoid overruns
      if (dmxLen > (uint16_t)(len - start)) dmxLen = (uint16_t)(len - start);
      
      // compute Art-Net universe (SubUni at byte 14 = low, Net at byte 15 = high)
      uint16_t universe = (uint16_t)packet[14] | ((uint16_t)packet[15] << 8);
      
      // map DMX channels across universes so multiple universes can address >170 pixels
      for (uint16_t i = 0; i + 2 < dmxLen; i += 3) {
        uint32_t globalChannel = (uint32_t)universe * 512u + (uint32_t)i; // channel offset across universes
        uint32_t ledIndex = globalChannel / 3u;
        if (ledIndex >= NUM_LEDS) break; // stop if beyond our strip
        leds[ledIndex].r = packet[start + i];
        leds[ledIndex].g = packet[start + i + 1];
        leds[ledIndex].b = packet[start + i + 2];
      }
      
      // Only show() at a controlled rate, not after every packet
      if (millis() - lastShowTime >= SHOW_INTERVAL) {
        FastLED.show();
        lastShowTime = millis();
      }
    }
  }
}
