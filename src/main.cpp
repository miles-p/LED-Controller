#include <Arduino.h>
#include <FastLED.h>

const int LED_PIN = 12;
const int NUM_LEDS = 300;

CRGB leds[NUM_LEDS];

void setup() {
    // put your setup code here, to run once:
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
}

void loop() {
    // put your main code here, to run repeatedly:

}