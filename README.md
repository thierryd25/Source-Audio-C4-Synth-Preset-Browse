# Source-Audio-C4-Synth-Preset-Browse
Source Audio C4 Synth pedal is an awewome guitar/bass effect but lacks preset browse and selection functionalities
Current possibilities are :
1. Hardware implementation, only allows to access the first 6 presets in memory through a selection switch and a toggle button
2. USB-MIDI host to send MIDI PC to the C4, does the job, but cannot display presets names
3. Computer using Neuro Desktop app, does everything but a computer is required
4. Neuro Mobile app, never used it, but limited possibilites from my understanding

Basic reverse engineering shows that the C4 is obviously a USB-MIDI device, but also features a HID interface. This HID endpoint is used by Neuro Desktop app to comminicate between the pedal and the computer through interrupt requests.


