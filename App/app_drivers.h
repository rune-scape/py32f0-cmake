#ifndef __APP_DRIVERS_H
#define __APP_DRIVERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void TIM3_OC_SetCompareCH1(uint32_t CompareValue);
void TIM3_OC_SetCompareCH2(uint32_t CompareValue);
void TIM3_OC_SetCompareCH3(uint32_t CompareValue);

void TIM1_OC_SetCompareCH3(uint32_t CompareValue);
void TIM1_OC_SetCompareCH4(uint32_t CompareValue);

// DutyCycle is 0-100
void CoilTB1_PWM_SetDutyCycle(int DutyCycle);
void CoilTB2_PWM_SetDutyCycle(int DutyCycle);
void AllCoilsOff();

// colors are 0-255
void LEDRing_SetColor(uint8_t green, uint8_t blue, uint8_t red);

void delay4();
void delay44();
void delay44x(uint32_t x);

#ifdef __cplusplus
}
#endif

#endif /* __APP_DRIVERS_H */