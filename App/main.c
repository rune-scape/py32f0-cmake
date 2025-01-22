/***
 * Demo: LED Toggle
 * 
 * PA0   ------> LED+
 * GND   ------> LED-
 */
//#include "py32f0xx_bsp_printf.h"
#include "py32f0xx_hal.h"
#include <stdint.h>

void display_write_clock_signal(int value) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, value);
}

void display_write_data_signal(int value) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, value);
}

uint8_t display_data0[21] = {0};
uint8_t display_data1[4] = {0};
uint8_t display_bytes_written_count = 0;
uint8_t display_chunk_count = 0;
uint8_t display_digits[5] = {0};
uint8_t display_data_out[145] = {0};

void display_init_data0() {
  display_data0[0] = 0;
  display_data0[1] = 1;
  display_data0[2] = 8;
  display_data0[3] = 1;
  display_data0[4] = 0;
  display_data0[5] = 0;
  display_data0[6] = 3;
  display_data0[7] = 1;
  display_data0[8] = 1;
  display_data0[9] = 1;
  display_data0[10] = 0;
  display_data0[11] = 0;
  display_data0[12] = 0x24;
  display_data0[13] = 0;
  display_data0[14] = 0;
  display_data0[15] = 0;
  display_data0[16] = 0;
  display_data0[17] = 0;
  display_data0[18] = 0;
  display_data0[19] = 0;
  display_data0[20] = 1;
}

void display_init_data1() {
  display_data1[0] = display_data0[12];
}

void display_init_data2() {
  display_data1[1] =
      (display_data0[5] << 7) +
      (display_data0[2] << 2) +
      (display_data0[4] << 1) +
      (display_data0[3] << 0);
}

void display_init_data3() {
  display_data1[2] =
      (display_data0[11] << 7) +
      (display_data0[10] << 6) +
      (display_data0[6] << 4) +
      (display_data0[7] << 3) +
      (display_data0[8] << 2) +
      (display_data0[9] << 0);
}

void display_init_data4() {
  display_data1[3] =
      (display_data0[13] << 7) +
      (display_data0[14] << 6) +
      (display_data0[15] << 5) +
      (display_data0[16] << 4) +
      (display_data0[17] << 3) +
      (display_data0[18] << 2) +
      (display_data0[19] << 1) +
      (display_data0[20] << 0);
}

void display_write_byte(uint8_t bits) {
  for (int i = 0; i < 8; ++i) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 0);
    int v = bits & 0x80 ? 1 : 0;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, v);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, v);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 1);
    bits <<= 1;
  }
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 0);
}

void display_write_separator(uint8_t data) {
  display_write_byte(0x5a);
  display_write_byte(0xff);
  display_data0[1] = data;
  data += display_data0[0] << 4;
  display_write_byte(data);
  display_write_byte(data + 0x59);
}

void display_write_chunk1() {
  display_write_byte(display_data1[0]);
  display_write_byte(display_data1[1]);
  display_write_byte(display_data1[2]);
  display_write_byte(display_data1[3]);
  display_write_byte(display_data1[0] + display_data1[1] + display_data1[2] + display_data1[3]);
}

void delay4() {
  for (register unsigned char i = 0; i < 4; ++i) {
    __asm__("nop");
  }
}

void delay44() {
  for (register unsigned char i = 0; i < 44; ++i) {
    __asm__("nop");
  }
}

void delay44x(int x) {
  for (int i = 0; i < x; ++i) {
    delay44();
  }
}

void display_write_chunk() {
  if (display_bytes_written_count == 0) {
    display_write_separator(1);
    display_write_chunk1();
    display_write_separator(2);
    display_chunk_count = 0;
  }

  uint16_t begin = display_bytes_written_count;
  uint16_t end = begin + 0x10;
  for (int i = begin; i < end; ++i) {
    display_write_byte(display_data_out[i]);
    display_chunk_count += display_data_out[i];
    display_bytes_written_count++;
  }

  uint32_t total_bytes = (uint32_t)(display_data0[2] + 1) << 4;
  uint32_t bytes_written = display_bytes_written_count;
  if (bytes_written >= total_bytes) {
    display_write_byte(display_chunk_count);
    display_write_separator(4);
    display_write_clock_signal(0);
    delay4();
    display_write_clock_signal(1);
    display_write_clock_signal(0);
    display_bytes_written_count = 0;
  }
}

void display_init_data() {
  display_bytes_written_count = 0;
  display_write_data_signal(1);
  display_write_clock_signal(1);
  display_init_data0();
  display_init_data1();
  display_init_data2();
  display_init_data3();
  display_init_data4();
}

void display_write_init(uint8_t v) {
  uint8_t end = (display_data0[2] + 1) << 4;
  for (int i = 0; i < end; ++i) {
    display_data_out[i] = v;
  }
  display_write_chunk();
  if (display_data0[3] == 0) {
    display_write_separator(4);
  }
}

void display_decompose_digits(uint32_t v) {
  uint32_t remainder = v % 10000;
  display_digits[0] = v / 10000;
  v = remainder;
  remainder = v % 1000;
  display_digits[1] = v / 1000;
  v = remainder;
  remainder = v % 100;
  display_digits[2] = v / 100;
  v = remainder;
  remainder = v % 10;
  display_digits[3] = v / 10;
  display_digits[4] = remainder;
}

void display_set_7seg_bits(int begin, int digit, uint8_t brightness) {
  static const uint8_t sseg_bits[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
  if (begin < (144 - 7)) {
    uint8_t bits = sseg_bits[digit];
    int end = begin + 7;
    for (int i = begin; i < end; ++i) {
      if (bits & 0x01) {
        display_data_out[i] = brightness;
      } else {
        display_data_out[i] = 0;
      }
      bits >>= 1;
    }
  }
}

void display_set_charge_percent(int percent, uint8_t brightness) {
  display_decompose_digits(percent);
  display_data_out[32] = brightness > 0 ? 100 : 0;
  if (percent < 100) {
    display_data_out[7] = 0;
    display_data_out[15] = 0;
    if (percent < 10) {
      display_set_7seg_bits(0, display_digits[3], 0);
    } else {
      display_set_7seg_bits(0, display_digits[3], brightness);
    }
    display_set_7seg_bits(8, display_digits[4], brightness);
  } else {
    display_data_out[7] = brightness;
    display_data_out[15] = brightness;
    display_set_7seg_bits(0, display_digits[3], brightness);
    display_set_7seg_bits(8, display_digits[4], brightness);
  }
}

void display_set_charge_percent_alt(int percent, uint8_t brightness) {
  display_decompose_digits(percent);
  display_data_out[32] = 0;
  if (percent < 100) {
    display_data_out[7] = 0;
    display_data_out[15] = 0;
    display_set_7seg_bits(0, display_digits[3], brightness);
    display_set_7seg_bits(8, display_digits[4], brightness);
  } else {
    display_data_out[7] = brightness;
    display_data_out[15] = brightness;
    display_set_7seg_bits(0, display_digits[3], brightness);
    display_set_7seg_bits(8, display_digits[4], brightness);
  }
}

void display_set_juice_percent(int percent, int brightness) {
  display_decompose_digits(percent);
  display_data_out[49] = brightness > 0 ? 100 : 0;
  if (percent < 100) {
    display_data_out[23] = 0;
    display_data_out[31] = 0;
    if (percent < 10) {
      display_set_7seg_bits(16, display_digits[3], 0);
    } else {
      display_set_7seg_bits(16, display_digits[3], brightness);
    }
    display_set_7seg_bits(24, display_digits[4], brightness);
  } else {
    display_data_out[23] = brightness;
    display_data_out[31] = brightness;
    display_set_7seg_bits(16, display_digits[3], brightness);
    display_set_7seg_bits(24, display_digits[4], brightness);
  }
}

void display_set_juice_percent_alt(int percent, uint8_t brightness) {
  display_decompose_digits(percent);
  display_data_out[49] = 0;
  if (percent < 100) {
    display_data_out[23] = 0;
    display_data_out[31] = 0;
    display_set_7seg_bits(16, display_digits[3], brightness);
    display_set_7seg_bits(24, display_digits[4], brightness);
  } else {
    display_data_out[23] = brightness;
    display_data_out[31] = brightness;
    display_set_7seg_bits(16, display_digits[3], brightness);
    display_set_7seg_bits(24, display_digits[4], brightness);
  }
}

uint8_t display_data_flash0[101] = { 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x11, 0x12, 0x13, 0x14, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x21, 0x22, 0x23, 0x25, 0x26, 0x28, 0x29, 0x2b, 0x2d, 0x2f, 0x31, 0x33, 0x35, 0x37, 0x39, 0x3b, 0x3e, 0x40, 0x43, 0x45, 0x48, 0x4b, 0x4e, 0x51, 0x54, 0x58, 0x5b, 0x5f, 0x63, 0x67, 0x6b, 0x6f, 0x74, 0x78, 0x7d, 0x82, 0x88, 0x8d, 0x93, 0x99, 0x9f, 0xa5, 0xac, 0xb3, 0xba, 0xc1, 0xc9, 0xd1, 0xda, 0xe3, 0xec, 0xf5, 0xff };
void display_trails_3_fn_14b8(const uint8_t * indices, int v) {
  if (v <= 100) {
    display_data_out[indices[0]] = display_data_flash0[v];
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
  } else if (v <= 200) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[v - 100];
    display_data_out[indices[2]] = 0;
  } else if (v <= 300) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[v - 200];
  } else if (v <= 400) {
    display_data_out[indices[0]] = display_data_flash0[400 - v];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
  } else if (v <= 500) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = display_data_flash0[500 - v];
    display_data_out[indices[2]] = display_data_flash0[100];
  } else if (v <= 600) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = display_data_flash0[600 - v];
  } else {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
  }
}

void display_trails_4_fn_14b8(const uint8_t * indices, int v) {
  if (v <= 100) {
    display_data_out[indices[0]] = display_data_flash0[v];
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
  } else if (v <= 200) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[v - 100];
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
  } else if (v <= 300) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[v - 200];
    display_data_out[indices[3]] = 0;
  } else if (v <= 400) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[v - 300];
  } else if (v <= 500) {
    display_data_out[indices[0]] = display_data_flash0[500 - v];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[100];
  } else if (v <= 600) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = display_data_flash0[600 - v];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[100];
  } else if (v <= 700) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = display_data_flash0[700 - v];
    display_data_out[indices[3]] = display_data_flash0[100];
  } else if (v <= 800) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = display_data_flash0[800 - v];
  } else {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
  }
}

void display_trails_6_fn_0e8c(const uint8_t * indices, int v) {
  if (v <= 100) {
    display_data_out[indices[0]] = display_data_flash0[v];
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
    display_data_out[indices[4]] = 0;
    display_data_out[indices[5]] = 0;
  } else if (v <= 200) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[v - 100];
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
    display_data_out[indices[4]] = 0;
    display_data_out[indices[5]] = 0;
  } else if (v <= 300) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[v - 200];
    display_data_out[indices[3]] = 0;
    display_data_out[indices[4]] = 0;
    display_data_out[indices[5]] = 0;
  } else if (v <= 400) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[v - 300];
    display_data_out[indices[4]] = 0;
    display_data_out[indices[5]] = 0;
  } else if (v <= 500) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[100];
    display_data_out[indices[4]] = display_data_flash0[v - 400];
    display_data_out[indices[5]] = 0;
  } else if (v <= 600) {
    display_data_out[indices[0]] = display_data_flash0[100];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[100];
    display_data_out[indices[4]] = display_data_flash0[100];
    display_data_out[indices[5]] = display_data_flash0[v - 500];
  } else if (v <= 700) {
    display_data_out[indices[0]] = display_data_flash0[700 - v];
    display_data_out[indices[1]] = display_data_flash0[100];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[100];
    display_data_out[indices[4]] = display_data_flash0[100];
    display_data_out[indices[5]] = display_data_flash0[100];
  } else if (v <= 800) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = display_data_flash0[800 - v];
    display_data_out[indices[2]] = display_data_flash0[100];
    display_data_out[indices[3]] = display_data_flash0[100];
    display_data_out[indices[4]] = display_data_flash0[100];
    display_data_out[indices[5]] = display_data_flash0[100];
  } else if (v <= 900) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = display_data_flash0[900 - v];
    display_data_out[indices[3]] = display_data_flash0[100];
    display_data_out[indices[4]] = display_data_flash0[100];
    display_data_out[indices[5]] = display_data_flash0[100];
  } else if (v <= 1000) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = display_data_flash0[1000 - v];
    display_data_out[indices[4]] = display_data_flash0[100];
    display_data_out[indices[5]] = display_data_flash0[100];
  } else if (v <= 1100) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
    display_data_out[indices[4]] = display_data_flash0[1100 - v];
    display_data_out[indices[5]] = display_data_flash0[100];
  } else if (v <= 1200) {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
    display_data_out[indices[4]] = 0;
    display_data_out[indices[5]] = display_data_flash0[1200 - v];
  } else {
    display_data_out[indices[0]] = 0;
    display_data_out[indices[1]] = 0;
    display_data_out[indices[2]] = 0;
    display_data_out[indices[3]] = 0;
    display_data_out[indices[4]] = 0;
    display_data_out[indices[5]] = 0;
  }
}

void display_single_set_brightness_fn_149c(uint8_t brightness, const uint8_t * indices, int len) {
  for (int i = 0; i < len; ++i) {
    display_data_out[indices[i]] = brightness;
  }
}

uint16_t display_small_stars_progress = 0;
const uint8_t display_small_shooting_star_indices[66] = { 0x5c, 0x5d, 0x5e, 0x90, 0x68, 0x90, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x50, 0x51, 0x52, 0x46, 0x47, 0x48, 0x90, 0x5b, 0x90, 0x4e, 0x4d, 0x4c, 0x3b, 0x3c, 0x3d, 0x40, 0x41, 0x42, 0x43, 0x90, 0x90, 0x34, 0x33, 0x90, 0x38, 0x39, 0x90, 0x6d, 0x6e, 0x6f, 0x6a, 0x6b, 0x6c, 0x71, 0x72, 0x73, 0x7a, 0x7b, 0x90, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x89, 0x8a, 0x90, 0x8c, 0x8d, 0x8e, 0x86, 0x87, 0x90 };
const uint8_t display_comet_indices[8] = { 0x53, 0x54, 0x55, 0x56, 0x74, 0x75, 0x76, 0x77 };
const uint8_t display_planet_indices[11] = { 0x5f, 0x58, 0x59, 0x4b, 0x4a, 0x49, 0x44, 0x3e, 0x36, 0x79, 0x7e };
void display_small_stars_fn_15b8() {
  display_small_stars_progress++;
  if (display_small_stars_progress > 64000) {
    display_small_stars_progress = 0;
  }

  int p = display_small_stars_progress;
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 0, (p + 0) % 1000);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 3, (p + 100) % 1500);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 6, (p + 500) % 1700);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 9, (p + 300) % 1300);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 12, (p + 200) % 2000);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 15, (p + 400) % 1800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 18, (p + 150) % 1100);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 21, (p + 550) % 1200);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 24, (p + 350) % 1900);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 27, (p + 250) % 1800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 30, (p + 450) % 1600);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 33, (p + 170) % 1550);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 36, (p + 570) % 1750);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 39, (p + 370) % 1350);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 42, (p + 270) % 2050);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 45, (p + 470) % 1850);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 48, (p + 120) % 1450);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 51, (p + 520) % 1650);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 54, (p + 320) % 1250);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 57, (p + 220) % 1950);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 60, (p + 110) % 1050);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 63, (p + 250) % 1950);

  // TODO: what is this flag ...
  const int unknown_flag0 = 0;
  if (unknown_flag0) {
    display_single_set_brightness_fn_149c(0, display_comet_indices, 8);
  } else {
    int brightness = (p / 2) % 1500;
    display_trails_4_fn_14b8(display_comet_indices, brightness);
    display_trails_4_fn_14b8(display_comet_indices + 4, brightness);
  }

  // TODO: what is this flag ...
  const int epic_mode = 1;
  if (epic_mode) {
    display_single_set_brightness_fn_149c(100, display_planet_indices, 11);
  }
}

void display_small_stars_fn_18ac() {
  display_small_stars_progress++;
  if (display_small_stars_progress >= 64000) {
    display_small_stars_progress = 0;
  }

  uint16_t p = display_small_stars_progress / 3;
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 0, (p + 650) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 3, (p + 600) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 6, (p + 550) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 9, (p + 500) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 12, (p + 450) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 15, (p + 400) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 18, (p + 350) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 21, (p + 300) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 24, (p + 250) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 27, (p + 200) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 30, (p + 150) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 33, (p + 100) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 36, (p + 50) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 39, (p + 0) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 42, (p + 800) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 45, (p + 700) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 48, (p + 600) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 51, (p + 500) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 54, (p + 400) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 57, (p + 300) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 60, (p + 200) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 63, (p + 100) % 800);
}

void display_small_stars_fn_1b04() {
  display_small_stars_progress++;
  if (display_small_stars_progress >= 64000) {
    display_small_stars_progress = 0;
  }

  uint16_t p = display_small_stars_progress / 3;
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 6, (p + 550) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 27, (p + 200) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 42, (p + 800) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 51, (p + 500) % 800);
  display_trails_3_fn_14b8(display_small_shooting_star_indices + 60, (p + 200) % 800);
}

uint16_t display_off_mode_charging_progress = 0;
uint16_t display_battery_percent = 0;
const uint8_t display_X_indices[4] = { 0x2d, 0x30, 0x2f, 0x2e };
const uint8_t display_orbiting_comet_indices[12] = { 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c };
void display_off_charge_mode_fn_0d90() {
  // TODO: what is this flag ...
  const int unknown_flag0 = 0;
  if (unknown_flag0) {
    display_off_mode_charging_progress++;
    if (display_off_mode_charging_progress <= 2000) {
      if (display_off_mode_charging_progress == 1) {
        display_set_charge_percent(display_battery_percent, 100);
      }
      display_single_set_brightness_fn_149c(display_data_flash0[display_off_mode_charging_progress / 20], display_X_indices, 4);
    } else if (display_off_mode_charging_progress <= 4000) {
      display_single_set_brightness_fn_149c(display_data_flash0[200 - display_off_mode_charging_progress / 20], display_X_indices, 4);
    } else {
      display_off_mode_charging_progress = 0;
    }
  } else {
    display_off_mode_charging_progress++;
    if (display_off_mode_charging_progress > 2500) {
      display_off_mode_charging_progress = 0;
    } else {
      if (display_off_mode_charging_progress == 1) {
        display_set_charge_percent(display_battery_percent, 100);
        display_single_set_brightness_fn_149c(display_data_flash0[100], display_X_indices, 4);
      }
      display_trails_6_fn_0e8c(display_orbiting_comet_indices, display_off_mode_charging_progress / 2);
      display_trails_6_fn_0e8c(display_orbiting_comet_indices + 6, display_off_mode_charging_progress / 2);
    }
  }
}

void display_center_fn_0d90(int p1, int p2) {
  if (p1 == 2) {
    display_single_set_brightness_fn_149c(p2, display_X_indices, 4);
    display_single_set_brightness_fn_149c(p2, display_orbiting_comet_indices, 12);
  } else {
    if (p2 > 0) {
      p2 = 0xff;
    }
    display_single_set_brightness_fn_149c(p2, display_X_indices, 4);
    display_single_set_brightness_fn_149c(0, display_orbiting_comet_indices, 12);
  }
}

void display_XOn_OrbitingCometsAndPlanetsOff_fn_1d84() {
  display_single_set_brightness_fn_149c(0xff, display_X_indices, 4);
  display_single_set_brightness_fn_149c(0, display_orbiting_comet_indices, 12);
  display_single_set_brightness_fn_149c(0, display_planet_indices, 11);
}

uint8_t epic_mode = 2;
void display_OrbitingCometState_fn_1fc8() {
  if (epic_mode < 2) {
    display_single_set_brightness_fn_149c(0, display_orbiting_comet_indices, 12);
  } else {
    display_single_set_brightness_fn_149c(0xff, display_orbiting_comet_indices, 12);
  }
}

void display_star_trails_off_fn_1fc8() {
  display_single_set_brightness_fn_149c(0, display_small_shooting_star_indices, 66);
  display_single_set_brightness_fn_149c(0, display_comet_indices, 8);
}

uint16_t display_stars_glow_up_progress0 = 0;
uint16_t display_stars_glow_up_progress1 = 0;
uint16_t display_stars_glow_up_flag = 0;
const uint8_t display_stars_indices[20] = { 0x60, 0x67, 0x57, 0x5a, 0x4f, 0x45, 0x3f, 0x3a, 0x37, 0x35, 0x32, 0x69, 0x70, 0x78, 0x7d, 0x7c, 0x85, 0x8f, 0x8b, 0x88 };
void display_stars_glow_up_fn_1f6c() {
  display_stars_glow_up_progress0++;
  if (display_stars_glow_up_progress0 > 10) {
    display_stars_glow_up_progress0 = 0;
    display_star_trails_off_fn_1fc8();
    display_stars_glow_up_progress1++;
    if (display_stars_glow_up_progress1 <= 100) {
      display_single_set_brightness_fn_149c(display_data_flash0[display_stars_glow_up_progress1], display_stars_indices, 20);
    } else {
      display_stars_glow_up_flag = 6;
    }
  }
}

uint16_t display_stars_progress0 = 0;
uint16_t display_stars_progress1 = 0;
uint8_t display_stars_state = 0;
void display_stars_fn_22f4() {
  display_stars_progress0++;
  if (display_stars_progress0 > 65000) {
    display_stars_progress0 = 10000;
  }
  if (display_stars_progress0 % 10 == 0) {
    display_stars_progress1++;
    if (display_stars_progress1 < 51) {
      if (display_stars_state == 0) {
        display_single_set_brightness_fn_149c(display_data_flash0[display_stars_progress1 + 50], display_stars_indices, 20);
      } else if (display_stars_state == 1) {
        display_single_set_brightness_fn_149c(display_data_flash0[100 - display_stars_progress1], display_stars_indices, 20);
      }
    } else {
      display_stars_progress1 = 0;
      display_stars_state++;
      if (display_stars_state > 1) {
        display_stars_state = 0;
      }
    }
  }
  if (display_stars_progress0 > 1000) {
    display_small_stars_fn_18ac();
  }
}

void init_gpio() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_ADC_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 2;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Alternate = 1;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 1);

  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0);
}

void deinit_gpio_for_sleep() {
  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, 0);
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_2, 0);
}

void init_charger_PROG_gpio() {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void init_sleepy_gpio() {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = 0;
  GPIO_InitStruct.Mode = 0;
  GPIO_InitStruct.Pull = 0;
  GPIO_InitStruct.Speed = 0;
  GPIO_InitStruct.Alternate = 0;

  init_charger_PROG_gpio();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1);

  __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);
  __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_3);
  __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_2);

  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_NVIC_EnableIRQ(5);
  HAL_NVIC_SetPriority(5,1,0);
  HAL_NVIC_EnableIRQ(6);
  HAL_NVIC_SetPriority(6,1,0);
  HAL_NVIC_EnableIRQ(7);
  HAL_NVIC_SetPriority(7,1,0);
}

void sleep_mode() {
  deinit_gpio_for_sleep();
  init_sleepy_gpio();

}

int main(void) {
  HAL_Init();
  init_gpio();
  delay44x(200);

  display_init_data();
  display_write_init(0);

  int i = 0;
  while(1) {
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) || HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0)) {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0);
    } else {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 1);
      HAL_Delay(10);
      continue;
    }
    if (i > 2000) {
      i = 0;
    }
    int val = i;
    if (val > 1000) {
      val = 2000 - val;
    }
    //HAL_Delay(1);
    delay44x(50);
    display_battery_percent = i / 10;
    display_stars_fn_22f4();
    display_small_stars_fn_15b8();
    display_off_charge_mode_fn_0d90();
    display_set_charge_percent(i / 10, val / 10);
    display_set_juice_percent(((i + 1000) % 2000) / 10, val / 10);
    display_write_chunk();
    i++;
  }
  
  //return 0;
  /*while (1) {
    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
    printf("echo\r\n");
  }*/
}


void APP_ErrorHandler(void) {
  while (1);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Export assert error source and line number
  */
void assert_failed(uint8_t *file, uint32_t line) {
  /* printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  while (1);
}
#endif /* USE_FULL_ASSERT */
