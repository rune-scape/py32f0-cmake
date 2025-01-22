#include "main.h"
#include "app_config.h"
#include "app_drivers.h"
#include <stdbool.h>
#include <stdint.h>

#define vcc_millivolts_buffer_size 8
#define LEDRing_REPEAT_FOREVER 0xff

volatile uint32_t display_flags1;
uint8_t unknown00d0[16];
uint32_t v00e0;
volatile uint32_t flags4;
uint32_t display_flags2;
volatile uint32_t display_error_flags;
MainMode_t main_mode;
uint8_t power_switch_change_progress;
PowerState_t PowerSwitch_state;
volatile uint32_t awake_time;
DisplayData_t display_data;
uint8_t v00fd;
uint8_t v00fe;
uint16_t v0100;
uint16_t v0102;
volatile int current_puff_time;
volatile uint32_t current_puff_limiter_time;
uint32_t total_puff_time;
uint16_t counter0110;
uint16_t counter0112;
uint8_t v0114;
uint16_t battery_percent;
uint16_t actual_battery_percent;
int v011c;
uint32_t v0120;
uint32_t v0124;
uint32_t v0128;
int32_t vcc_millivolts;
int8_t vcc_millivolts_buffer_frame;
uint32_t v0134;
uint16_t coil_duty_cycle;
volatile uint8_t display_frame_time;
uint8_t unknown013b;
uint8_t display_current_section = 0;
uint16_t LEDRing_charge_anim_progress;
uint8_t display_charge_anim_frame;
uint16_t display_anim_frame_time;
uint8_t display_anim_frame_num;
uint32_t RCC_CSR_xRSTF_Flags;
uint8_t unknown014c[0x28];
uint32_t v0174;
uint8_t unknown0178[0x60];
LEDRingMode_t LEDRing_mode;
uint16_t LEDRing_anim_time;
uint8_t LEDRing_anim_repeat_count;
PowerState_t LEDRing_vape_level;
bool LEDRing_should_reset_anim_frame;
// indexed with DisplaySection
uint8_t display_state[DISPLAY_SECTION_COUNT] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5 };


TIM_HandleTypeDef TIM16_Handle;
TIM_HandleTypeDef TIM1_Handle;
TIM_HandleTypeDef TIM3_Handle;
TIM_OC_InitTypeDef TIM1_OC;
TIM_OC_InitTypeDef TIM3_OC;
DisplayData_t display_data_buffer[2];
int vcc_millivolts_buffer[vcc_millivolts_buffer_size];
RTC_HandleTypeDef RTC_Handle;
IWDG_HandleTypeDef IWDG_Handle;
uint32_t v0330;
uint32_t v0334;
volatile uint32_t ADC_DMA_Buffer[ADC_DMA_BUFFER_SIZE];
uint8_t unknown0378[16];
ADC_HandleTypeDef ADC_Handle;
ADC_ChannelConfTypeDef ADC_ChannelConf;
uint8_t unknown03d4[0x8c];
uint32_t FLASH_UserDataBufferData[32] = {0};
DMA_HandleTypeDef DMA_Handle;
uint8_t unknown0524[0x50];
LPTIM_HandleTypeDef LPTIM_Handle;


static void APP_FlashErase(void);
static void APP_FlashProgram(void);
static void APP_FlashBlank(void);
static void APP_FlashVerify(void);

void Display_AllOff() {
  // set annodes high
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  // set cathodes high
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

#define DISPLAY_WRITE_SEGMENT_BITS(section)\
  do {\
    if (READ_BIT(display_state[section], 0x01)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x02)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x04)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x08)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x10)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x20)) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x40)) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x80)) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);\
  } while(0)

#define DISPLAY_WRITE_CENTER_SEGMENT_BITS(section)\
  do {\
    if (READ_BIT(display_state[section], 0x01)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x02)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x04)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x08)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x10)) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);\
    if (READ_BIT(display_state[section], 0x20)) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);\
  } while(0)

void display_write_next_section() {
  if (display_current_section < 9) {
    display_current_section++;
  } else {
    display_current_section = 0;
  }

  switch (display_current_section) {
    case 0:
      Display_AllOff();
      DISPLAY_WRITE_SEGMENT_BITS(DISPLAY_SECTION_7SEG_TOP_LEFT);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      break;
    case 1:
      Display_AllOff();
      DISPLAY_WRITE_SEGMENT_BITS(DISPLAY_SECTION_7SEG_TOP_RIGHT);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
      break;
    case 3:
      Display_AllOff();
      DISPLAY_WRITE_SEGMENT_BITS(DISPLAY_SECTION_7SEG_BOT_LEFT);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
      break;
    case 4:
      Display_AllOff();
      DISPLAY_WRITE_SEGMENT_BITS(DISPLAY_SECTION_7SEG_BOT_RIGHT);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
      break;
    case 6:
      Display_AllOff();
      DISPLAY_WRITE_SEGMENT_BITS(DISPLAY_SECTION_BOTTOM);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
      break;
    case 7:
      Display_AllOff();
      DISPLAY_WRITE_SEGMENT_BITS(DISPLAY_SECTION_TOP);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
      break;
    case 2:
    case 5:
    case 8:
      Display_AllOff();
      DISPLAY_WRITE_CENTER_SEGMENT_BITS(DISPLAY_SECTION_CENTER);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
      break;
  }
}

bool display_advance_shutdown_animation() {
  static const uint8_t display_anim_data[]  = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x3f,
    0xfe, 0xfe, 0xff, 0xff, 0xff, 0xc0, 0x3f,
    0x5c, 0xdc, 0xff, 0xff, 0xff, 0xc0, 0x3f,
    0x1c, 0x9c, 0xff, 0xff, 0xff, 0xc0, 0x3f,
    0x08, 0x08, 0xff, 0xff, 0xff, 0xc0, 0x3f,
    0x00, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x3f,
    0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x3f,
    0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x39,
    0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x09,
    0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0xfe, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0x5c, 0xdc, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0x1c, 0x9c, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x08, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  display_anim_frame_time++;
  if (display_anim_frame_time > 8) {
    display_anim_frame_time = 0;
    display_anim_frame_num++;
    if (display_anim_frame_num > 23) {
      display_anim_frame_time = 0;
      display_anim_frame_num = 0;
      return true;
    }
    for (int i = 0; i < DISPLAY_SECTION_COUNT; ++i) {
      display_state[i] = display_anim_data[display_anim_frame_num * 7 + i];
    }
  }
  return false;
}

#ifdef ENABLE_RUNEYS_GAME

#define runeys_get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) + (uint32_t)(((const uint8_t *)(d))[0]))

uint32_t runeys_SuperFastHash (const char * data, int len) {
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += runeys_get16bits(data);
    tmp    = (runeys_get16bits(data + 2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2 * sizeof(uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
    case 3: hash += runeys_get16bits(data);
      hash ^= hash << 16;
      hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
      hash += hash >> 11;
      break;
    case 2: hash += runeys_get16bits(data);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;
    case 1: hash += (signed char) *data;
      hash ^= hash << 10;
      hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

static bool in_runeys_game = false;
bool update_runeys_game() {
  if (IS_FLAG_CLEARED(display_flags1, FLAGS1_UPDATE_MAIN_MODE)) {
    return true;
  }

  static bool reset_game = false;
  static uint32_t rand_seed = 0;
  static PowerState_t last_power_switch_state = POWER_STATE_OFF;
  static int last_power_switch_state_timer = -1;
  static int last_power_switch_state_counter = 0;

  static int current_score = 20;
  static int high_score = 0;

  if (PowerSwitch_state == POWER_STATE_OFF) {
    last_power_switch_state_timer = 0;
    last_power_switch_state_counter = 0;
    in_runeys_game = false;
    if (current_score > high_score) {
      high_score = current_score;
    }
  } else if (!in_runeys_game) {
    if (++last_power_switch_state_timer > 1000) {
      last_power_switch_state_counter = 0;
    }
    if (PowerSwitch_state != last_power_switch_state) {
      last_power_switch_state_timer = 0;
      last_power_switch_state_counter++;
      last_power_switch_state = PowerSwitch_state;
    }
    if (last_power_switch_state_counter >= 7 && PowerSwitch_state == POWER_STATE_NORMAL) {
      in_runeys_game = true;
      reset_game = true;
      uint32_t seed_data[] = {awake_time, vcc_millivolts, display_current_section};
      rand_seed = runeys_SuperFastHash((const char *)seed_data, sizeof(seed_data));
    }
  }

  if (!in_runeys_game) {
    return false;
  }

  static int current_dot = 0;
  static int current_dot_progress = 0;
  static int target_dot = 0;

  static int game_led_ring_anim_progress = 0;
  static int my_ledring_mode = -1;

  awake_time = 0;
  LEDRing_mode = LEDRingMode_IDLE;
  game_led_ring_anim_progress += 1;

  bool do_hit_test = false;
  bool init_level = false;
  if (PowerSwitch_state != last_power_switch_state) {
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      do_hit_test = true;
    } else if (PowerSwitch_state == POWER_STATE_NORMAL) {
      init_level = true;
    }
    last_power_switch_state = PowerSwitch_state;
  }

  if (PowerSwitch_state == POWER_STATE_NORMAL) {
    if (reset_game) {
      reset_game = false;
      current_score = 0;
      init_level = true;
    }

    if (init_level) {
      init_level = false;
      my_ledring_mode = -1;
      game_led_ring_anim_progress = 0;
      current_dot = 0;
      int last_target_dot = target_dot;
      do {
        rand_seed = runeys_SuperFastHash((const char *)&rand_seed, sizeof(rand_seed));
        target_dot = rand_seed % 5;
      } while (target_dot == last_target_dot);
    }
  }

  int dot_timing;
  if (current_score < 10) {
    dot_timing = 200 - current_score * 10;
  } else if (current_score < 100) {
    dot_timing = 100 - (current_score - 10)/2;
  } else {
    dot_timing = 50;
  }

  if (PowerSwitch_state != POWER_STATE_PULSE) {
    current_dot_progress += 1;
    if (current_dot_progress >= dot_timing) {
      current_dot_progress -= dot_timing;
      current_dot = (current_dot + 1) % 8;
    }
  }

  const int current_dot_actual = current_dot < 5 ? current_dot : 8 - current_dot;

  if (do_hit_test) {
    if (current_dot_actual == target_dot) {
      my_ledring_mode = 1;
      game_led_ring_anim_progress = 0;
      current_score++;
      init_level = true;
    } else {
      if (current_score > high_score) {
        high_score = current_score;
      }
      my_ledring_mode = 0;
      game_led_ring_anim_progress = 0;
      reset_game = true;
    }
  }

  if (my_ledring_mode == -1) {
    const int ambient_led_ring_time = (dot_timing * 8) * 3;
    const int ambient_led_ring_part_time = ambient_led_ring_time / 3;
    if (game_led_ring_anim_progress > (ambient_led_ring_time * 2)) {
      game_led_ring_anim_progress -= ambient_led_ring_time;
    }

    int led_ring_progress = game_led_ring_anim_progress % ambient_led_ring_time;
    uint32_t green = 0;
    uint32_t blue  = 0;
    uint32_t red   = 0;
    if (led_ring_progress < (ambient_led_ring_part_time * 1)) {
      red   = led_ring_progress - (ambient_led_ring_part_time * 0);
      green = 0;
      blue  = (ambient_led_ring_part_time * 1) - led_ring_progress;
    } else if (led_ring_progress < (ambient_led_ring_part_time * 2)) {
      red   = (ambient_led_ring_part_time * 2) - led_ring_progress;
      green = led_ring_progress - (ambient_led_ring_part_time * 1);
      blue  = 0;
    } else {
      red   = 0;
      green = (ambient_led_ring_part_time * 3) - led_ring_progress;
      blue  = led_ring_progress - (ambient_led_ring_part_time * 2);
    }

    red   = red   * 100 / ambient_led_ring_part_time + 10;
    green = green * 100 / ambient_led_ring_part_time + 10;
    blue  = blue  * 100 / ambient_led_ring_part_time + 10;

    const int startup_time = 1000;
    if (game_led_ring_anim_progress < startup_time) {
      red   = red   * game_led_ring_anim_progress / startup_time;
      green = green * game_led_ring_anim_progress / startup_time;
      blue  = blue  * game_led_ring_anim_progress / startup_time;
    }
    
    LEDRing_SetColor(green, blue, red);
  } else {
    const int judgment_pulse_time = (200 * 1000) / dot_timing;
    if (my_ledring_mode == 1) {
      if (game_led_ring_anim_progress <= judgment_pulse_time) {
        LEDRing_SetColor((judgment_pulse_time - game_led_ring_anim_progress) * 255 / judgment_pulse_time, 0, 0);
      } else {
        LEDRing_SetColor(0, 0, 0);
        my_ledring_mode = -1;
        game_led_ring_anim_progress = 0;
      }
    } else if (my_ledring_mode == 0) {
      if (game_led_ring_anim_progress <= judgment_pulse_time) {
        LEDRing_SetColor(0, 0, (judgment_pulse_time - game_led_ring_anim_progress) * 255 / judgment_pulse_time);
      } else {
        LEDRing_SetColor(0, 0, 0);
        my_ledring_mode = -1;
        game_led_ring_anim_progress = 0;
      }
    }
  }

  int top_number = current_score;
  if (IS_FLAG_SET(display_flags1, FLAGS1_UPDATE_STATE)) {
    static const uint8_t display_7seg_bits[20] = { 0x3f, 0x00, 0x06, 0x00, 0x5b, 0x00, 0x4f, 0x00, 0x66, 0x00, 0x6d, 0x00, 0x7d, 0x00, 0x07, 0x00, 0x7f, 0x00, 0x6f, 0x00 };
    if (current_score < 200) {
      display_state[DISPLAY_SECTION_7SEG_TOP_LEFT] = display_7seg_bits[((current_score / 10) % 10) * 2];
      display_state[DISPLAY_SECTION_7SEG_TOP_RIGHT] = display_7seg_bits[(current_score % 10) * 2];
      if (current_score > 99) {
        // display leading '1'
        display_state[DISPLAY_SECTION_7SEG_TOP_LEFT] |= 0x80;
        display_state[DISPLAY_SECTION_7SEG_TOP_RIGHT] |= 0x80;
      }
    } else {
      display_state[DISPLAY_SECTION_7SEG_TOP_RIGHT] = 0;
      display_state[DISPLAY_SECTION_7SEG_TOP_LEFT] = 0;
    }

    if (high_score < 200) {
      display_state[DISPLAY_SECTION_7SEG_BOT_LEFT] = display_7seg_bits[((high_score / 10) % 10) * 2];
      display_state[DISPLAY_SECTION_7SEG_BOT_RIGHT] = display_7seg_bits[(high_score % 10) * 2];
      if (high_score > 99) {
        // display leading '1'
        display_state[DISPLAY_SECTION_7SEG_BOT_LEFT] |= 0x80;
        display_state[DISPLAY_SECTION_7SEG_BOT_RIGHT] |= 0x80;
      }
    } else {
      display_state[DISPLAY_SECTION_7SEG_BOT_RIGHT] = 0;
      display_state[DISPLAY_SECTION_7SEG_BOT_LEFT] = 0;
    }
  }

  display_state[DISPLAY_SECTION_TOP] = 1 << current_dot_actual;
  display_state[DISPLAY_SECTION_BOTTOM] = 1 << target_dot;
  display_state[DISPLAY_SECTION_CENTER] = 0;

  return true;
}

#endif

#define DISPLAY_METER_SEP_AND_GRAPHIC_ONLY 0
#define DISPLAY_METER_ALL_ON 5
#define DISPLAY_METER_SEP_ONLY 6
#define DISPLAY_METER_ALL_OFF 7

#define DISPLAY_METER_FILL_ANIM_OFFSET 0
#define DISPLAY_METER_NO_SEP_EMPTY_ANIM_OFFSET 8
#define DISPLAY_METER_NO_SEP_FILL_ANIM_OFFSET 13

#define DISPLAY_CENTER_ALL_OFF 0
#define DISPLAY_CENTER_NO_GRAPHIC 4
#define DISPLAY_CENTER_ALL_ON 5
#define DISPLAY_CENTER_GRAPHIC_ONLY 6

#define DISPLAY_CENTER_NO_GRAPHIC_FILL_ANIM_OFFSET 0
#define DISPLAY_CENTER_FILL_ANIM_OFFSET 6
#define DISPLAY_CENTER_FILL_ANIM_OFFSET 6

void display_refresh_state() {
  static const uint8_t display_7seg_bits[20] = { 0x3f, 0x00, 0x06, 0x00, 0x5b, 0x00, 0x4f, 0x00, 0x66, 0x00, 0x6d, 0x00, 0x7d, 0x00, 0x07, 0x00, 0x7f, 0x00, 0x6f, 0x00 };
  static const uint8_t display_meter_frame_data[19] = { 0xe0, 0xe1, 0xe3, 0xe7, 0xef, 0xff, 0xc0, 0x00, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 0x20, 0x21, 0x23, 0x27, 0x2f, 0x3f };
  static const uint8_t display_center_frame_data[11] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x3f, 0x30, 0x31, 0x33, 0x37, 0x3f };

    
#ifdef ENABLE_RUNEYS_GAME
    if (in_runeys_game) {
      return;
    }
#endif

  if (IS_FLAG_SET(display_flags2, FLAGS2_SHUTDOWN_ANIMATION_FLAG)) {
    if (display_advance_shutdown_animation()) {
      CLEAR_FLAG(display_flags2, FLAGS2_SHUTDOWN_ANIMATION_FLAG);
    }
  } else {
    if (display_data.top_number < 200) {
      display_state[DISPLAY_SECTION_7SEG_TOP_LEFT] = display_7seg_bits[((display_data.top_number / 10) % 10) * 2];
      display_state[DISPLAY_SECTION_7SEG_TOP_RIGHT] = display_7seg_bits[(display_data.top_number % 10) * 2];
      if (display_data.top_number > 99) {
        // display leading '1'
        display_state[DISPLAY_SECTION_7SEG_TOP_LEFT] |= 0x80;
        display_state[DISPLAY_SECTION_7SEG_TOP_RIGHT] |= 0x80;
      }
    } else {
      display_state[DISPLAY_SECTION_7SEG_TOP_RIGHT] = 0;
      display_state[DISPLAY_SECTION_7SEG_TOP_LEFT] = 0;
    }

    if (display_data.bot_number < 200) {
      display_state[DISPLAY_SECTION_7SEG_BOT_LEFT] = display_7seg_bits[((display_data.bot_number / 10) % 10) * 2];
      display_state[DISPLAY_SECTION_7SEG_BOT_RIGHT] = display_7seg_bits[(display_data.bot_number % 10) * 2];
      if (display_data.bot_number > 99) {
        // display leading '1'
        display_state[DISPLAY_SECTION_7SEG_BOT_LEFT] |= 0x80;
        display_state[DISPLAY_SECTION_7SEG_BOT_RIGHT] |= 0x80;
      }
    } else {
      display_state[DISPLAY_SECTION_7SEG_BOT_RIGHT] = 0;
      display_state[DISPLAY_SECTION_7SEG_BOT_LEFT] = 0;
    }
    display_state[DISPLAY_SECTION_BOTTOM] = display_meter_frame_data[display_data.bot_pose];
    display_state[DISPLAY_SECTION_TOP] = display_meter_frame_data[display_data.top_pose];
    display_state[DISPLAY_SECTION_CENTER] = display_center_frame_data[display_data.mid_pose];
  }
}

#define FLASH_USER_START_ADDR 0x0800f000
#define FLASH_DATA_PRE_GUARD  0x55aa55aa
#define FLASH_DATA_POST_GUARD 0x12345678
#define TOTAL_PUFF_TIME_START 100000

typedef struct {
  uint32_t preguard;
  uint32_t value;
  uint32_t postguard;
} FLASH_TotalPuffTimeStruct;


#define FLASH_UserData ((volatile FLASH_TotalPuffTimeStruct *)FLASH_USER_START_ADDR)
#define FLASH_UserDataBuffer ((FLASH_TotalPuffTimeStruct *)FLASH_UserDataBufferData)
#define FLASH_UserDataBufferTmp (((FLASH_TotalPuffTimeStruct *)FLASH_UserDataBufferData) + 1)

uint32_t get_total_puff_time_FLASH() {
  FLASH_UserDataBufferTmp->preguard = FLASH_UserData->preguard;
  FLASH_UserDataBufferTmp->value = FLASH_UserData->value;
  FLASH_UserDataBufferTmp->postguard = FLASH_UserData->postguard;
  if (FLASH_UserDataBufferTmp->preguard == FLASH_DATA_PRE_GUARD && FLASH_UserDataBufferTmp->postguard == FLASH_DATA_POST_GUARD) {
    if (FLASH_UserDataBufferTmp->value > TOTAL_PUFF_TIME_START) {
      return FLASH_UserDataBufferTmp->value;
    }
  }
  return 0;
}

void write_total_puff_time_FLASH(uint32_t total_puff_time) {
  FLASH_UserDataBuffer->preguard = FLASH_DATA_PRE_GUARD;
  FLASH_UserDataBuffer->value = total_puff_time;
  FLASH_UserDataBuffer->postguard = FLASH_DATA_POST_GUARD;
  HAL_FLASH_Unlock();
  APP_FlashErase();
  APP_FlashBlank();
  APP_FlashProgram();
  HAL_FLASH_Lock();
  APP_FlashVerify();
}


static void APP_FlashErase() {
  uint32_t PAGEError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;

  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGEERASE;
  EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
  EraseInitStruct.NbPages  = sizeof(FLASH_UserDataBufferData) / FLASH_PAGE_SIZE;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
    APP_ErrorHandler();
  }
}

static void APP_FlashBlank() {
  uint32_t addr = 0;

  while (addr < sizeof(FLASH_UserDataBufferData)) {
    if (0xFFFFFFFF != HW32_REG(FLASH_USER_START_ADDR + addr)) {
      APP_ErrorHandler();
    }
    addr += 4;
  }
}

static void APP_FlashProgram() {
  uint32_t flash_program_start = FLASH_USER_START_ADDR;
  uint32_t flash_program_end = (FLASH_USER_START_ADDR + sizeof(FLASH_UserDataBufferData));
  uint32_t *src = (uint32_t *)FLASH_UserDataBufferData;

  while (flash_program_start < flash_program_end) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, flash_program_start, src) == HAL_OK) {
      flash_program_start += FLASH_PAGE_SIZE;
      src += FLASH_PAGE_SIZE / 4;
    }
  }
}

static void APP_FlashVerify() {
  uint32_t addr = 0;

  while (addr < sizeof(FLASH_UserDataBufferData)) {
    if (FLASH_UserDataBufferData[addr / 4] != HW32_REG(FLASH_USER_START_ADDR + addr)) {
      APP_ErrorHandler();
    }
    addr += 4;
  }
}

static int get_VCC_millivolts() {
  uint32_t buf[6] = {0};

  for (uint8_t i = 0; i < ADC_DMA_CHANNEL_BUFFER_SIZE; ++i) {
    buf[i] = ADC_DMA_GET_VINTREF_VALUE(i);
  }

  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_DMA_CHANNEL_BUFFER_SIZE; ++i) {
    sum += ADC_DMA_GET_VINTREF_VALUE(i);
  }
  
  uint32_t value = sum / ADC_DMA_CHANNEL_BUFFER_SIZE;
  if (value > 0xfff) {
    return vcc_millivolts;
  } else {
    return 4914000 / value; // magic num
  }
}

const uint16_t battery_percent_map[21] = {
  3507, 3587, 3619, 3643,
  3661, 3675, 3687, 3697,
  3709, 3721, 3746, 3766,
  3793, 3822, 3852, 3882,
  3912, 3972, 4012, 4075,
  4196
};

uint8_t battery_millivolts_to_percent(uint16_t millivolts) {
  uint8_t i;
  for (i = 0; i < 20; ++i) {
    if ((millivolts + 60) <= battery_percent_map[i]) {
      break;
    }
  }
  return i * 5;
}

const uint32_t puff_time_initial_value = 60000;
const uint32_t puff_time_percent_step = 27000;
uint8_t convert_puff_time_to_juice_percent(uint32_t puff_time) {
  if (puff_time > puff_time_initial_value) {
    puff_time -= puff_time_initial_value;
  } else {
    puff_time = 0;
  }

  uint32_t percent = puff_time / puff_time_percent_step;
  if (percent < 100) {
    return 100 - percent;
  } else {
    return 0;
  }
}

void Display_RefreshNumbers() {
  if (PowerSwitch_state == 0) {
    display_data.top_number = 201;
    display_data.bot_number = 201;
    display_data.mid_pose = 0;
    display_data.bot_pose = 7;
    display_data.top_pose = 7;
    v00fd = 0;
  } else {
    display_data.top_number = battery_percent;
    display_data.bot_number = convert_puff_time_to_juice_percent(total_puff_time);
    if (PowerSwitch_state == 1) {
      display_data.mid_pose = 4;
    } else {
      display_data.mid_pose = 6;
    }
    display_data.bot_pose = (display_data.bot_number + 19) / 20;
    display_data.top_pose = (display_data.top_number + 19) / 20;
    v00fd = 1;
  }
}

void update_charging_state() {
  // if we are not charged
  if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET) {
    if (IS_FLAG_CLEARED(display_flags1, FLAGS1_IS_CHARGER_ENABLED_FLAG)) {
      Charger_Enable();
    }

    SET_FLAG(display_flags1, FLAGS1_IS_CHARGER_ENABLED_FLAG);
    awake_time = 0;

    SET_FLAG(display_flags1, FLAGS1_FLAG2);
    v0124 += 10;
  } else {
    if (IS_FLAG_SET(display_flags1, FLAGS1_IS_CHARGER_ENABLED_FLAG)) {
      Charger_Disable();
      Display_RefreshNumbers();
      LEDRing_mode = LEDRingMode_OFF;
      LEDRing_should_reset_anim_frame = true;
    }

    CLEAR_FLAG(display_flags1, FLAGS1_IS_CHARGER_ENABLED_FLAG);
    CLEAR_FLAG(display_flags1, FLAGS1_FLAG2);
    v00fe = 0;
    v0100 = 0;
  }
}

bool LEDRing_FlashColor(int time, int repeat_count, uint8_t green, uint8_t blue, uint8_t red) {
  if (time % 100 == 1) {
    LEDRing_SetColor(0, 0, 0);
  } else if (time % 100 == 50) {
    LEDRing_SetColor(green, blue, red);
  }
  if ((time / 100) >= repeat_count) {
    LEDRing_SetColor(0, 0, 0);
    return true;
  }

  return false;
}

bool LEDRing_FadeIn(int time, PowerState_t vape_level) {
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t red = 0;

  if (vape_level == POWER_STATE_PULSE) {
    red = 255;
    green = 0;
    blue = 0;
  } else {
    red = 0;
    green = 255;
    blue = 0;
  }

  if (time < 50) {
    uint16_t mult = time * 2;
    red = mult * red / 100;
    green = mult * green / 100;
    blue = mult * blue / 100;
  }

  LEDRing_SetColor(green, blue, red);
  return false;
}

bool LEDRing_FadeOut(int time, PowerState_t vape_level) {
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t red = 0;

  if (vape_level == POWER_STATE_PULSE) {
    red = 255;
    green = 0;
    blue = 0;
  } else {
    red = 0;
    green = 255;
    blue = 0;
  }

  if (time < 100) {
    LEDRing_SetColor(green, blue, red);
    return false;
  } else if (time < 200) {
    uint16_t mult = 200 - time;
    red = mult * red / 100;
    green = mult * green / 100;
    blue = mult * blue / 100;
    LEDRing_SetColor(green, blue, red);
    return false;
  } else {
    LEDRing_SetColor(0, 0, 0);
    return true;
  }
}

bool LEDRing_BatteryChargingBreathe(int time) {
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t red = 0;

  if (time <= (255 * 2)) {
    if (time > 255) {
      time = (255 * 2) - time;
    }
    if (battery_percent < 100) {
      green = time;
    } else {
      green = 255;
    }

    LEDRing_SetColor(green, 0, 0);
    return false;
  }
  return true;
}

bool LEDRing_TestColors(int time, int p2) {
  if (time == 1) {
    LEDRing_SetColor(0, 0, 0);
  } else if (time == 70) {
    if (p2 == 0) {
      LEDRing_SetColor(255, 0, 0);
    } else {
      LEDRing_SetColor(0, 0, 255);
    }
  } else if (time == 100) {
    LEDRing_SetColor(0, 0, 0);
  } else if (time == 120) {
    LEDRing_SetColor(0, 255, 0);
  } else if (time == 150) {
    LEDRing_SetColor(0, 0, 0);
  } else if (time == 170) {
    if (p2 == 0) {
      LEDRing_SetColor(0, 0, 255);
    } else {
      LEDRing_SetColor(255, 0, 0);
    }
  } else if (time == 200) {
    LEDRing_SetColor(0, 0, 0);
  } else if (time > 200) {
    return true;
  }

  return false;
}

bool LEDRing_Intro(int time, PowerState_t vape_level) {
  if (time >= 201) {
    return true;
  }

  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t red = 0;
  if (time > (200 - 85)) {
    time = 200 - time;
  } else if (time > 85) {
    time = 85;
  }
  if (vape_level == POWER_STATE_PULSE) {
    red = time * 3;
  } else if (vape_level == POWER_STATE_NORMAL) {
    green = time * 3;
  } else {
    green = time * 3;
    red = time * 3;
  }

  LEDRing_SetColor(green, blue, red);
  return false;
}

void LEDRing_Update() {
  if (++LEDRing_anim_time > 60000) {
    LEDRing_anim_time = 50000;
  }
  if (LEDRing_should_reset_anim_frame) {
    LEDRing_should_reset_anim_frame = false;
    LEDRing_anim_time = 1;
  }

  switch(LEDRing_mode) {
    case LEDRingMode_IDLE: // 0x08004d6a + 0x05*2 = 0x08004D74
      // do nothing
      break;
    case LEDRingMode_FLASHING: // 0x08004d6a + 0x06*2 = 0x08004D76
      if (LEDRing_FlashColor(LEDRing_anim_time, LEDRing_anim_repeat_count, 0, 0, 255)) {
        LEDRing_mode = LEDRingMode_OFF;
      }
      break;
    case LEDRingMode_FADE_IN: // 0x08004d6a + 0x16*2 = 0x08004D96
      LEDRing_FadeIn(LEDRing_anim_time, LEDRing_vape_level);
      break;

    case LEDRingMode_FADE_OUT: // 0x08004d6a + 0x1d*2 = 0x08004DA4
      if (LEDRing_FadeOut(LEDRing_anim_time, LEDRing_vape_level)) {
        LEDRing_mode = LEDRingMode_OFF;
      }
      break;
    case LEDRingMode_CHARGING_BREATE: // 0x08004d6a + 0x29*2 = 0x08004DBC
      if (LEDRing_BatteryChargingBreathe(LEDRing_anim_time)) {
        LEDRing_anim_time = 0;
        if (LEDRing_anim_repeat_count < 2) {
          LEDRing_mode = LEDRingMode_OFF;
        } else if (LEDRing_anim_repeat_count != LEDRing_REPEAT_FOREVER) {
          LEDRing_anim_repeat_count--;
        }
      }
      break;
    case LEDRingMode_TEST_COLORS: // 0x08004d6a + 0x44*2 = 0x08004DF2
      if (LEDRing_TestColors(LEDRing_anim_time, LEDRing_mode % 2)) {
        LEDRing_anim_time = 0;
        if (LEDRing_anim_repeat_count < 2) {
          LEDRing_mode = LEDRingMode_OFF;
        } else if (LEDRing_anim_repeat_count != LEDRing_REPEAT_FOREVER) {
          LEDRing_anim_repeat_count--;
        }
      }
      break;
    case LEDRingMode_INTRO: // 0x08004d6a + 0x63*2 = 0x08004E30
      if (LEDRing_Intro(LEDRing_anim_time, LEDRing_vape_level)) {
        LEDRing_anim_time = 0;
        if (LEDRing_anim_repeat_count < 2) {
          LEDRing_mode = LEDRingMode_OFF;
        } else if (LEDRing_anim_repeat_count != LEDRing_REPEAT_FOREVER) {
          LEDRing_anim_repeat_count--;
        }
      }
      break;
    default: // 0x08004d6a + 0x80*2 = 0x08004E6A
      LEDRing_SetColor(0, 0, 0);
      LEDRing_mode = LEDRingMode_IDLE;
      LEDRing_anim_repeat_count = 0;
      LEDRing_anim_time = 0;
      break;
  }
}

uint16_t calc_coil_duty_cycle(int full_power_threshold_mA, int vcc_mA) {
  static uint16_t coil_voltage_scale_table[100] = { 1000, 995, 990, 985, 980, 975, 970, 964, 959, 954, 949, 943, 938, 933, 927, 922, 917, 911, 906, 900, 894, 889, 883, 877, 872, 866, 860, 854, 849, 843, 837, 831, 825, 819, 812, 806, 800, 794, 787, 781, 775, 768, 762, 755, 748, 742, 735, 728, 721, 714, 707, 700, 693, 686, 678, 671, 663, 656, 648, 640, 632, 624, 616, 608, 600, 592, 583, 574, 566, 557, 548, 539, 529, 520, 510, 500, 490, 480, 469, 458, 447, 436, 424, 412, 400, 387, 374, 361, 346, 332, 316, 300, 283, 265, 245, 224, 200, 173, 141, 100 };

  if (vcc_mA < full_power_threshold_mA) {
    return 100;
  }

  uint32_t scale = full_power_threshold_mA * 1000 / vcc_mA;
  uint32_t result;
  // danger, not safe with the array
  for (result = 100; scale < coil_voltage_scale_table[100 - result]; --result);
  return result;
}

uint16_t PowerSwitch_AnalogRead() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_DMA_CHANNEL_BUFFER_SIZE; ++i) {
    sum += ADC_DMA_GET_POWER_SWITCH_VALUE(i);
  }
  return sum / ADC_DMA_CHANNEL_BUFFER_SIZE;
}

void Display_UpdateChargeAnim() {
  if (PowerSwitch_state == 0) {
    if (++LEDRing_charge_anim_progress > 500) {
      LEDRing_charge_anim_progress = 0;
      display_charge_anim_frame++;
      if (display_charge_anim_frame > 5) {
        display_charge_anim_frame = battery_percent / 20;
      }
      display_data.top_pose = DISPLAY_METER_NO_SEP_FILL_ANIM_OFFSET + display_charge_anim_frame;
    }
  } else {
    if (++LEDRing_charge_anim_progress > 300) {
      LEDRing_charge_anim_progress = 0;
      display_charge_anim_frame++;
      if (display_charge_anim_frame <= 5) {
        display_data.top_pose = DISPLAY_METER_FILL_ANIM_OFFSET + display_charge_anim_frame;
        display_data.bot_pose = DISPLAY_METER_SEP_AND_GRAPHIC_ONLY;
        display_data.mid_pose = DISPLAY_CENTER_ALL_OFF;
      } else if (display_charge_anim_frame <= 10) {
        display_data.top_pose = DISPLAY_METER_ALL_ON;
        display_data.bot_pose = DISPLAY_METER_NO_SEP_EMPTY_ANIM_OFFSET + (display_charge_anim_frame - 6);
        display_data.mid_pose = DISPLAY_CENTER_ALL_OFF;
      } else if (display_charge_anim_frame <= 15) {
        display_data.top_pose = DISPLAY_METER_ALL_ON;
        display_data.bot_pose = DISPLAY_METER_ALL_ON;
        display_data.mid_pose = DISPLAY_CENTER_NO_GRAPHIC_FILL_ANIM_OFFSET + (display_charge_anim_frame - 10);
      } else {
        display_data.top_pose = DISPLAY_METER_SEP_AND_GRAPHIC_ONLY;
        display_data.bot_pose = DISPLAY_METER_SEP_AND_GRAPHIC_ONLY;
        display_data.mid_pose = DISPLAY_CENTER_ALL_OFF;
        display_charge_anim_frame = 0;
      }
    }
  }
}

#define POWER_SWITCH_LOWER_THRESHOLD 300
#define POWER_SWITCH_MIDDLE_APPROX 600
#define POWER_SWITCH_UPPER_THRESHOLD 1000

void PowerSwitch_UpdateState() {
  uint16_t power_switch_v = PowerSwitch_AnalogRead();
  if (PowerSwitch_state == POWER_STATE_OFF) {
    if (power_switch_v < POWER_SWITCH_LOWER_THRESHOLD || power_switch_v > POWER_SWITCH_UPPER_THRESHOLD) {
      if (++power_switch_change_progress > 50) {
        power_switch_change_progress = 0;
        if (power_switch_v < POWER_SWITCH_LOWER_THRESHOLD) {
          PowerSwitch_state = POWER_STATE_PULSE;
        } else {
          PowerSwitch_state = POWER_STATE_NORMAL;
        }
        SET_FLAG(display_flags2, FLAGS2_POWER_SWITCH_CHANGED_FLAG);
        awake_time = 0;
      }
    } else {
      power_switch_change_progress = 0;
    }
  } else if (PowerSwitch_state == POWER_STATE_NORMAL) {
    if (power_switch_v < POWER_SWITCH_UPPER_THRESHOLD) {
      if (++power_switch_change_progress > 50) {
        power_switch_change_progress = 0;
        if (power_switch_v < POWER_SWITCH_LOWER_THRESHOLD) {
          PowerSwitch_state = POWER_STATE_PULSE;
        } else {
          PowerSwitch_state = POWER_STATE_OFF;
        }
        SET_FLAG(display_flags2, FLAGS2_POWER_SWITCH_CHANGED_FLAG);
        awake_time = 0;
      }
    } else {
      power_switch_change_progress = 0;
    }
  } else {
    if (power_switch_v > POWER_SWITCH_LOWER_THRESHOLD) {
      if (++power_switch_change_progress > 50) {
        power_switch_change_progress = 0;
        if (power_switch_v < POWER_SWITCH_MIDDLE_APPROX) {
          PowerSwitch_state = POWER_STATE_OFF;
        } else {
          PowerSwitch_state = POWER_STATE_NORMAL;
        }
        SET_FLAG(display_flags2, FLAGS2_POWER_SWITCH_CHANGED_FLAG);
        awake_time = 0;
      }
    } else {
      power_switch_change_progress = 0;
    }
  }
}

bool Display_Alert(uint16_t progress, int repeat_count, DisplayData_t *p3) {
  if (progress % 1000 == 1) {
    display_data.top_number = p3[0].top_number;
    display_data.bot_number = p3[0].bot_number;
    display_data.mid_pose = p3[0].mid_pose;
    display_data.bot_pose = p3[0].bot_pose;
    display_data.top_pose = p3[0].top_pose;
    display_refresh_state();
    LEDRing_SetColor(0, 0, 0);
  } else if (progress % 1000 == 500) {
    display_data.top_number = p3[1].top_number;
    display_data.bot_number = p3[1].bot_number;
    display_data.mid_pose = p3[1].mid_pose;
    display_data.bot_pose = p3[1].bot_pose;
    display_data.top_pose = p3[1].top_pose;
    display_refresh_state();
    LEDRing_SetColor(0, 0, 255);
  }

  if (progress / 1000 >= repeat_count) {
    LEDRing_SetColor(0, 0, 0);
    return true;
  }

  return false;
}

bool fn5058() {
  if (vcc_millivolts < 2950) {
    if (++v0114 > 8) {
      SET_FLAG(display_error_flags, FLAGS3_FLAG2);
      v0114 = 0;
      return false;
    }
  } else {
    v0114 = 0;
  }

  return true;
}

uint32_t fn0688() {
  uint32_t v2 = 0;
  uint32_t v3 = 0;

  Display_AllOff();

  CoilTB2_PWM_SetDutyCycle(100);
  CoilTB1_PWM_SetDutyCycle(100);
  WRITE_REG(TIM1->CNT, 500 - 1);
  delay44x(50);

  for (uint8_t i = 0; i < 4; ++i) {
    v2 += ADC_DMA_GET_TB2_VALUE(i);
    v3 += ADC_DMA_GET_TB1_VALUE(i);
  }

  CoilTB2_PWM_SetDutyCycle(0);
  CoilTB1_PWM_SetDutyCycle(0);
  WRITE_REG(TIM1->CNT, 500 - 1);

  if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG0)) {
    return 0;
  }

  if ((v2 / ADC_DMA_CHANNEL_BUFFER_SIZE) < 3900) {
    return 0;
  } else if ((v3 / ADC_DMA_CHANNEL_BUFFER_SIZE) < 3900) {
    return 0;
  } else if ((v2 / ADC_DMA_CHANNEL_BUFFER_SIZE) > 4080) {
    return 9999;
  } else if ((v3 / ADC_DMA_CHANNEL_BUFFER_SIZE) > 4080) {
    return 9999;
  } else {
    return 1100;
  }
}

bool fn4f58() { 
  display_error_flags = 0;
  if (vcc_millivolts < 3400 || (battery_percent == 0 && vcc_millivolts < 3500)) {
    SET_FLAG(display_error_flags, FLAGS3_FLAG2);
    return false;
  }

  v0134 = fn0688();
  CLEAR_FLAG(display_error_flags, FLAGS3_FLAG5);
  if (v0134 > 2200) {
    SET_FLAG(display_error_flags, FLAGS3_FLAG1);
    return false;
  } else if (v0134 < 300) {
    SET_FLAG(display_error_flags, FLAGS3_FLAG0);
    return false;
  } else {
    return true;
  }
}

void sleepy_loop() {
  HAL_DMA_DeInit(&DMA_Handle);
  __HAL_RCC_DMA_CLK_DISABLE();
  delay44x(10);

  __HAL_RCC_DMA_FORCE_RESET();
  __HAL_RCC_DMA_RELEASE_RESET();

  HAL_ADC_DeInit(&ADC_Handle);
  __HAL_RCC_ADC_CLK_DISABLE();

  HAL_NVIC_DisableIRQ(TIM16_IRQn);
  HAL_TIM_Base_DeInit(&TIM16_Handle);
  __HAL_RCC_TIM16_CLK_DISABLE();

  HAL_TIM_PWM_Stop(&TIM1_Handle, TIM_CHANNEL_3);
  HAL_TIM_PWM_Stop(&TIM1_Handle, TIM_CHANNEL_4);
  HAL_TIM_PWM_DeInit(&TIM1_Handle);
  __HAL_RCC_TIM1_CLK_DISABLE();

  HAL_TIM_PWM_Stop(&TIM3_Handle, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&TIM3_Handle, TIM_CHANNEL_2);
  HAL_TIM_PWM_Stop(&TIM3_Handle, TIM_CHANNEL_3);
  HAL_TIM_PWM_DeInit(&TIM3_Handle);
  __HAL_RCC_TIM3_CLK_DISABLE();

  APP_SystemClockConfig_Sleep();
  APP_GPIO_Config_Sleep();
  SWD_Disable();

  SET_FLAG(flags4, FLAGS4_FLAG4);
  while (IS_FLAG_SET(flags4, FLAGS4_FLAG4)) {
    HAL_SuspendTick();
    __HAL_RCC_LPTIM_CLK_ENABLE();
    SET_BIT(PWR->CR1, PWR_CR1_BIAS_CR_3);
    CLEAR_BIT(LPTIM->CR, LPTIM_CR_ENABLE);
    SET_BIT(LPTIM->CR, LPTIM_CR_ENABLE);
    delay44x(10);

    SET_BIT(LPTIM->CR, LPTIM_CR_SNGSTRT);
    SET_BIT(LPTIM->IER, LPTIM_IER_ARRMIE);
    WRITE_REG(LPTIM->ARR, 102);
    if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_SET) {
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    } else {
      CLEAR_FLAG(flags4, FLAGS4_FLAG4);
    }
    HAL_ResumeTick();
    if (HAL_IWDG_Refresh(&IWDG_Handle) != HAL_OK) {
      APP_ErrorHandler();
    }
  }

  SET_BIT(PWR->CR1, PWR_CR1_VOS);
  HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
  HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
  APP_MainConfig();
  CLEAR_FLAG(flags4, FLAGS4_FLAG3);
  v00e0 = 0;
  v0174 = 0;
  SWD_Enable();
}

void update_mode0() {
  if (IS_FLAG_CLEARED(display_flags1, FLAGS1_UPDATE_MAIN_MODE)) {
    return;
  }
  CLEAR_FLAG(display_flags1, FLAGS1_UPDATE_MAIN_MODE);
  
  current_puff_time++;
  if (current_puff_time == 60000) {
    current_puff_time = 20000;
  }

  PowerSwitch_UpdateState();

  if (IS_FLAG_SET(display_flags2, FLAGS2_POWER_SWITCH_CHANGED_FLAG)) {
    CLEAR_FLAG(display_flags2, FLAGS2_POWER_SWITCH_CHANGED_FLAG);
    v0102 = 0;
    LEDRing_charge_anim_progress = 0;
    Display_RefreshNumbers();
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      LEDRing_mode = LEDRingMode_INTRO;
      LEDRing_vape_level = POWER_STATE_PULSE;
      LEDRing_anim_repeat_count = 1;
      LEDRing_should_reset_anim_frame = true;
      CLEAR_FLAG(display_flags2, FLAGS2_SHUTDOWN_ANIMATION_FLAG);
    } else if (PowerSwitch_state == POWER_STATE_NORMAL) {
      LEDRing_mode = LEDRingMode_INTRO;
      LEDRing_vape_level = POWER_STATE_NORMAL;
      LEDRing_anim_repeat_count = 1;
      LEDRing_should_reset_anim_frame = true;
      CLEAR_FLAG(display_flags2, FLAGS2_SHUTDOWN_ANIMATION_FLAG);
    } else if (PowerSwitch_state == POWER_STATE_OFF) {
      LEDRing_mode = LEDRingMode_INTRO;
      LEDRing_vape_level = POWER_STATE_OFF;
      LEDRing_anim_repeat_count = 1;
      LEDRing_should_reset_anim_frame = true;
      SET_FLAG(display_flags2, FLAGS2_SHUTDOWN_ANIMATION_FLAG);
      display_anim_frame_num = 0;
      display_anim_frame_time = 0;
    }
  }

  if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_SET && PowerSwitch_state != POWER_STATE_OFF && IS_FLAG_CLEARED(display_flags1, FLAGS1_FLAG2)) {
    if (IS_FLAG_CLEARED(display_flags1, FLAGS1_FLAG1) && current_puff_limiter_time > 4) {
      Display_RefreshNumbers();
      if (fn4f58()) {
        display_data.bot_number = convert_puff_time_to_juice_percent(total_puff_time);
        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
        main_mode = MAIN_MODE_1;
        current_puff_time = 0;
      } else {
        main_mode = MAIN_MODE_2;
        current_puff_time = 0;
        CLEAR_FLAG(display_flags1, FLAGS1_FLAG7);
        LEDRing_mode = LEDRingMode_OFF;
      }
    }
  } else {
    CLEAR_FLAG(display_flags1, FLAGS1_FLAG1);
  }

  if (IS_FLAG_SET(display_flags1, FLAGS1_FLAG7) && PowerSwitch_state == POWER_STATE_PULSE) {
    if (current_puff_time <= 2000) {
      if (current_puff_time % 150 == 0) {
        if (display_data.mid_pose < 10) {
          display_data.mid_pose++;
        } else {
          display_data.mid_pose = 7;
        }
      }
    } else {
      CLEAR_FLAG(display_flags1, FLAGS1_FLAG7);
      Display_RefreshNumbers();
    }
  } else {
    CLEAR_FLAG(display_flags1, FLAGS1_FLAG7);
  }

  if (current_puff_time == 1) {
    v0102 = 0;
    if (v0120 <= v011c) {
      v011c -= v0120;
      if (battery_percent != 0) {
        battery_percent--;
      }
    }
    if (IS_FLAG_CLEARED(display_flags1, FLAGS1_FLAG7)) {
      Display_RefreshNumbers();
    }
  }

  if (current_puff_time % 1000 == 0) {
    actual_battery_percent = battery_millivolts_to_percent(vcc_millivolts);
    if (battery_percent >= actual_battery_percent + 20) {
      v0120 = 2000;
      v0128 = 60000;
    } else if (battery_percent >= actual_battery_percent + 10) {
      v0120 = 3000;
      v0128 = 50000;
    } else if (battery_percent >= actual_battery_percent + 5) {
      v0120 = 4000;
      v0128 = 45000;
    } else if (actual_battery_percent >= battery_percent + 20) {
      v0120 = 15000;
      v0128 = 22000;
    } else if (actual_battery_percent >= battery_percent + 10) {
      v0120 = 10000;
      v0128 = 30000;
    } else if (actual_battery_percent >= battery_percent + 5) {
      v0120 = 6000;
      v0128 = 36000;
    } else {
      v0120 = 5000;
      v0128 = 40000;
    }
  }

  if (IS_FLAG_SET(display_flags1, FLAGS1_FLAG2)) {
    if (IS_FLAG_CLEARED(display_flags1, FLAGS1_FLAG7)) {
      if (++v0102 == 1000) {
        LEDRing_mode = LEDRingMode_CHARGING_BREATE;
        LEDRing_anim_repeat_count = LEDRing_REPEAT_FOREVER;
        LEDRing_should_reset_anim_frame = true;
      } else if (v0102 >= (6000 + 120)) {
        v0102 = 2040;
      }

      if (v0124 >= v0128) {
        v0124 -= v0128;
        if (battery_percent < 100) {
          battery_percent++;
        }
        if (PowerSwitch_state != POWER_STATE_OFF) {
          display_data.top_number = battery_percent;
        }
      }

      Display_UpdateChargeAnim();
    }
  } else {
    v0102 = 0;
    LEDRing_charge_anim_progress = 0;
  }

  if (awake_time > 10000) {
    awake_time = 0;
    sleepy_loop();
    current_puff_time = 0;
  }
}

#define MULTIPLY_BY_1_890625(v) ((v * 484) >> 8)

void update_mode1() {
  if (IS_FLAG_CLEARED(display_flags1, FLAGS1_UPDATE_MAIN_MODE)) {
    return;
  }
  CLEAR_FLAG(display_flags1, FLAGS1_UPDATE_MAIN_MODE);

  if (++current_puff_time == 1) {
    LEDRing_mode = LEDRingMode_FADE_IN;
    LEDRing_should_reset_anim_frame = true;
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      display_data.mid_pose = 6;
    } else {
      display_data.mid_pose = 0;
      if (IS_FLAG_SET(display_flags2, FLAGS2_FLAG6)) {
        if (counter0112 < counter0110) {
          counter0112++;
        } else {
          CLEAR_FLAG(display_flags2, FLAGS2_FLAG6);
          counter0110++;
        }
      } else {
        if (counter0110 < (counter0112 + 5)) {
          counter0110++;
        } else {
          SET_FLAG(display_flags2, FLAGS2_FLAG6);
          counter0112++;
        }
      }
    }
  }

  if (current_puff_time % 50 == 1) {
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      if (vcc_millivolts < 3600) {
        coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA, vcc_millivolts);
        CoilTB1_PWM_SetDutyCycle(coil_duty_cycle);
        CoilTB2_PWM_SetDutyCycle(coil_duty_cycle);
      } else {
        if (total_puff_time < LOW_POWER_PUFF_TIME_THRESHOLD_ms) {
          coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA + 200, vcc_millivolts);
        } else {
          coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA, vcc_millivolts);
        }
        CoilTB1_PWM_SetDutyCycle(coil_duty_cycle);
        
        coil_duty_cycle = calc_coil_duty_cycle(HIGH_VOLTAGE_COIL2_THRESHOLD_mA, vcc_millivolts);
        CoilTB2_PWM_SetDutyCycle(coil_duty_cycle);
      }
    } else {
      if (IS_FLAG_SET(display_flags2, FLAGS2_FLAG6)) {
        if (total_puff_time < LOW_POWER_PUFF_TIME_THRESHOLD_ms) {
          coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA + 200, vcc_millivolts);
        } else {
          coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA, vcc_millivolts);
        }
        CoilTB2_PWM_SetDutyCycle(0);
        CoilTB1_PWM_SetDutyCycle(coil_duty_cycle);
      } else {
        if (total_puff_time < LOW_POWER_PUFF_TIME_THRESHOLD_ms) {
          coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA + 200, vcc_millivolts);
        } else {
          coil_duty_cycle = calc_coil_duty_cycle(COIL_THRESHOLD_mA, vcc_millivolts);
        }
        CoilTB2_PWM_SetDutyCycle(coil_duty_cycle);
        CoilTB1_PWM_SetDutyCycle(0);
      }
    }
  }

  if (PowerSwitch_state == POWER_STATE_PULSE) {
    if (current_puff_time % 150 == 0) {
      if (display_data.mid_pose < 10) {
        display_data.mid_pose++;
      } else {
        display_data.mid_pose = 7;
      }
    }
  } else {
    if (current_puff_time % 200 == 0) {
      if (display_data.mid_pose < 4) {
        display_data.mid_pose++;
      } else {
        display_data.mid_pose = 1;
      }
    }
  }

  if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG0)) {
    AllCoilsOff();
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      current_puff_time = MULTIPLY_BY_1_890625(current_puff_time);
    }
    total_puff_time += current_puff_time;
    v011c += current_puff_time;
    main_mode = MAIN_MODE_2;
    current_puff_time = 0;
    Display_RefreshNumbers();
    LEDRing_mode = LEDRingMode_OFF;
  } else if (current_puff_time > 9000) {
    AllCoilsOff();
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      current_puff_time = MULTIPLY_BY_1_890625(current_puff_time);
    }
    total_puff_time += current_puff_time;
    v011c += current_puff_time;
    SET_FLAG(display_error_flags, FLAGS3_FLAG5);
    main_mode = MAIN_MODE_2;
    current_puff_time = 0;
    Display_RefreshNumbers();
    LEDRing_mode = LEDRingMode_OFF;
  } else if (!fn5058()) {
    AllCoilsOff();
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      current_puff_time = MULTIPLY_BY_1_890625(current_puff_time);
    }
    total_puff_time += current_puff_time;
    v011c += current_puff_time;
    main_mode = MAIN_MODE_2;
    current_puff_time = 0;
    Display_RefreshNumbers();
    LEDRing_mode = LEDRingMode_OFF;
  } else if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_RESET) { // if we are no longer vaping
    AllCoilsOff();
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    if (PowerSwitch_state == POWER_STATE_PULSE) {
      current_puff_time = MULTIPLY_BY_1_890625(current_puff_time);
    }
    total_puff_time += current_puff_time;
    write_total_puff_time_FLASH(total_puff_time);
    v011c += current_puff_time;
    display_data.bot_number = convert_puff_time_to_juice_percent(total_puff_time);
    if (display_data.bot_number < 6) {
      SET_FLAG(display_error_flags, FLAGS3_FLAG4);
      main_mode = MAIN_MODE_2;
      CLEAR_FLAG(display_flags1, FLAGS1_FLAG7);
      Display_RefreshNumbers();
      LEDRing_mode = LEDRingMode_OFF;
    } else {
      main_mode = MAIN_MODE_0;
      LEDRing_mode = LEDRingMode_FADE_OUT;
      LEDRing_should_reset_anim_frame = true;
      SET_FLAG(display_flags1, FLAGS1_FLAG7);
    }
    current_puff_time = 0;
  }
}

void update_mode2() {
  if (IS_FLAG_CLEARED(display_flags1, FLAGS1_UPDATE_MAIN_MODE)) {
    return;
  }
  CLEAR_FLAG(display_flags1, FLAGS1_UPDATE_MAIN_MODE);

  bool anim_done = 
  current_puff_time++;
  if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG5)) {
    if (current_puff_time == 1) {
      display_data_buffer[0].top_number = display_data.top_number;
      display_data_buffer[0].bot_number = display_data.bot_number;
      display_data_buffer[0].mid_pose = 0;
      display_data_buffer[0].bot_pose = display_data.bot_pose;
      display_data_buffer[0].top_pose = display_data.top_pose;
      display_data_buffer[1].top_number = display_data.top_number;
      display_data_buffer[1].bot_number = display_data.bot_number;
      display_data_buffer[1].mid_pose = 5;
      display_data_buffer[1].bot_pose = display_data.bot_pose;
      display_data_buffer[1].top_pose = display_data.top_pose;
    }
    if (Display_Alert(current_puff_time, 3, display_data_buffer)) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG5);
      main_mode = MAIN_MODE_0;
      current_puff_time = 0;
      awake_time = 0;
      SET_FLAG(display_flags1, FLAGS1_FLAG1);
    }
  } else if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG1)) {
    if (current_puff_time == 1) {
      display_data_buffer[0].top_number = 201;
      display_data_buffer[0].bot_number = 201;
      display_data_buffer[0].mid_pose = 0;
      display_data_buffer[0].bot_pose = 7;
      display_data_buffer[0].top_pose = 7;
      display_data_buffer[1].top_number = display_data.top_number;
      display_data_buffer[1].bot_number = display_data.bot_number;
      display_data_buffer[1].mid_pose = display_data.mid_pose;
      display_data_buffer[1].bot_pose = display_data.bot_pose;
      display_data_buffer[1].top_pose = display_data.top_pose;
    }
    if (Display_Alert(current_puff_time, 3, display_data_buffer)) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG1);
      main_mode = MAIN_MODE_0;
      current_puff_time = 0;
      awake_time = 0;
      SET_FLAG(display_flags1, FLAGS1_FLAG1);
    }
  } else if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG0)) {
    if (current_puff_time == 1) {
      display_data_buffer[0].top_number = 201;
      display_data_buffer[0].bot_number = 201;
      display_data_buffer[0].mid_pose = 0;
      display_data_buffer[0].bot_pose = 7;
      display_data_buffer[0].top_pose = 7;
      display_data_buffer[1].top_number = display_data.top_number;
      display_data_buffer[1].bot_number = display_data.bot_number;
      display_data_buffer[1].mid_pose = display_data.mid_pose;
      display_data_buffer[1].bot_pose = display_data.bot_pose;
      display_data_buffer[1].top_pose = display_data.top_pose;
    }
    if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG3)) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG0);
    }
    if (Display_Alert(current_puff_time, 8, display_data_buffer)) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG0);
      main_mode = MAIN_MODE_0;
      current_puff_time = 0;
      awake_time = 0;
      SET_FLAG(display_flags1, FLAGS1_FLAG1);
    }
  } else if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG2)) {
    if (current_puff_time == 1) {
      display_data_buffer[0].top_number = 201;
      display_data_buffer[0].bot_number = display_data.bot_number;
      display_data_buffer[0].mid_pose = display_data.mid_pose;
      display_data_buffer[0].bot_pose = display_data.bot_pose;
      display_data_buffer[0].top_pose = 6;
      display_data_buffer[1].top_number = 0;
      display_data_buffer[1].bot_number = display_data.bot_number;
      display_data_buffer[1].mid_pose = display_data.mid_pose;
      display_data_buffer[1].bot_pose = display_data.bot_pose;
      display_data_buffer[1].top_pose = 0;
    }
    if (Display_Alert(current_puff_time, 5, display_data_buffer)) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG2);
      battery_percent = 0;
      main_mode = MAIN_MODE_0;
      current_puff_time = 0;
      awake_time = 0;
      SET_FLAG(display_flags1, FLAGS1_FLAG1);
    }
  } else if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG4)) {
    if (current_puff_time == 1) {
      display_data_buffer[0].top_number = display_data.top_number;
      display_data_buffer[0].bot_number = 201;
      display_data_buffer[0].mid_pose = display_data.mid_pose;
      display_data_buffer[0].bot_pose = 6;
      display_data_buffer[0].top_pose = display_data.top_pose;
      display_data_buffer[1].top_number = display_data.top_number;
      display_data_buffer[1].bot_number = display_data.bot_number;
      display_data_buffer[1].mid_pose = display_data.mid_pose;
      display_data_buffer[1].bot_pose = display_data.bot_pose;
      display_data_buffer[1].top_pose = display_data.top_pose;
    }
    if (Display_Alert(current_puff_time, 5, display_data_buffer) || HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_SET) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG4);
      main_mode = MAIN_MODE_0;
      current_puff_time = 0;
      awake_time = 0;
      SET_FLAG(display_flags1, FLAGS1_FLAG1);
    }
  } else if (IS_FLAG_SET(display_error_flags, FLAGS3_FLAG3)) {
    if (current_puff_time >= 1000) {
      actual_battery_percent = battery_millivolts_to_percent(vcc_millivolts);
      if (actual_battery_percent > battery_percent) {
        battery_percent = actual_battery_percent;
      }
    }
    if (current_puff_time == 1) {
      display_data.top_number = 188;
      display_data.bot_number = 188;
      display_data.mid_pose = 5;
      display_data.bot_pose = 5;
      display_data.top_pose = 5;
      LEDRing_mode = LEDRingMode_TEST_COLORS;
      LEDRing_anim_repeat_count = 1;
      LEDRing_should_reset_anim_frame = true;
    }
    if (LEDRing_mode == LEDRingMode_IDLE && current_puff_time >= 2000) {
      CLEAR_FLAG(display_error_flags, FLAGS3_FLAG3);
      main_mode = MAIN_MODE_0;
      current_puff_time = 0;
      awake_time = 9900;
      SET_FLAG(display_flags1, FLAGS1_FLAG1);
    }
  } else {
    main_mode = MAIN_MODE_0;
    current_puff_time = 0;
    awake_time = 0;
    SET_FLAG(display_flags1, FLAGS1_FLAG1);
  }
}

const uint32_t c60b0 = 7000;
const uint32_t c60b8 = 36000;
void main() {
  HAL_Init();
  APP_MainConfig();
  delay44x(20);
  total_puff_time = get_total_puff_time_FLASH();
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  main_mode = 2;
  battery_percent = 0;
  SET_FLAG(display_error_flags, FLAGS3_FLAG3);
  current_puff_time = 0;
  CLEAR_FLAG(display_error_flags, FLAGS3_FLAG0);
  CoilTB2_PWM_SetDutyCycle(0);
  CoilTB1_PWM_SetDutyCycle(0);
  for (counter0110 = 0; counter0110 < 10; ++counter0110) {
    delay44x(20);
    vcc_millivolts = get_VCC_millivolts();
    uint8_t new_actual_battery_percent = battery_millivolts_to_percent(vcc_millivolts);
    uint8_t old_actual_battery_percent = actual_battery_percent;
    actual_battery_percent = new_actual_battery_percent;
    if (old_actual_battery_percent > battery_percent) {
      battery_percent = actual_battery_percent;
    }
  }

  counter0110 = 0;
  counter0112 = 0;
  vcc_millivolts_buffer_frame = 0;
  PowerSwitch_state = 2;
  v0120 = c60b0;
  v0128 = c60b8;

  while(1) {
    if (main_mode == MAIN_MODE_0) {
      update_mode0();
    } else if (main_mode == MAIN_MODE_1) {
      update_mode1();
    } else if (main_mode == MAIN_MODE_2) {
      update_mode2();
    }

#ifdef ENABLE_RUNEYS_GAME
    update_runeys_game();
#endif

    if (IS_FLAG_SET(display_flags1, FLAGS1_REFRESH_DISPLAY)) {
      CLEAR_FLAG(display_flags1, FLAGS1_REFRESH_DISPLAY);
      display_write_next_section();
      vcc_millivolts_buffer[vcc_millivolts_buffer_frame++] = get_VCC_millivolts();
      if (vcc_millivolts_buffer_frame >= vcc_millivolts_buffer_size) {
        vcc_millivolts = 0;
        for (vcc_millivolts_buffer_frame = 0; vcc_millivolts_buffer_frame < vcc_millivolts_buffer_size; ++vcc_millivolts_buffer_frame) {
          vcc_millivolts += vcc_millivolts_buffer[vcc_millivolts_buffer_frame];
        }
        vcc_millivolts /= vcc_millivolts_buffer_size;
        vcc_millivolts_buffer_frame = 0;
      }
    }

    if (IS_FLAG_SET(display_flags1, FLAGS1_UPDATE_STATE)) {
      CLEAR_FLAG(display_flags1, FLAGS1_UPDATE_STATE);
      HAL_IWDG_Refresh(&IWDG_Handle);
      if (IS_FLAG_CLEARED(display_error_flags, FLAGS3_FLAG3)) {
        update_charging_state();
      }
      
      display_refresh_state();
      LEDRing_Update();
    }
  }

  int display_num = 0;
  int frame_count = 0;
  while(1) {
    frame_count++;
    if (frame_count >= 10) {
      frame_count = 0;
      display_refresh_state();
    }

    vcc_millivolts = get_VCC_millivolts();
    display_data.top_number = vcc_millivolts / 100;


    //set_top_display_number(display_num / 100 - 10);
    //set_bottom_display_number(200 - (display_num / 100));
    //display_state[DISPLAY_SECTION_CENTER] = display_center_frames[frame + 1];
    /*if (display_num >= 150) {
      display_num = 0;
      frame++;
    }
    if (frame > 10) {
      display_num = 0;
      frame = 0;
    }*/
    display_data.bot_pose = 5;
    display_data.top_pose = 5;
    display_data.mid_pose = 6;
    display_write_next_section();
    HAL_Delay(1);
    display_num++;
    if (display_num >= 21000) {
      display_num = 0;
    }
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

volatile const char *error_file;
volatile int error_line;
void APP_ErrorHandlerImpl(const char *file, int line) {
  while (1);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Export assert error source and line number
  */
void assert_failed(uint8_t *file, uint32_t line) {
  error_file = file;
  error_line = line;
  /* printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  while (1);
}
#endif /* USE_FULL_ASSERT */
