#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ArtnetEther.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FastLED.h>
#include <SPI.h>

// Config switches
#define DEBUG     0  // 1: DEBUG at 115200, 0: No DEBUG
#define DHCP      1  // 1: Use DHCP, 0: Use static IP as defined in main
#define TEST_MODE 1  // 1: Just run some LEDs on Red, 0: normal behaviour

// Pin definitions
#define WS2812_DATA_PIN      6
#define NETWORK_STATUS_PIN   4
#define LED_WRITE_STATUS_PIN 3

// LED Configuration
#define NUM_LEDS              300
#define ARTNET_PORT           6454
#define START_UNIVERSE        0  // we may not want to begin on universe 0 (remember that artnet is 0-indexed)
#define LEDS_PER_UNIVERSE     170
#define CHANNELS_PER_UNIVERSE (LEDS_PER_UNIVERSE * 3)

// Calculated constants
extern const uint8_t NUM_UNIVERSES;
extern const uint8_t ALL_UNI_MASK;

// Network configuration
extern byte mac[];
extern IPAddress ip;
extern ArtnetEtherReceiver artnet;

// LED data
extern CRGB leds[];
extern uint8_t universesReceived;
extern unsigned long lastShowTime;
extern const unsigned long MIN_SHOW_INTERVAL;

// Function declarations
void led_status(String led, bool state);
void artnet_callback(const uint8_t* data,
                     uint16_t size,
                     const ArtDmxMetadata& metadata,
                     const ArtNetRemoteInfo& remote);
void led_hello();
void led_oh_shit(int led_pin);
void init_leds();
void init_networking();

#endif  // MAIN_H