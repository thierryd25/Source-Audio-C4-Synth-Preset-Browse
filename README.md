# Source Audio C4 Synth Preset Browse
Source Audio C4 Synth pedal is an awesome guitar/bass effect, but it lacks preset browse and selection functionalities.
Current possibilities are :
1. Hardware implementation, only allows to access the first 6 presets in memory through a selection switch and a toggle button
2. USB-MIDI host to send MIDI PC to the C4, does the job but cannot display presets names
3. Computer using Neuro Desktop app, does everything but a computer running Windows or MacOS is required
4. Neuro Mobile app, never used it, but apparently quite limited possibilities

Basic reverse engineering shows that the C4 is obviously a USB-MIDI device, but also features a HID interface. This HID endpoint is used by Neuro Desktop app to communicate between the pedal and the computer through interrupt requests.
Communication protocols I have discovered and used for this project :
- 0x36 followed by a preset memory address coded on 3 bytes dumps 32 bytes starting from the memory address provided
- 0x77 followed by a preset number (0-127) coded on 1 byte selects the corresponding preset

# First step : Python implementation and test
The basic Python code included in this repository demonstrates the possibility to use HID interrupt requests to display and change presets. It uses Python HID libraries. This code is mainly aimed at testing the communication protocol.

# Current step : prototype using a Raspberry Pi Pico with TinyUSB and PICO-PIO-USB libraries
Thanks to the awesome work done by Hathach (TinyUSB) and Sekigon-Gonnoc (PICO-PIO-USB), I am working on a prototype that does the following :
- Load all C4 active presets numbers/names, skip empty ones
- Display current preset number/name on a I2C LCD screen
- Use a rotary encoder+switch to browse active presets and select the one I want

The prototype is still under development but does the job quite well (still few bugs though...). I will upload the code later as some cleanup is required.

# Disclaimer
This project is not meant to be commercial, I am just an electronics hobbyist (and bass player) who wants to add preset browse/select functionality to the C4 Synth pedal.
Otherwise said, you can freely use the code and schematics provided here, but at your own risk. Do not expect any technical support from my side as I am far from being a highly skilled MCU/Electronics developer... Take a look at my code and you will understand :-)
