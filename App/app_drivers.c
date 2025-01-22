#include "app_drivers.h"
#include "app_config.h"
#include "py32f0xx_hal.h"

void TIM3_OC_SetCompareCH1(uint32_t CompareValue) {
  WRITE_REG(TIM3->CCR1, CompareValue);
}

void TIM3_OC_SetCompareCH2(uint32_t CompareValue) {
  WRITE_REG(TIM3->CCR2, CompareValue);
}

void TIM3_OC_SetCompareCH3(uint32_t CompareValue) {
  WRITE_REG(TIM3->CCR3, CompareValue);
}

void TIM1_OC_SetCompareCH3(uint32_t CompareValue) {
  WRITE_REG(TIM1->CCR3, CompareValue);
}

void TIM1_OC_SetCompareCH4(uint32_t CompareValue) {
  WRITE_REG(TIM1->CCR4, CompareValue);
}

void CoilTB1_PWM_SetDutyCycle(int DutyCycle) {
  if (DutyCycle > 100) {
    DutyCycle = 100;
  }
  TIM1_OC_SetCompareCH4(DutyCycle * (COIL_PWM_PERIOD / 100));
}

void CoilTB2_PWM_SetDutyCycle(int DutyCycle) {
  if (DutyCycle > 100) {
    DutyCycle = 100;
  }

  TIM1_OC_SetCompareCH3(COIL_PWM_PERIOD - DutyCycle * (COIL_PWM_PERIOD / 100));
}

void AllCoilsOff() {
  TIM1_OC_SetCompareCH3(500);
  TIM1_OC_SetCompareCH4(0);
  WRITE_REG(TIM1->CNT, COIL_PWM_PERIOD - 1);
}

void LEDRing_SetColor(uint8_t green, uint8_t blue, uint8_t red) {
  TIM3_OC_SetCompareCH1(blue);
  TIM3_OC_SetCompareCH2(red);
  TIM3_OC_SetCompareCH3(green);
}

void delay4() {
  for (register unsigned char i = 0; i < 4; ++i) {
    __NOP();
  }
}

void delay44() {
  for (register unsigned char i = 0; i < 44; ++i) {
    __NOP();
  }
}

void delay44x(uint32_t x) {
  for (uint32_t i = 0; i < x; ++i) {
    delay44();
  }
}
