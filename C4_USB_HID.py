
import hid
import time


class C4_Synth:
    """
    Source Audio C4 Synth pedal
    USB MIDI connection only allows PC or CC
    HID connection allows PC and also to get preset names
    C4 Neuro Desktop presets range from 1 to 128
    C4 internal presets range from 0 to 127
    USB Vendor ID : 0x29a4 | Product ID : 0x0302
    HID C4 Synth communication protocol :
        0x36 followed by memory address coded on 3 bytes dumps memory location
        0x77 followed by internal preset number (0-127) selects corresponding preset
    """

    BASE_PRESET_ADDRESS = 0x80000   # Base address for presets
    PRESET_SIZE = 0x1000            # Each preset fills 4 KB, 128 presets total
    PRESET_NAME_OFFSET = 0xa0       # Preset name is at offset 0xA0 from preset base address

    addr_bytes = []                 # Memory addresses to read to get all presets names
    preset_list = []                # List of active presets (number + name for each)
    c4 = []


    def __init__(self):             # Open C4 HID interface and get active presets
        self.c4 = hid.Device(0x29a4, 0x0302)
        self.c4.nonblocking = 1
        self.generate_addr_bytes()
        self.get_preset_list()


    def hid_close(self):            # Close C4 HID interface
        self.c4.close()


    def generate_addr_bytes(self):  # Generate all memory addresses to read to get presets names
        for i in range(0, 128):
            addr = self.BASE_PRESET_ADDRESS + i * self.PRESET_SIZE
            addr_byte_3 = addr >> 16
            addr_byte_2 = (addr - (addr_byte_3 << 16)) >> 8
            self.addr_bytes.append([addr_byte_3, addr_byte_2, self.PRESET_NAME_OFFSET])
        

    def get_preset_list(self):      # Get active presets numbers + names and store them
        for i in range(0, 128):
            self.c4.write(bytes([0x36, self.addr_bytes[i][0], self.addr_bytes[i][1], self.addr_bytes[i][2]]))
            time.sleep(0.005)       # Let some time for C4 to respond
            while True:
                d = self.c4.read(38)
                if d:
                    if d[1] == 0xff:    # Name filled with 0xFF -> Preset is empty, skip and poll next preset address
                        break
                    preset_name = ""
                    preset_name = preset_name.join([chr(c) for c in d[1:33]])   # Store preset name
                    self.preset_list.append([i, preset_name])                   # Store preset number + name in preset list
                else:
                    break


    def hid_pc(self, preset):       # Program Change using HID interrupt endpoint
        self.c4.write(bytes([0x77, preset]))
        time.sleep(0.005)


# Main loop
def main():

    # Instance and initialise C4 Synth Pedal
    c4 = C4_Synth()

    print(f"*--- {len(c4.preset_list)} Active Presets ---*")
    for i in range(0, len(c4.preset_list)):
        print(f"{c4.preset_list[i][0]: 4} - {c4.preset_list[i][1]}")

    print()

    while True:
        sel = input("Choose Preset (1-128) : ")
        if (sel == ""):
            break
        int_sel = int(sel)
        if int_sel in range (1,129):
            c4.hid_pc(int_sel - 1)

    c4.hid_close()                  # Close C4 HID interface


if __name__ == '__main__':
    main()
