//
// This file is part of the Concrete Ear Project by Friday Bros.
// Copyright 2014-2015 Klaas Freitag <concreteear@volle-kraft-voraus.de>
//
// This is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// It is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Concrete Ear.  If not, see <http://www.gnu.org/licenses/>.
//
//

#include <Wire.h>
#include <LEDFader.h>

#define LEDCNT 3
#define LED_ON_VAL 255
#define LED_OFF_VAL 20

#define STATE_PLAY  1
#define STATE_PAUSE 2
#define STATE_STOP  3
#define STATE_UNKNOWN 0

#define DURATION 800

int _state = 0;
int _current_led;
boolean _state_change = true;

LEDFader _leds[LEDCNT] = {
  LEDFader(3),
  LEDFader(5),
  LEDFader(6)
};

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  pinMode(13, OUTPUT);

  Serial.begin(9600);           // start serial for output

    // Fade from 0 - 255 in 3 seconds
  for( int i = 0; i < LEDCNT; i++ ) {
    _leds[i].set_value(LED_OFF_VAL);
    _leds[i].fade(LED_ON_VAL, DURATION);
  }
  _state = STATE_PLAY;
}

void loop()
{
  visualize_state();
  for( int i = 0; i < LEDCNT; i++ ) {
    LEDFader *led = &_leds[i];
    led->update();

  }
}

void visualize_state()
{
  if( _state_change ) {
    for( int i = 0; i < LEDCNT; i++ ) {
      LEDFader *led = &_leds[i];
      led->set_value(LED_OFF_VAL);
      _current_led = 0;
    }
    _state_change = false;
  }

  if( _state == STATE_PLAY ) {
    LEDFader *led = &_leds[_current_led];
    // LED is on, fade off
    if( !led->is_fading() ) {
      if( led->get_value() == LED_ON_VAL ) {
        led->fade(LED_OFF_VAL, DURATION);
      } else {
        // LED has faded out. Start the next one
        _current_led++;
        if( _current_led > LEDCNT-1 ) _current_led = 0;
        led = &_leds[_current_led];
        led->fade(LED_ON_VAL, DURATION);
      }
    } else {
      // LED is fading in or out, all nice.
    }
  } else if( _state == STATE_PAUSE ) {
     _current_led = 0;
    _leds[0].set_value(LED_OFF_VAL);
    _leds[2].set_value(LED_OFF_VAL);

    LEDFader *led = &_leds[1];
    // only get active if led is not fading
    if( !led->is_fading() ) {
      if( led->get_value() == LED_ON_VAL ) {
        led->fade(LED_OFF_VAL, 2*DURATION);
      } else {
        led->fade(LED_ON_VAL, 2*DURATION);
      }
    } else {
      // LED is fading in or out, all nice.
    }
  } else if( _state == STATE_STOP ) {
     _current_led = 0;
    _leds[0].set_value(0);
    LEDFader *led = &_leds[1];
    if( !led->is_fading() && led->get_value() == LED_OFF_VAL ) {
      led->fade(LED_ON_VAL, DURATION);
    }
    _leds[2].set_value(0);
  } else if( _state == STATE_UNKNOWN ) {
    for( int i = 0; i < LEDCNT; i++ ) {
      LEDFader *led = &_leds[i];
      // LED is on, fade off
      if( !led->is_fading() ) {
        if( led->get_value() == LED_ON_VAL ) {
          led->fade(LED_OFF_VAL, DURATION);
        } else {
          led->fade(LED_ON_VAL, DURATION);
        }
      } else {
        // LED is fading in or out, all nice.
      }
    }
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  String state;
  while(Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    state += c;
    Serial.print(c);         // print the character
  }
  Serial.println(state);         // print the integer

  if( state == "play" ) {
    if( _state != STATE_PLAY ) {
      _state = STATE_PLAY;
      _state_change = true;
    }
  } else if( state == "pause" ) {
    if( _state != STATE_PAUSE ) {
      _state = STATE_PAUSE;
      _state_change = true;
    }
  } else if( state == "stop" ) {
    if( _state != STATE_STOP ) {
      _state = STATE_STOP;
      _state_change = true;
    }
  } else {
    if( _state != STATE_UNKNOWN ) {
      _state = STATE_UNKNOWN;
      _state_change = true;
    }
  }
}

