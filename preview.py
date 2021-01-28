import struct
import math
import serial
from PIL import Image

MARKER = b'==FRAME=='

with serial.Serial('COM3', 1000000) as ser:
    marker_buf = [b'\0'] * len(MARKER)
    i = 0

    while True:
        x = ser.read(1)[0]
        marker_buf = marker_buf[1:] + [x]
        
        if marker_buf == list(MARKER):
            raw = ser.read(400*296*2)
            
            img = Image.new('RGB', (352, 288))
            width, height = img.size
            data = img.load()

            for y in range(height):
                for x in range(width):
                    idx = y * width + x
                    v = struct.unpack('<H', raw[2*idx:2*(idx+1)])[0]

                    r, g, b = v >> (5 + 6), (v >> 5) & 0b111111, v & 0b11111 

                    r = math.floor(r / 0x1F * 0xFF)
                    g = math.floor(g / 0x3F * 0xFF)
                    b = math.floor(b / 0x1F * 0xFF)
                    data[x, y] = (r, g, b)

            img.save(f'out{i}.png')

            i += 1
