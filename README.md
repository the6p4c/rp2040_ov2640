# rp2040_ov2640
Quick demonstration of interfacing with an OV2640 camera from the Raspberry Pi
Pico (RP2040).

Captures images in CIF resolution (352 by 288) and sends them to a computer over
the UART. Images can be captured and the camera's registers read/written from a
small IPython shell.

```
python shell.py [serial port]
```

## Wiring
Wire the following Raspberry Pi Pico pins to an OV2640 board.

|Raspberry Pi Pico|OV2640|
|----|------|
|GND|GND|
|3V3(OUT)|VCC (3.3)|
|GP2|RST|
|GP3|XCLK[^no_xclk]|
|GP4 (I2C0 SDA)|SDA[^i2c_pullups]|
|GP5 (I2C0 SCL)|SCL[^i2c_pullups]|
|GP6|D0|
|GP7|D1|
|GP8|D2|
|GP9|D3|
|GP10|D4|
|GP11|D5|
|GP12|D6|
|GP13|D7|
|GP14|PCLK|
|GP15|HREF|
|GP16|VSYNC|

[^no_xclk]: Missing on some breakout boards like the "red" board. Leave unconnected for internal clock.

[^i2c_pullups]: Likely require I2C pullups unless breakout board provides them. Even so, adding a 10k pullup won't hurt.
