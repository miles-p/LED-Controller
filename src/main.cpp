// Artnet LED Decoder
// by Miles Punch

// All Rights Reserved 2025
// Licensed under the GNU GPL License.

#include "main.h"

// Global variable definitions
const uint8_t NUM_UNIVERSES = (NUM_LEDS + LEDS_PER_UNIVERSE - 1) / LEDS_PER_UNIVERSE;
const uint8_t ALL_UNI_MASK  = (1 << NUM_UNIVERSES) - 1;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 50);
ArtnetEtherReceiver artnet;

CRGB leds[NUM_LEDS];
uint8_t universesReceived             = 0;
unsigned long lastShowTime            = 0;
const unsigned long MIN_SHOW_INTERVAL = 8;  // ~125fps max

void led_status(String led, bool state) {
#if DEBUG
    Serial.print(led);
    Serial.print(" LED is now ");
    Serial.println(state ? "ON" : "OFF");
#endif

    if (led == "network") {
        digitalWrite(NETWORK_STATUS_PIN, state ? HIGH : LOW);
    }
    else if (led == "led_write") {
        digitalWrite(LED_WRITE_STATUS_PIN, state ? HIGH : LOW);
    }
}

void artnet_callback(const uint8_t* data,
                     uint16_t size,
                     const ArtDmxMetadata& metadata,
                     const ArtNetRemoteInfo& remote) {
    uint8_t rel = metadata.universe - START_UNIVERSE;
    if (rel >= NUM_UNIVERSES)
        return;

    uint16_t start = rel * LEDS_PER_UNIVERSE;
    uint16_t count = (size / 3);
    if (count > LEDS_PER_UNIVERSE)
        count = LEDS_PER_UNIVERSE;
    if (start + count > NUM_LEDS)
        count = NUM_LEDS - start;

    memcpy(&leds[start], data, count * 3);

    universesReceived |= (1 << rel);
    unsigned long now = millis();

#if DEBUG
    Serial.print("Universe: ");
    Serial.print(metadata.universe);
    Serial.print(" | Received: 0x");
    Serial.println(universesReceived, HEX);
#endif

    if (universesReceived == ALL_UNI_MASK && now - lastShowTime >= MIN_SHOW_INTERVAL) {
        led_status("led_write", true);
        FastLED.show();
        lastShowTime      = now;
        universesReceived = 0;
        led_status("led_write", false);
    }
}

void led_hello() {
    // do a little dance to say hello
    digitalWrite(LED_WRITE_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, LOW);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, LOW);
    delay(200);

    digitalWrite(NETWORK_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(NETWORK_STATUS_PIN, LOW);
    delay(200);
    digitalWrite(NETWORK_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(NETWORK_STATUS_PIN, LOW);
    delay(200);

    digitalWrite(LED_WRITE_STATUS_PIN, HIGH);
    digitalWrite(NETWORK_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, LOW);
    digitalWrite(NETWORK_STATUS_PIN, LOW);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, HIGH);
    digitalWrite(NETWORK_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, LOW);
    digitalWrite(NETWORK_STATUS_PIN, LOW);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, HIGH);
    digitalWrite(NETWORK_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, LOW);
    digitalWrite(NETWORK_STATUS_PIN, LOW);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, HIGH);
    digitalWrite(NETWORK_STATUS_PIN, HIGH);
    delay(200);
    digitalWrite(LED_WRITE_STATUS_PIN, LOW);
    digitalWrite(NETWORK_STATUS_PIN, LOW);
    delay(200);
}

void led_oh_shit(int led_pin) {
    while (1) {
        digitalWrite(led_pin, HIGH);
        delay(250);
        digitalWrite(led_pin, LOW);
        delay(250);
    }
}

void init_leds() {
    // initialize FastLED
    FastLED.addLeds<WS2812B, WS2812_DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxRefreshRate(0);  // Remove artificial frame rate limit
    FastLED.setDither(false);
    FastLED.setCorrection(UncorrectedColor);
    FastLED.clear();
    FastLED.show();

    // initialize status pins
    pinMode(NETWORK_STATUS_PIN, OUTPUT);
    pinMode(LED_WRITE_STATUS_PIN, OUTPUT);

    led_hello();
}

void init_networking() {
    if (DHCP) {
        if (Ethernet.begin(mac)) {
            // DHCP configured successfully :)
        }
        else {
            // shit shit shit shit shit
            led_oh_shit(NETWORK_STATUS_PIN);
        }
    }
    else {
        Ethernet.begin(mac, ip);
    }

    if (Ethernet.linkStatus() == LinkON) {
        led_status("network", true);
    }
    else {
        led_oh_shit(NETWORK_STATUS_PIN);
    }

#if DEBUG
    Serial.print("Ethernet initialized with IP: ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask: ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP: ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("Link Status: ");
    Serial.println(Ethernet.linkStatus() == LinkON ? "LinkON" : "LinkOFF");
#endif

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
    if (TEST_MODE) {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB::Red;
        }

        FastLED.show();
        while (1)
            ;  // halt!
    }
    else {
        artnet.parse();
    }
}
