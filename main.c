#include <stdio.h>
#include "pico/stdlib.h"
#include "ov2640.h"

const int PIN_LED = 25;

const int PIN_CAM_SIOC = 5; // I2C0 SCL
const int PIN_CAM_SIOD = 4; // I2C0 SDA
const int PIN_CAM_RESETB = 2;
const int PIN_CAM_XCLK = 3;
const int PIN_CAM_VSYNC = 16;
const int PIN_CAM_Y2_PIO_BASE = 6;

const uint8_t CMD_REG_WRITE = 0xAA;
const uint8_t CMD_REG_READ = 0xBB;
const uint8_t CMD_CAPTURE = 0xCC;

uint8_t image_buf[352*288*2];

int main() {
	stdio_uart_init();
	uart_set_baudrate(uart0, 1000000);

	printf("\n\nBooted!\n");

	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);

	struct ov2640_config config;
	config.sccb = i2c0;
	config.pin_sioc = PIN_CAM_SIOC;
	config.pin_siod = PIN_CAM_SIOD;

	config.pin_resetb = PIN_CAM_RESETB;
	config.pin_xclk = PIN_CAM_XCLK;
	config.pin_vsync = PIN_CAM_VSYNC;
	config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE;

	config.pio = pio0;
	config.pio_sm = 0;

	config.dma_channel = 0;
	config.image_buf = image_buf;
	config.image_buf_size = sizeof(image_buf);

	ov2640_init(&config);

	ov2640_reg_write(&config, 0xff, 0x01);
	uint8_t midh = ov2640_reg_read(&config, 0x1C);
	uint8_t midl = ov2640_reg_read(&config, 0x1D);
	printf("MIDH = 0x%02x, MIDL = 0x%02x\n", midh, midl);

	while (true) {
		uint8_t cmd;
		uart_read_blocking(uart0, &cmd, 1);

		gpio_put(PIN_LED, !gpio_get(PIN_LED));
		
		if (cmd == CMD_REG_WRITE) {
			uint8_t reg;
			uart_read_blocking(uart0, &reg, 1);

			uint8_t value;
			uart_read_blocking(uart0, &value, 1);

			ov2640_reg_write(&config, reg, value);
		} else if (cmd == CMD_REG_READ) {
			uint8_t reg;
			uart_read_blocking(uart0, &reg, 1);

			uint8_t value = ov2640_reg_read(&config, reg);

			uart_write_blocking(uart0, &value, 1);
		} else if (cmd == CMD_CAPTURE) {
			ov2640_capture_frame(&config);
			uart_write_blocking(uart0, config.image_buf, config.image_buf_size);
		}
	}

	return 0;
}
