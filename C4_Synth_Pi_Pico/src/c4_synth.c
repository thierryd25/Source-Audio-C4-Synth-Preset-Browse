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

#include "bsp/board.h"
#include "tusb.h"

extern int encoder_pos;
extern bool encoder_sw;
extern void lcd_string_write(char *message, uint8_t line, uint8_t pos);
void wait_us(uint32_t delay_us);

#define BASE_PRESET_ADDRESS 0x80000 // Base address for presets
#define PRESET_SIZE 0x1000			// Each preset fills 4 KB, 128 presets total
#define PRESET_NAME_OFFSET 0xa0		// Preset name is at offset 0xA0 from preset base address

uint32_t addr_bytes[128][3]; // Memory addresses to read to get all active presets names
char presets_names[128][33]; // List of active presets names
uint8_t presets_idx[128];	 // List of active presets indexes
uint8_t cur_preset_index = 0, sel_preset_index = 255, preset_index = 0, preset_number = 0, active_presets = 0, c4_address, c4_instance;

// check if device is Source Audio C4 Synth
static inline bool is_c4_synth(uint8_t dev_addr)
{
	uint16_t vid, pid;
	tuh_vid_pid_get(dev_addr, &vid, &pid);

	return ((vid == 0x29a4) && (pid == 0x0302));
}

void wait_ms(uint32_t ms)
{
	uint32_t m1 = board_millis();

	while (board_millis() < m1 + ms);
}

// Generate address list for preset names
void c4_generate_addr_bytes()
{
	uint8_t i, addr_byte_2, addr_byte_3;
	uint32_t addr;

	for (i = 0; i < 128; i++)
	{
		addr = BASE_PRESET_ADDRESS + i * PRESET_SIZE;
		addr_byte_3 = addr >> 16;
		addr_byte_2 = (addr - (addr_byte_3 << 16)) >> 8;
		addr_bytes[i][0] = addr_byte_3;
		addr_bytes[i][1] = addr_byte_2;
		addr_bytes[i][2] = PRESET_NAME_OFFSET;
		// printf("%d %d %d\r\n", addr_byte_3, addr_byte_2, PRESET_NAME_OFFSET);
	}
}

void c4_get_preset(uint8_t dev_addr, uint8_t instance, uint8_t index)
{
	uint8_t command[4];

	command[0] = 0x36;
	command[1] = addr_bytes[index][0];
	command[2] = addr_bytes[index][1];
	command[3] = addr_bytes[index][2];

	wait_ms(25);		// Delay for C4 Synth to get prepaired (erratic behavior if no delay)
	tuh_hid_send_report(dev_addr, instance, 0, command, 4);
	tuh_hid_receive_report(dev_addr, instance);
}

void c4_save_preset(uint8_t index, uint8_t number, uint8_t const *report)
{
	uint8_t i;

	for (i = 1; i < 33; i++)
	{
		presets_names[index][i - 1] = (char)report[i];
	}
	presets_idx[index] = number;

	// printf("Preset saved - %.3d - %.3d - %s\r\n", index, presets_idx[index], presets_names[index]);
}

void c4_program_change(uint8_t dev_addr, uint8_t instance, uint8_t preset)
{
	uint8_t command[2];

	command[0] = 0x77;
	command[1] = preset;

	printf("Program Change %d\r\n", preset);

	tuh_hid_send_report(dev_addr, instance, 0, command, 2);
}

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

void c4_synth_task(void)
{
	uint8_t i;
	char preset_str[5];
	char preset_name[21];
	char act[4];

	if ((c4_address) & (preset_number == 128))
	{
		if (encoder_pos == 1) // CW
		{
			if (preset_index == (active_presets - 1))
				preset_index = 0;
			else
				preset_index += 1;
		}
		if (encoder_pos == -1) // CCW
		{
			if (preset_index == 0)
				preset_index = active_presets - 1;
			else
				preset_index -= 1;
		}
		if (preset_index != cur_preset_index)
		{
			sprintf(preset_str, "%.3d ", presets_idx[preset_index] + 1);
			lcd_string_write(preset_str, 2, 8);	  // Display preset number using Neuro Desktop range (1-128)
			if (preset_index == sel_preset_index) // Show "*" if this preset has previously been selected
			{
				strcpy(act, "ACT");
				lcd_string_write(act, 2, 17);
			}
			else
			{
				strcpy(act, "   ");
				lcd_string_write(act, 2, 17);
			}
			for (i = 0; i < 20; i++) // keep only first 20 characters
				preset_name[i] = presets_names[preset_index][i];
			preset_name[20] = 0;				 // End of string
			lcd_string_write(preset_name, 3, 0); // Display preset name (20 first characters out of 32)
		}
		if (encoder_sw) // Switch pressed -> select preset or exit
		{
			c4_program_change(c4_address, c4_instance, presets_idx[preset_index]); // Apply program change with selected preset
			sel_preset_index = preset_index;									   // Save index of selected preset
			strcpy(act, "ACT");
			lcd_string_write(act, 2, 17); // Show "*" as preset has been selected
			encoder_sw = false;
		}
		cur_preset_index = preset_index; // Save index of displayed preset
		encoder_pos = 0;				 // Save encoder position
	}
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
	(void)desc_report;
	(void)desc_len;
	uint16_t vid, pid;
	char message[4];

	tuh_vid_pid_get(dev_addr, &vid, &pid);

	printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
	printf("VID = %04x, PID = %04x\r\n", vid, pid);

	c4_address = dev_addr;
	c4_instance = instance;

	// Source Audio C4 Synth
	if (is_c4_synth(dev_addr))
	{
		printf("Source Audio C4 Synth connected\r\n");
		strcpy(message, " OK");
		lcd_string_write(message, 1, 17);
		c4_generate_addr_bytes();
		c4_get_preset(dev_addr, instance, 0); // Get first preset
	}
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
	char message[21];

	c4_address = 0;					// Reset variables
	sel_preset_index = 255;
	preset_index = 0;
	cur_preset_index = 0;
	active_presets = 0;
	preset_number = 0;

	strcpy(message, "NOK");
	lcd_string_write(message, 1, 17);
	strcpy(message, "N/A");
	lcd_string_write(message, 2, 8);
	strcpy(message, "   ");
	lcd_string_write(message, 2, 17);
	strcpy(message, "                    ");
	lcd_string_write(message, 3, 0);
	strcpy(message, "N/A");
	lcd_string_write(message, 4, 0);

	printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
	char message[4], preset_name[21];
	(void)len;
	uint8_t i;

	if (is_c4_synth(dev_addr))
	{
		if ((report[0] == 0x36) && (preset_number < 128))
		{
			if (report[1] != 0xff)
			{
				c4_save_preset(active_presets, preset_number, report); 	// Save preset if not empty (name filled with 0xff if empty)
				active_presets += 1;
			}
			preset_number += 1;	
			if (preset_number < 128) 
				c4_get_preset(dev_addr, instance, preset_number); 		// Get next preset
		}
		if (preset_number == 128)					// All presets have been saved
		{
			sprintf(message, "%.3d", presets_idx[0] + 1);
			lcd_string_write(message, 2, 8);	  	// Display first preset number using Neuro Desktop range (1-128)
			for (i = 0; i < 20; i++) 				// keep only first 20 characters
				preset_name[i] = presets_names[0][i];
			preset_name[20] = 0;				 	// End of string
			lcd_string_write(preset_name, 3, 0); 	// Display first preset name (20 first characters out of 32)
			sprintf(message, "%.3d", active_presets);
			lcd_string_write(message, 4, 0); 		// Display number of active presets in C4 memory
			sel_preset_index = 255;					// Reset variables
			preset_index = 0;
			cur_preset_index = 0;

			printf("%d active presets\r\n", active_presets);
			for (i = 0; i < active_presets; i++)
				printf("%.3d - %.3d - %.3d - %s\r\n", i, presets_idx[i], presets_idx[i] + 1, presets_names[i]);
		}
		return;
	}

	// continue to request to receive report
	if (!tuh_hid_receive_report(dev_addr, instance))
	{
		printf("Error: cannot request to receive report\r\n");
	}
}
