// set the gain control of the LMH6401 VGA

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define GAIN_REG 2
#define GAIN_MASK 0x3f

#ifdef PICO_DEFAULT_SPI_CSN_PIN
static inline void cs_select() {
  asm volatile("nop \n nop \n nop");
  gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
  asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
  asm volatile("nop \n nop \n nop");
  gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
  asm volatile("nop \n nop \n nop");
}
#endif

#if defined(spi_default) && defined(PICO_DEFAULT_SPI_CSN_PIN)
static void write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg & 0x7f; // address is 7 bits, LMH6401 requires bit 8 to be low for writes
    buf[1] = data;
    cs_select();
    spi_write_blocking(spi_default, buf, 2);
    cs_deselect();
    sleep_ms(10);
}

static void read_register(uint8_t reg, uint8_t *buf) {
    reg |= 0x80; // address is 7 bits, LMH6401 requires bit 8 to be high for reads
    cs_select();
    spi_write_blocking(spi_default, &reg, 1);
    sleep_ms(10);
    spi_read_blocking(spi_default, 0, buf, 1);
    cs_deselect();
    sleep_ms(10);
}
#endif

int main() {
  // setup
  stdio_init_all();

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi_lmh6401 requires a board with SPI pins
  puts("Default SPI pins were not defined\n");
#else
  spi_init(spi_default, 1000000);
  gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

  // Make the SPI pins available to picotool
  bi_decl(bi_2pins_with_func(PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
  gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
  gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);

  // Make the CS pin available to picotool
  bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

  // wait for connection from USB host
  // it is crucial to wait before attempting to read from the FIFO, otherwise the host will not be able to properly connect
  while (!stdio_usb_connected()) {
    printf(".");
    sleep_ms(500);
  }

  printf("initialized device, please input desired attenuation in dB and press enter\n");
  uint8_t reg;
  read_register(0x2, &reg);

  uint8_t atten = reg & 0x3f;
  printf("Device has default attenuation %d\n", atten);
  uint8_t parsed_atten = 0;
  while (1) {
    // read from stdin
    parsed_atten = 0;
    while (1) {
      int c = getchar_timeout_us(100);
      if (c != PICO_ERROR_TIMEOUT) {
        c &= 0xff;
        if (c == '\r') {
          atten = parsed_atten;
          printf("\n");
          break;
        } else if ((c >= '0') && (c <= '9')) {
          printf("%c", c);
          parsed_atten = (parsed_atten * 10) + (c - '0');
        } else {
          printf("Invalid character; please input attenuation in dB (as a 2-digit decimal) and press ENTER\n");
          break;
        }
      }
    }
    // send new attenuation to LMH6401
    // check attenuation is valid
    if ((atten < 0) || (atten > 32)) {
      printf("Invalid attenuation, valid range is 0-32dB\n");
      continue;
    }
    printf("setting attenuation to %ddB\n", atten);
    read_register(0x2, &reg);
    write_register(0x2, (reg & 0xc0) | atten);
    read_register(0x2, &reg);
    printf("read attenuation back: %ddB\n", reg & 0x3f);
  }
  return 0;
#endif
}
