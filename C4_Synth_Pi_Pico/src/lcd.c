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

#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// LCD 20x4 display functions

// device parameters
#define    LCD_ADDR		0x27         // I2C device address
#define    LCD_WIDTH	20           // Maximum characters per line
#define    LCD_RS_INST	0b0000       // HD44780 RS pin / 0 = instruction
#define    LCD_RS_DATA  0b0001       // HD44780 RS pin / 1 = data
#define    LCD_RW_WRITE 0b0000       // HD44780 RW pin / 0 = write
#define    LCD_RW_READ  0b0010       // HD44780 RW pin / 1 = read
#define    LCD_ENABLE  	0b0100       // HD44780 E pin / 1 = enable
#define    LCD_BACKLIGHT_ON  0b1000  // LCD Backlight

#define    LCD_LINE_1  0x00           // LCD RAM address for the 1st line
#define    LCD_LINE_2  0x40           // LCD RAM address for the 2nd line
#define    LCD_LINE_3  0x14           // LCD RAM address for the 3rd line
#define    LCD_LINE_4  0x54           // LCD RAM address for the 4th line

// Timing constants us
#define    E_PULSE  500
#define    E_DELAY  500


void wait_us(uint32_t delay_us)
{
	uint32_t i, j;

	for(j = 0; j < delay_us; j++)
	{
		for (i = 0; i < 4000; i++);
	}
}

void handletmp(int tmp)
{
	tmp++;
}

void lcd_clear()
{
	uint8_t byte_p[1];
	int tmp;

	byte_p[0] = 0b00000001;
	tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);
	handletmp(tmp);
}

void lcd_backlight_off()
{
	uint8_t byte_p[1];
	int tmp;

	byte_p[0] = 0b00000000;
	tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);
	handletmp(tmp);
}

void lcd_byte_write(uint8_t byte, uint8_t RS)
{
	uint8_t byte_h, byte_l;
	uint8_t byte_p[2];
	int tmp = 0;

	handletmp(tmp);

    byte_h = RS | (byte & 0xF0) | LCD_BACKLIGHT_ON;
    byte_l = RS | ((byte<<4) & 0xF0) | LCD_BACKLIGHT_ON;

    byte_p[0] = byte_h;
    tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);		// 4 high bits
    wait_us(E_DELAY);
    byte_p[0] = byte_h | LCD_ENABLE;
    tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);
    wait_us(E_PULSE);
    byte_p[0] = byte_h & ~LCD_ENABLE;
    tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);
    wait_us(E_DELAY);

    byte_p[0] = byte_l;
    tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);		// 4 low bits
    wait_us(E_DELAY);
    byte_p[0] = byte_l | LCD_ENABLE;
    tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);
    wait_us(E_PULSE);
    byte_p[0] = byte_l & ~LCD_ENABLE;
    tmp = i2c_write_blocking(i2c0, LCD_ADDR, byte_p, 1, false);
    wait_us(E_DELAY);
}

void lcd_string_write(char *message, uint8_t line, uint8_t pos)
{
	uint8_t line_addr, i;

	switch(line)
	{
		case 1:
			line_addr = LCD_LINE_1;
			break;
		case 2:
			line_addr = LCD_LINE_2;
			break;
		case 3:
			line_addr = LCD_LINE_3;
			break;
		case 4:
			line_addr = LCD_LINE_4;
			break;
		default:
			line_addr = LCD_LINE_1;
	}

	lcd_byte_write(line_addr + (pos | 0b10000000), LCD_RS_INST);	// Set line DDRAM address

	for (i = 0; i < strlen(message); i++)
	{
		lcd_byte_write(message[i], LCD_RS_DATA);
	}
}

void lcd_init()
{
	lcd_byte_write(0b00110011, LCD_RS_INST);
	lcd_byte_write(0b00110010, LCD_RS_INST);
	lcd_byte_write(0b00001100, LCD_RS_INST);
	lcd_byte_write(0b00000001, LCD_RS_INST);
	lcd_byte_write(0b00000110, LCD_RS_INST);
}






