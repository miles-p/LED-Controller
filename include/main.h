// config switches
#define DEBUG 0 // 1: DEBUG at 115200, 0: No DEBUG
#define DHCP 1 // 1: Use DHCP, 0: Use static IP as defined in main

// Defined constants
#define WS2812_DATA_PIN     6
#define NETWORK_STATUS_PIN 4
#define LED_WRITE_STATUS_PIN 3
#define NUM_LEDS    33
#define ARTNET_PORT 6454
#define START_UNIVERSE 0 // we may not want to begin on universe 0 (remember that artnet is 0-indexed)
#define LEDS_PER_UNIVERSE 170
#define CHANNELS_PER_UNIVERSE (LEDS_PER_UNIVERSE * 3)