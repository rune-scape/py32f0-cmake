#include "main.h"
#include "app_drivers.h"
#include "py32f0xx_it.h"

void NMI_Handler() {
}

void HardFault_Handler() {
  while (1) {}
}

void SVC_Handler() {
}

void PendSV_Handler() {
}

void SysTick_Handler() {
  HAL_IncTick();
}

void RTC_IRQHandler() {
  HAL_RTCEx_RTCIRQHandler(&RTC_Handle);
}

void EXTI0_1_IRQHandler() {
  CLEAR_FLAG(flags4, FLAGS4_FLAG4);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1); // GPIOF pin 1, CHRG input
}

void EXTI2_3_IRQHandler() {
  CLEAR_FLAG(flags4, FLAGS4_FLAG4);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3); // GPIOF pin 3, pressure input
}

void EXTI4_15_IRQHandler() {
  CLEAR_FLAG(flags4, FLAGS4_FLAG4);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7); // GPIOA pin 7, switch input
}

void DMA1_Channel1_IRQHandler() {
  HAL_DMA_IRQHandler(ADC_Handle.DMA_Handle);
}

void ADC_COMP_IRQHandler() {
  HAL_ADC_IRQHandler(&ADC_Handle);
}

void LPTIM1_IRQHandler() {
  HAL_LPTIM_IRQHandler(&LPTIM_Handle);
}

void TIM16_IRQHandler() {
  HAL_TIM_IRQHandler(&TIM16_Handle);
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc) {
  SET_FLAG(display_error_flags, FLAGS3_FLAG0);
  AllCoilsOff();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  SET_FLAG(display_flags1, FLAGS1_REFRESH_DISPLAY);
  SET_FLAG(display_flags1, FLAGS1_UPDATE_MAIN_MODE);
  display_frame_time++;
  awake_time++;
  if (display_frame_time > 9) {
    SET_FLAG(display_flags1, FLAGS1_UPDATE_STATE);
    display_frame_time = 0;
  }

  // if we are we not vaping
  if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_RESET) {
    current_puff_limiter_time = 0;
  } else {
    if (current_puff_limiter_time < PUFF_TIME_LIMIT_MS) {
      current_puff_limiter_time++;
      awake_time = 0;
    }
  }
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  uint8_t v2 = 0;
  uint8_t v3 = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    uint16_t v0 = ADC_DMA_GET_TB2_VALUE(i);
    uint16_t v1 = ADC_DMA_GET_TB1_VALUE(i);
    if (v0 < 3900 && v0 > 400) {
      v2++;
      if (v2 > 3) {
        SET_FLAG(display_error_flags, FLAGS3_FLAG0);
        AllCoilsOff();
      }
    }
    if (v1 < 3900 && v1 > 400) {
      v3++;
      if (v3 > 3) {
        SET_FLAG(display_error_flags, FLAGS3_FLAG0);
        AllCoilsOff();
      }
    }
  }
}