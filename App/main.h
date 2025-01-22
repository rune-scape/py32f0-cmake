#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "py32f0xx_hal.h"

#define ENABLE_RUNEYS_GAME
#define LOW_POWER_PUFF_TIME_THRESHOLD_ms 3430
#define COIL_THRESHOLD_mA 3430
#define HIGH_VOLTAGE_COIL2_THRESHOLD_mA 3130

// the set and read flag macros must have weird things in them to begenerating those weird instructions
// ok maybe its just the compiler????? idk idc going for intent not pristeen recreation
//#define CLEAR_FLAG(REG, FLAG_INDEX)      ((REG) &= ~(1U << FLAG_INDEX))
//#define SET_FLAG(REG, FLAG_INDEX)        ((REG) = ((REG) & ~(1U << FLAG_INDEX)) + (1U << FLAG_INDEX))
//#define IS_FLAG_SET(REG, FLAG_INDEX)     (((int32_t)((((uint32_t)*((volatile uint8_t *)&REG)) << (31 - FLAG_INDEX)) >> 31)) != 0)
//#define IS_FLAG_CLEARED(REG, FLAG_INDEX) (((int32_t)((((uint32_t)*((volatile uint8_t *)&REG)) << (31 - FLAG_INDEX)) >> 31)) == 0)
#define CLEAR_FLAG(REG, MASK)      ((REG) &= ~(MASK))
#define SET_FLAG(REG, MASK)        ((REG) |= (MASK))
#define IS_FLAG_SET(REG, MASK)     (((REG) & (MASK)) != 0)
#define IS_FLAG_CLEARED(REG, MASK) (((REG) & (MASK)) == 0)


#define PUFF_TIME_LIMIT_MS 65000
#define COIL_PWM_PERIOD 500
#define LED_RING_PWM_PERIOD 255

#define ADC_DMA_BUFFER_SIZE 16
#define ADC_DMA_CHANNEL_BUFFER_SIZE (ADC_DMA_BUFFER_SIZE / 4)

typedef enum {
  DISPLAY_SECTION_7SEG_TOP_LEFT,
  DISPLAY_SECTION_7SEG_TOP_RIGHT,
  DISPLAY_SECTION_7SEG_BOT_LEFT,
  DISPLAY_SECTION_7SEG_BOT_RIGHT,
  DISPLAY_SECTION_BOTTOM,
  DISPLAY_SECTION_TOP,
  DISPLAY_SECTION_CENTER,
  DISPLAY_SECTION_COUNT,
} DisplaySection;

typedef struct {
  uint8_t top_number;
  uint8_t bot_number;
  uint8_t mid_pose;
  uint8_t bot_pose;
  uint8_t top_pose;
} DisplayData_t;

typedef enum {
  MAIN_MODE_0,
  MAIN_MODE_1,
  MAIN_MODE_2,
} MainMode_t;

typedef enum {
  POWER_STATE_OFF,
  POWER_STATE_NORMAL,
  POWER_STATE_PULSE,
} PowerState_t;

typedef enum {
  LEDRingMode_IDLE,
  LEDRingMode_FLASHING,
  LEDRingMode_FADE_IN,
  LEDRingMode_FADE_OUT,
  LEDRingMode_CHARGING_BREATE,
  LEDRingMode_TEST_COLORS,
  LEDRingMode_INTRO,
  LEDRingMode_OFF = 255,
} LEDRingMode_t;

extern volatile uint32_t ADC_DMA_Buffer[ADC_DMA_BUFFER_SIZE];
#define ADC_DMA_GET_VINTREF_VALUE(i) (((volatile uint16_t *)ADC_DMA_Buffer)[i * 4 + 0])
#define ADC_DMA_GET_POWER_SWITCH_VALUE(i) (((volatile uint16_t *)ADC_DMA_Buffer)[i * 4 + 1])
#define ADC_DMA_GET_TB1_VALUE(i) (((volatile uint16_t *)ADC_DMA_Buffer)[i * 4 + 2])
#define ADC_DMA_GET_TB2_VALUE(i) (((volatile uint16_t *)ADC_DMA_Buffer)[i * 4 + 3])

extern IWDG_HandleTypeDef IWDG_Handle;
extern RTC_HandleTypeDef RTC_Handle;
extern LPTIM_HandleTypeDef LPTIM_Handle;
extern TIM_HandleTypeDef TIM1_Handle;
extern TIM_OC_InitTypeDef TIM1_OC;
extern TIM_HandleTypeDef TIM3_Handle;
extern TIM_OC_InitTypeDef TIM3_OC;
extern TIM_HandleTypeDef TIM16_Handle;
extern ADC_HandleTypeDef ADC_Handle;
extern ADC_ChannelConfTypeDef ADC_ChannelConf;
extern DMA_HandleTypeDef DMA_Handle;
extern uint32_t RCC_CSR_xRSTF_Flags;

#define FLAGS4_FLAG3 (1UL << 3U)
#define FLAGS4_FLAG4 (1UL << 4U)

#define FLAGS3_FLAG0 (1UL << 0U)
#define FLAGS3_FLAG1 (1UL << 1U)
#define FLAGS3_FLAG2 (1UL << 2U)
#define FLAGS3_FLAG3 (1UL << 3U)
#define FLAGS3_FLAG4 (1UL << 4U)
#define FLAGS3_FLAG5 (1UL << 5U)

#ifdef ENABLE_RUNEYS_GAME
#define FLAGS2_RUNEYS_GAME_FLAG (1UL << 3U)
#endif
#define FLAGS2_SHUTDOWN_ANIMATION_FLAG (1UL << 4U)
#define FLAGS2_FLAG6 (1UL << 6U)
#define FLAGS2_POWER_SWITCH_CHANGED_FLAG (1UL << 7U)

#define FLAGS1_UPDATE_MAIN_MODE (1UL << 0U)
#define FLAGS1_FLAG1 (1UL << 1U)
#define FLAGS1_FLAG2 (1UL << 2U)
#define FLAGS1_IS_CHARGER_ENABLED_FLAG (1UL << 4U)
#define FLAGS1_REFRESH_DISPLAY (1UL << 5U)
#define FLAGS1_UPDATE_STATE (1UL << 6U)
#define FLAGS1_FLAG7 (1UL << 7U)

extern volatile uint32_t display_flags1;
extern volatile uint32_t flags4;
extern uint32_t display_flags2;
extern volatile uint32_t display_error_flags;
extern MainMode_t main_mode;
extern volatile uint32_t awake_time;
extern DisplayData_t display_data;
extern volatile int current_puff_time;
extern volatile uint32_t current_puff_limiter_time;
extern uint32_t total_puff_time;
extern uint16_t battery_percent;
extern uint16_t actual_battery_percent;
extern int32_t vcc_millivolts;
extern volatile uint8_t display_frame_time;

#define APP_ErrorHandler() APP_ErrorHandlerImpl(__FILE__, __LINE__);
void APP_ErrorHandlerImpl(const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
