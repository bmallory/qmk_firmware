/* Copyright 2019 cyril cheney
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "muse.h"

#define SOLENOID_DEFAULT_DWELL 12 
#define SOLENOID_MAX_DWELL 100
#define SOLENOID_MIN_DWELL 4
#define SOLENOID_PIN A8

bool solenoid_enabled = false;
bool solenoid_on = false;
bool solenoid_buzz = false;
bool solenoid_buzzing = false;
uint16_t solenoid_start = 0;
uint8_t solenoid_dwell = SOLENOID_DEFAULT_DWELL;

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN
};

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    QMKBEST = SAFE_RANGE,
    QMKURL,
    SOLENOID_TOG,
    SOLENOID_DWELL_MINUS,
    SOLENOID_DWELL_PLUS,
    SOLENOID_BUZZ_ON,
    SOLENOID_BUZZ_OFF
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
  KC_GESC,    KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_MINS,   KC_EQL,            KC_BSPC,KC_HOME,
  KC_TAB,               KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,   KC_Y,   KC_U,   KC_I,   KC_O,   KC_P,      KC_LBRC, KC_RBRC,          KC_END,
  MO(_FN),              KC_A,     KC_S,     KC_D,     KC_F,     KC_G,   KC_H,   KC_J,   KC_K,   KC_L,   KC_SCLN,      KC_QUOT , KC_NUHS ,  KC_ENT, KC_PGUP,
  KC_LSFT,    KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,   KC_N,   KC_M,   KC_COMM,KC_DOT,            KC_SLSH, KC_RSFT,  KC_UP,  KC_PGDN,
  KC_LCTL,    KC_LGUI,            MO(_FN) , KC_LALT,           LT(1, KC_SPC),         KC_SPC,         KC_RALT,MO(_FN),   KC_RCTL, KC_LEFT,  KC_DOWN, KC_RGHT
  ),
    [_FN] = LAYOUT(
  KC_GRV,     KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,  KC_F7,  KC_F8,    KC_F9,  KC_F10, KC_F11,    KC_F12,            KC_NO,  KC_HOME,
  KC_NO,                KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,  KC_NO,  KC_NO,    KC_NO,  KC_NO,  KC_PSCR,   KC_VOLD, KC_VOLU,          KC_END,
  MO(_FN),                KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,  KC_NO,  KC_MPRV,  KC_MPLY,KC_MNXT,KC_MSTP,   KC_NO,   KC_NO,    KC_NO,  KC_NO,
  KC_NO,      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,  KC_NO,  KC_NO,    BL_BRTG,BL_STEP,           KC_NO,   KC_NO,    KC_NO,  CK_TOGG,
  KC_NO,      KC_NO,              MO(_FN),  KC_NO,              KC_TRNS,          SOLENOID_TOG,            KC_NO,  MO(_FN),     KC_NO,   KC_NO,    KC_PGDN,MU_TOG
    )
};


const uint16_t PROGMEM fn_actions[] = {

};

bool muse_mode = false;
uint8_t last_muse_note = 0;
uint16_t muse_counter = 0;
uint8_t muse_offset = 70;
uint16_t muse_tempo = 50;



void solenoid_buzz_on(void) {
  solenoid_buzz = true;
}

void solenoid_buzz_off(void) {
  solenoid_buzz = false;
}

void solenoid_dwell_minus(void) {
  if (solenoid_dwell > 0) solenoid_dwell--;
}

void solenoid_dwell_plus(void) {
  if (solenoid_dwell < SOLENOID_MAX_DWELL) solenoid_dwell++;
}

void solenoid_toggle(void) {
  solenoid_enabled = !solenoid_enabled;
  //muse_mode = !muse_mode;
}

void solenoid_stop(void) {
  writePinLow(SOLENOID_PIN);
  solenoid_on = false;
  solenoid_buzzing = false;
}

void solenoid_fire(void) {
  if (!solenoid_enabled) return;

  if (!solenoid_buzz && solenoid_on) return;
  if (solenoid_buzz && solenoid_buzzing) return;

  solenoid_on = true;
  solenoid_buzzing = true;
  solenoid_start = timer_read();
  writePinHigh(SOLENOID_PIN);
}

void solenoid_check(void) {
  uint16_t elapsed = 0;

  if (!solenoid_on) return;

  elapsed = timer_elapsed(solenoid_start);

  //Check if it's time to finish this solenoid click cycle 
  if (elapsed > solenoid_dwell) {
    solenoid_stop();
    return;
  }

  //Check whether to buzz the solenoid on and off
  if (solenoid_buzz) {
    if (elapsed / SOLENOID_MIN_DWELL % 2 == 0){
      if (!solenoid_buzzing) {
        solenoid_buzzing = true;
        writePinHigh(SOLENOID_PIN);
      }
    }
    else {
      if (solenoid_buzzing) {
        solenoid_buzzing = false;
        writePinLow(SOLENOID_PIN);
      }
    }
  }

}

void solenoid_setup(void) {
 // pinMode(SOLENOID_PIN, PinDirectionOutput);
  setPinOutput(SOLENOID_PIN);
  writePinLow(SOLENOID_PIN);
}

void matrix_init_user(void) {
  solenoid_setup();
}

void matrix_scan_user(void) {
  solenoid_check();
  #ifdef AUDIO_ENABLE
    if (muse_mode) {
        if (muse_counter == 0) {
            uint8_t muse_note = muse_offset + SCALE[muse_clock_pulse()];
            if (muse_note != last_muse_note) {
                stop_note(compute_freq_for_midi_note(last_muse_note));
                play_note(compute_freq_for_midi_note(muse_note), 0xF);
                last_muse_note = muse_note;
            }
        }
        muse_counter = (muse_counter + 1) % muse_tempo;
    } else {
        if (muse_counter) {
            stop_all_notes();
            muse_counter = 0;
        }
    }
  #endif
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    solenoid_fire();
  }

  switch (keycode) {
    case SOLENOID_TOG:
      if (record->event.pressed) {
        solenoid_toggle();
      }
      break;
    case SOLENOID_DWELL_MINUS:
      if (record->event.pressed) {
        solenoid_dwell_minus();
      }
      break;
    case SOLENOID_DWELL_PLUS:
      if (record->event.pressed) {
        solenoid_dwell_plus();
      }
      break;
    case SOLENOID_BUZZ_ON:
      if (record->event.pressed) {
        solenoid_buzz_on();
      }
      break;
    case SOLENOID_BUZZ_OFF:
      if (record->event.pressed) {
        solenoid_buzz_off();
      }
      break;
  }

  return true;
}