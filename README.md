# rp2040_ov2640
Quick demonstration of interfacing with an OV2640 camera from the Raspberry Pi
Pico (RP2040).

Captures images in CIF resolution (352 by 288) and sends them to a computer over
the UART. Images can be captured and the camera's registers read/written from a
small IPython shell.

```
python shell.py [serial port]
```
