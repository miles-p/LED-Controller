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
    Udp.read(packet, 530);

    // verify Art-Net and OpCode = ArtDMX (0x5000)
    if (memcmp(packet, "Art-Net", 7) == 0 && packet[8] == 0x00 && packet[9] == 0x50) {
      // length is in bytes 16 (high) and 17 (low), data starts at byte 18
      uint16_t dmxLen = (packet[16] << 8) | packet[17];
      uint16_t start = 18;

      // clamp to actual received UDP length (len from Udp.parsePacket()) to avoid overruns
      if (dmxLen > (uint16_t)(len - start)) dmxLen = (uint16_t)(len - start);

      for (uint16_t i = 0; i + 2 < dmxLen && i / 3 < NUM_LEDS; i += 3) {
        leds[i / 3].r = packet[start + i];
        leds[i / 3].g = packet[start + i + 1];
        leds[i / 3].b = packet[start + i + 2];
      }
      FastLED.show();
    }
  }
}
