#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "image.pio.h"

#include "ov2640_init.h"

const int LED = 25;
const int UART_TX = 0;

const int CAM_SIOC = 5; // I2C0 SCL
const int CAM_SIOD = 4; // I2C0 SDA
const int CAM_RESETB = 2;
const int CAM_XCLK = 3;
const int CAM_VSYNC = 16;

uint8_t image_buf[352*288*2];

const uint8_t OV2640_ADDR = 0x60 >> 1;

void ov2640_reg_write(uint8_t reg, uint8_t value) {
	uint8_t data[] = {reg, value};
	i2c_write_blocking(i2c0, OV2640_ADDR, data, sizeof(data), false);
}

uint8_t ov2640_reg_read(uint8_t reg) {
	i2c_write_blocking(i2c0, OV2640_ADDR, &reg, 1, false);

	uint8_t value;
	i2c_read_blocking(i2c0, OV2640_ADDR, &value, 1, false);

	return value;
}

void ov2640_regs_write(const uint8_t (*init_table)[2]) {
	while (1) {
		uint8_t reg = (*init_table)[0];
		uint8_t value = (*init_table)[1];

		if (reg == 0x00 && value == 0x00) {
			break;
		}

		ov2640_reg_write(reg, value);

		init_table++;
	}
}

int main() {
	stdio_uart_init();
	uart_set_baudrate(uart0, 1000000);

	printf("\n\nBooted!\n");

	// XCLK generation (~20.83 MHz)
	gpio_set_function(CAM_XCLK, GPIO_FUNC_PWM);
	uint slice_num = pwm_gpio_to_slice_num(CAM_XCLK);
	// 6 cycles (0 to 5), 125 MHz / 6 = ~20.83 MHz wrap rate
	pwm_set_wrap(slice_num, 5);
	pwm_set_gpio_level(CAM_XCLK, 3);
	pwm_set_enabled(slice_num, true);

	// Initialise I2C @ 100 kHz
	gpio_set_function(CAM_SIOC, GPIO_FUNC_I2C);
	gpio_set_function(CAM_SIOD, GPIO_FUNC_I2C);
	i2c_init(i2c0, 100 * 1000);

	// Initialise reset pin
	gpio_init(CAM_RESETB);
	gpio_set_dir(CAM_RESETB, GPIO_OUT);

	// Reset camera, and give it some time to wake back up
	gpio_put(CAM_RESETB, 0);
	sleep_ms(100);
	gpio_put(CAM_RESETB, 1);
	sleep_ms(100);

	printf("Doing init...\n");
	//ov2640_regs_write(ov2640_init);
	//printf("Doing VGA init...\n");
	ov2640_regs_write(ov2640_vga);
	//ov2640_regs_write(OV_REGS_INITIALIZE);
	ov2640_regs_write(OV2640_UXGA_CIF);
	//ov2640_regs_write(ov2640_settings_cif);
	printf("Init complete!\n");

	// Set PCLK to be qualified by HREF
	//ov2640_reg_write(0xff, 0x01);
	//uint8_t reg15 = ov2640_reg_read(0x15);
	//printf("reg15 = 0x%02x\n", reg15);
	//ov2640_reg_write(0x15, reg15 | (1 << 5));

	// Set RGB565 output mode
	ov2640_reg_write(0xff, 0x00);
	ov2640_reg_write(0xDA, (ov2640_reg_read(0xDA) & 0xC) | 0x8);

	// Color bar test pattern
	//ov2640_reg_write(0xff, 0x01);
	//ov2640_reg_write(0x12, ov2640_reg_read(0x12) | (1 << 1));

	ov2640_reg_write(0xff, 0x01);
	uint8_t midh = ov2640_reg_read(0x1C);
	uint8_t midl = ov2640_reg_read(0x1D);
	printf("MIDH = 0x%02x, MIDL = 0x%02x\n", midh, midl);

	// Init image reception PIO
	PIO pio = pio0;
	uint sm = 0;
	uint offset = pio_add_program(pio, &image_program);
	image_program_init(pio, sm, offset, 6);

	sleep_ms(100);

	dma_channel_config c = dma_channel_get_default_config(0);
	channel_config_set_read_increment(&c, false);
	channel_config_set_write_increment(&c, true);
	channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	
	size_t image_buf_size = sizeof(image_buf) / sizeof(image_buf[0]);
	dma_channel_configure(0, &c, image_buf, &pio->rxf[sm], image_buf_size, false);

	gpio_init(CAM_VSYNC);
	gpio_set_dir(CAM_VSYNC, GPIO_IN);

	gpio_init(LED);
	gpio_set_dir(LED, GPIO_OUT);
	gpio_put(LED, false);

	while (true) {
		// Wait for vsync rising edge to start frame
		while (gpio_get(CAM_VSYNC) == true);
		while (gpio_get(CAM_VSYNC) == false);

		gpio_put(LED, !gpio_get(LED));

		dma_channel_start(0);
		dma_channel_wait_for_finish_blocking(0);
		dma_channel_configure(0, &c, image_buf, &pio->rxf[sm], image_buf_size, false);

		printf("==FRAME==");
		for (int i = 0; i < image_buf_size; ++i) {
			uart_putc_raw(uart0, image_buf[i]);
		}
	}

	return 0;
}
