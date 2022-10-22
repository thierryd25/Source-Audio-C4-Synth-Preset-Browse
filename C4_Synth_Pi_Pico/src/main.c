/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "hardware/irq.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
void led_blinking_task(void);

extern void c4_synth_task(void);
extern void lcd_init();
extern void lcd_clear();
extern void lcd_backlight_off();
extern void lcd_string_write(char *message, uint8_t line, uint8_t pos);
extern int encoder_pos;
extern bool encoder_sw;

extern uint8_t cur_preset_index, sel_preset_index, preset_index;
extern int cur_enc;

/*------------- MAIN -------------*/
int main(void)
{
  char message[21];

  board_init();

  printf("Source Audio C4 Synth Program Change\r\n");

  // init host stack on configured roothub port
  tuh_init(BOARD_TUH_RHPORT);

  // Initialize LCD
  lcd_init();

  cur_preset_index = -1;
  sel_preset_index = -1;
  preset_index = 0;

  strcpy(message, "*-- C4 Synth --*    ");
  lcd_string_write(message, 1, 0);
  strcpy(message, "NOK");
  lcd_string_write(message, 1, 17);
  strcpy(message, "Preset  N/A");
  lcd_string_write(message, 2, 0);
  strcpy(message, "N/A Active Presets  ");
  lcd_string_write(message, 4, 0);

  while (1)
  {
    // tinyusb host task
    tuh_task();
    led_blinking_task();
    c4_synth_task();
  }
  return 0;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;

  static bool led_state = false;

  // Blink every interval ms
  if (board_millis() - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
