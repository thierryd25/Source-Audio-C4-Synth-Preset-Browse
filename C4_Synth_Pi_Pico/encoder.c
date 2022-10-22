/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
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

#include "hardware/gpio.h"

int encoder_pos = 0;
bool encoder_sw = false;

// Get encoder position, GPIO IRQ callback function
// Bourns PEC11R
void gpio_callback(uint gpio, uint32_t events)
{
  switch (gpio)
  {
    case 6:   // Encoder A
      if ((events && GPIO_IRQ_EDGE_RISE) & (gpio_get(7) == 0)) encoder_pos = 1;          // A Rise and B = 0 -> CW
      if ((events && GPIO_IRQ_EDGE_RISE) & (gpio_get(7) == 1)) encoder_pos = -1;         // A Rise and B = 1 -> CCW
      events &= ~GPIO_IRQ_EDGE_RISE;
     break;
    case 8:
        if ((events && GPIO_IRQ_EDGE_FALL)) encoder_sw = true;
        break;
    default:
      return;
  }
}
