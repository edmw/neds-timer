# Ned’s Timer

– Whenever Ned wants to do something timeboxed, he uses his timer.

* He uses a rotary switch to choose a timer length:
 * 5, 10, 15, 20, 30, 40, 45 or 60 minutes.
 * His selection will be shown on led ring.
* He presses a switch button to start the timer:
 * His remaining time will be shown on led ring.
* Visual and acustic alarm is triggered after timer has been expired.

![Ned’s Timer](/FILES/timer.jpg?raw=true "Ned’s Timer")

![Ned’s Timer circuit](/FILES/circuit.png?raw=true "Ned’s Timer circuit")

```Arduino
///////////////////////////////////////////////////////////////////////////////////////////////////
// neds-timer - https://github.com/edmw/neds-timer
//
// Functions:
// 1. Reads rotary switch position and displays value on neo pixel ring.
// 2. Operates timer for selected value and shows remaining time on neo pixel ring.
// 3. Displays alarm animation on neo pixel ring and outputs alarm tone on piezo buzzer,
//    when timer has been expired.
// 4. Pause mode with all leds turned off except flora’s neo pixel pulsing blue.
//
// Parts list:
// * FLORA - Wearable electronic platform
// * NeoPixel Ring
// * Rotary switch (8 positions)
// * Switch button (momentarily close)
// * Piezo buzzer
//
// Uses FastLED - http://fastled.io/
//
// Copyright (c) 2017 Michael Baumgärtner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <FastLED.h>

#include "millis.h"

// TIMER

// type for timer length in minutes
typedef unsigned int timer_len;

// preset timer lengths in minutes
#define TIMER_LENGTH_1 5
#define TIMER_LENGTH_2 10
#define TIMER_LENGTH_3 15
#define TIMER_LENGTH_4 20
#define TIMER_LENGTH_5 30
#define TIMER_LENGTH_6 40
#define TIMER_LENGTH_7 45
#define TIMER_LENGTH_8 60

// type for timer value in milliseconds
typedef unsigned long timer_val;

// LEDS

// pin for leds
#define LEDS_PIN 6

// total number of leds
#define LEDS_NUM 24

// leds state
CRGB leds[LEDS_NUM];

// number of leds used for each of the three sectors of the timer (must sum up to LED_NUM)
#define LEDS_TIMER_RUNNING_NUM 15
#define LEDS_TIMER_DRAINING_NUM 6
#define LEDS_TIMER_ENDING_NUM 3

// colors used for each of the three sectors of the timer
CHSV leds_timer_running_color = rgb2hsv_approximate(CRGB::Green);
CHSV leds_timer_draining_color = rgb2hsv_approximate(CRGB::Yellow);
CHSV leds_timer_ending_color = rgb2hsv_approximate(CRGB::Red);

// SWITCH (DURATION)

// type for switch value (8 positions)
typedef unsigned int switch_val;

// pin for switch (need to support analog input)
#define SWITCH_PIN 10

// SWITCH BUTTON (START/STOP)

// type for switch button value (on/off)
typedef bool button_val;

// timeout for button longpress (millis)
#define BUTTON_LONG_TIMEOUT 3000

// pin for switch button (momentarily close)
#define BUTTON_PIN 12

// BUZZER (ALARM)

// pin for buzzer (need to support PWM)
#define BUZZER_PIN 9

// FLORA

// pin for onboard led
#define FLORA_LED_PIN 8

// led state
CRGB flora_led[1];

// PAUSE

// milliseconds without user action to wait until pause
#define PAUSE_TIMEOUT 20 * 1000

///////////////////////////////////////////////////////////////////////////////////////////////////
// SETUP

//#define DEBUG_ON

void setup() {
  delay(1000);

  #ifdef DEBUG_ON
  Serial.begin(115200);
  Serial.println("DEBUG MODE ON");
  #endif

  fastled_init();
  
  // FLORA
  FastLED.addLeds<NEOPIXEL, FLORA_LED_PIN>(flora_led, 1);
  flora_led_off(),

  // LEDS
  FastLED.addLeds<NEOPIXEL, LEDS_PIN>(leds, LEDS_NUM);
  leds_off();

  FastLED.show();

  // SWITCH
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  read_switch();
  
  // BUTTON
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  read_button();
  
  // BUZZER
  pinMode(BUZZER_PIN, OUTPUT);

  #ifdef DEBUG_ON
  Serial.println("SETUP COMPLETE");
  #endif

  // startup animation
  int dot = 0;
  for (; dot < LEDS_NUM; dot++) {
    leds[(dot - 1) % LEDS_NUM] = CRGB::Black;
    leds[(dot + 0) % LEDS_NUM] = CRGB::Blue;
    FastLED.show();
    delay(500 / LEDS_NUM);
  }
  leds_off();

  prepare_loop();
}

int fastled_brightness = 20;

void fastled_init() {
  fastled_brightness = 20;
  FastLED.setBrightness(fastled_brightness);
}

void fastled_toggle_brightness() {
  if (fastled_brightness == 20) {
    fastled_brightness = 255;
  }
  else {
    fastled_brightness = 20;
  }
  FastLED.setBrightness(fastled_brightness);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// LOOP

bool timer = false;
bool alarm = false;
bool pause = false;

timer_val loop_timer_len = TIMER_LENGTH_1 * 60 * 1000;
timer_val loop_timer_cur = 0;
timer_val loop_timer_millis = 0;

switch_val loop_switch_val = 0;

button_val loop_button_val = false;
bool loop_button_act = false;
elapsed_millis last_button_millis;

elapsed_millis last_action_millis;

// Timer counts down and shows the remaining time.
void begin_timer() {
  unsigned long len = map_switch_to_length(loop_switch_val);
  loop_timer_len = len * 60 * 1000;
  loop_timer_cur = 0;
  loop_timer_millis = millis();
  #ifdef DEBUG_ON
  Serial.println("TIMER STARTED");
  #endif
}
void do_timer() {
  loop_timer_cur = millis() - loop_timer_millis;
  #ifdef DEBUG_ON
  loop_timer_cur = loop_timer_cur * 100;
  #endif
  blink_timer(loop_timer_cur);
  show_timer(loop_timer_cur, loop_timer_len);
  FastLED.show();
}
void end_timer() {
  flora_led_off();
  FastLED.show();
  #ifdef DEBUG_ON
  Serial.println("TIMER ENDED");
  #endif
}

// Alarm beeps with buzzer and shows a rainbow on ring leds.
void begin_alarm() {
  alarm = true;
  buzzer_beep_start();
  #ifdef DEBUG_ON
  Serial.println("ALARM STARTED");
  #endif
}
void do_alarm() {
  buzzer_beep();
  leds_rainbow();
  FastLED.show();
}
void end_alarm() {
  alarm = false;
  buzzer_off();
  #ifdef DEBUG_ON
  Serial.println("ALARM ENDED");
  #endif
}

// Pause turns off ring leds and pulsates flora led.
void begin_pause() {
  pause = true;
  leds_off();
  FastLED.show();
  #ifdef DEBUG_ON
  Serial.println("PAUSE STARTED");
  #endif
}
void do_pause() {
  flora_led_pulse();
  FastLED.show();
}
void end_pause() {
  pause = false;
  flora_led_off();
  FastLED.show();
  #ifdef DEBUG_ON
  Serial.println("PAUSE ENDED");
  #endif
}

// Settings
void do_setting() {
  show_timer_length(map_switch_to_length(loop_switch_val));
}

// LOOP

void prepare_loop() {
  loop_switch_val = 0;
  loop_switch_val = read_switch();
  last_action_millis = 0;
}

void loop() {
  long now = millis();

  if (timer) {

    // timer loop

    if (alarm) {
      do_alarm();
    }
    else {
      do_timer();
      if (loop_timer_cur >= loop_timer_len) {
        begin_alarm();
      }
    }
  }
  else {

    // setting loop

    // switch sets timer length
    switch_val sval = read_switch();
    if (sval != loop_switch_val) {
      loop_switch_val = sval;
      if (pause) {
        end_pause();
      }
      last_action_millis = 0;
    }

    if (pause) {
      do_pause();
    }
    else {
      do_setting();
    }
  }

  // switch button changes between timer setting and timer running
  button_val bval = read_button();
  if (bval && !loop_button_val) {
    // button down
    loop_button_val = true;
    loop_button_act = true;
    last_button_millis = 0;
    last_action_millis = 0;
  }
  else if (!bval && loop_button_val) {
    // button up
    loop_button_val = false;
    loop_button_act = false;
    if (last_button_millis > BUTTON_LONG_TIMEOUT) {
      // long press release
    }
    else {
      // normal press release
      if (pause) {
        end_pause();
      }
      else {
        if (alarm) {
          end_alarm();
        }
        timer = !timer;
        if (timer) {
          begin_timer();
        }
        else {
          end_timer();
        }
      }
    }
    last_action_millis = 0;
  }
  else {
    if (loop_button_act && loop_button_val && last_button_millis > BUTTON_LONG_TIMEOUT) {
      // long press
      fastled_toggle_brightness();
      loop_button_act = false;
    }
  }

  // begin pause after set time without user action if not in timer mode
  if (!timer) {
    if (last_action_millis > PAUSE_TIMEOUT) {
      last_action_millis -= PAUSE_TIMEOUT;
      if (!pause) {
        begin_pause();
      }
    }
  }

  FastLED.delay(10);
}

void blink_timer(timer_val cur) {
  if ((cur / 1000) % 2) {
    flora_led[0] = CRGB::Blue;
  }
  else {
    flora_led[0] = CRGB::Black;
  }
}

// Shows the timer with the leds.
// - parameter cur: Current value of the timer in milliseconds.
// - parameter len: Length of timer in milliseconds.
// - note: `cur` must be positive and less than `len`.
//
// This uses all leds to visualize the timer. Leds for elapsed time will be off. Leds for remaining
// time will be on. First led for remaining time will be dimmed for fractions of elapsed time.
//
// All leds are partioned into three sectors. Leds in these sectos will use the colors "running",
// "draining" and "ending" from beginning to end. This can be used to make the leds at the end of
// the timer look more important (just use green, yellow, red for example).
void show_timer(timer_val cur, timer_val len) {
  float factor = float(cur) / float(len);
  int threshold = int(LEDS_NUM * factor);
  int dot = 0;
  for(; dot < LEDS_NUM; dot++) {
    uint8_t hue;
    uint8_t sat;
    if (dot < LEDS_TIMER_RUNNING_NUM) {
      hue = leds_timer_running_color.hue;
      sat = leds_timer_running_color.sat;
    }
    else if (dot < LEDS_TIMER_RUNNING_NUM + LEDS_TIMER_DRAINING_NUM) {
      hue = leds_timer_draining_color.hue;
      sat = leds_timer_draining_color.sat;
    }
    else if (dot < LEDS_TIMER_RUNNING_NUM + LEDS_TIMER_DRAINING_NUM + LEDS_TIMER_ENDING_NUM) {
      hue = leds_timer_ending_color.hue;
      sat = leds_timer_ending_color.sat;
    }
    else {
      hue = 0;
      sat = 0;
    }
    if (dot == threshold) {
      int dim = 255 - (long(256 * LEDS_NUM * factor) & 255);
      leds[dot] = CHSV(hue, sat, dim);
    }
    else if (dot < threshold) {
      leds[dot] = CRGB::Black;
    }
    else {
      leds[dot] = CHSV(hue, sat, 255);;
    }
  }
}

// Shows the timer length with the leds.
// - parameter len: Length of timer in minutes.
// - note: length must be equal or less than 60
//
// This uses all leds to visualize the timer length. The whole ring of leds represents 60 minutes.
// The specified timer length will match a clock's minute hand.
void show_timer_length(timer_len len) {
  int pos = (LEDS_NUM * len) / 60;
  int dot = 0;
  for(; dot < pos; dot++) {
    leds[dot] = CRGB::White;
  }
  for(; dot < LEDS_NUM; dot++) {
    leds[dot] = CRGB::Black;
  }
  FastLED.show();
}

timer_len map_switch_to_length(switch_val val) {
  if(val == 0) {
    return TIMER_LENGTH_1;
  }
  else if(val == 1) {
    return TIMER_LENGTH_2;
  }
  else if(val == 2) {
    return TIMER_LENGTH_3;
  }
  else if(val == 3) {
    return TIMER_LENGTH_4;
  }
  else if(val == 4) {
    return TIMER_LENGTH_5;
  }
  else if(val == 5) {
    return TIMER_LENGTH_6;
  }
  else if(val == 6) {
    return TIMER_LENGTH_7;
  }
  else if(val == 7) {
      return TIMER_LENGTH_8;
  }
  // never to be default
  return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SWITCH

#define SWITCH_NUM 8
#define SWITCH_READ_VARIANCE 16
#define SWITCH_READ_DELAY 100

unsigned int switch_cur_value = 0;

unsigned int switch_last_value = 0;
elapsed_millis switch_last_change;

switch_val read_switch() {
  switch_val val = (analogRead(SWITCH_PIN) - SWITCH_READ_VARIANCE) / (1024 / SWITCH_NUM);
  if (val != switch_last_value) {
      switch_last_change = 0;
  }
  switch_last_value = val;
  if (switch_last_change > SWITCH_READ_DELAY) {
    switch_cur_value = val;
  }
  return switch_cur_value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// BUTTON

#define BUTTON_READ_DELAY 50

unsigned int button_cur_value = 0;

unsigned int button_last_value = 1;
elapsed_millis button_last_change;

button_val read_button() {
  int val = digitalRead(BUTTON_PIN);
  if (val != button_last_value) {
    button_last_change = 0;
  }
  button_last_value = val;
  if (button_last_change > BUTTON_READ_DELAY) {
    button_cur_value = val;
  }
  return (button_cur_value == LOW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void leds_off() {
  int dot = 0;
  for (; dot < LEDS_NUM; dot++) {
    leds[dot] = CRGB::Black;
  }
  FastLED.show();
}

int leds_rainbow_pos = 0;

void leds_rainbow() {
  for(int i = 0; i< LEDS_NUM; i++) {
    int pos = ((i * 256 / LEDS_NUM) + leds_rainbow_pos) & 255;
    if (pos < 85) {
      leds[i].setRGB(pos * 3, 255 - pos * 3, 0);
    }
    else if (pos < 170) {
      pos -= 85;
      leds[i].setRGB(255 - pos * 3, 0, pos * 3);
    }
    else {
      pos -= 170;
      leds[i].setRGB(0, pos * 3, 255 - pos * 3);
    }
  }

  leds_rainbow_pos += 4;
  if (leds_rainbow_pos >= 256 * 5) {
    leds_rainbow_pos = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "pitches.h"

int buzzer_beep_tone = 0;

unsigned long buzzer_beep_millis = 0;

void buzzer_beep_start() {
  buzzer_beep_millis = millis();
}

void buzzer_beep() {
  unsigned long now = buzzer_beep_millis - millis();
  if (now & 0x100 && now & 0x10) {
    if (buzzer_beep_tone == 0) {
      tone(BUZZER_PIN, NOTE_GS7, 10);
    }
  }
  else {
    if (buzzer_beep_tone != 0) {
      noTone(BUZZER_PIN);
      buzzer_beep_tone = 0;
    }
  }
}

void buzzer_off() {
  noTone(BUZZER_PIN);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void flora_led_off() {
  flora_led[0] = CRGB::Black;
}

CHSV flora_led_pulse_color = rgb2hsv_approximate(CRGB::Blue);

void flora_led_pulse() {
  float x = float(millis() & 2047) / 2048 * 2 * 3.141529;
  float y = sin(x);
  int dim = int(y * 120) + 130;
  flora_led[0] = CHSV(flora_led_pulse_color.hue, flora_led_pulse_color.sat, dim);
}
