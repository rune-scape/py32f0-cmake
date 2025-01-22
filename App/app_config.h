#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void APP_MainConfig();
void APP_RCC_ClearResetFlags();
void APP_IWDG_Config();
void APP_SystemClockConfig();
void APP_GPIO_Config();
void APP_TIM1_Config();
void APP_TIM3_Config();
void APP_TIM16_Config();
void APP_ADC_Config();
void APP_LPTIM_Config();

void Charger_Enable();
void Charger_Disable();

void SWD_Enable();
void SWD_Disable();

void APP_SystemClockConfig_Sleep();
void APP_GPIO_Config_Sleep();

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONFIG_H */