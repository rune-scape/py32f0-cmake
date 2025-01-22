#ifndef __PY32F0XX_IT_H
#define __PY32F0XX_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

void NMI_Handler();
void HardFault_Handler();
void SVC_Handler();
void PendSV_Handler();
void SysTick_Handler();
void RTC_IRQHandler();
void EXTI0_1_IRQHandler();
void EXTI2_3_IRQHandler();
void EXTI4_15_IRQHandler();
void DMA1_Channel1_IRQHandler();
void ADC_COMP_IRQHandler();
void LPTIM1_IRQHandler();
void TIM16_IRQHandler();

#ifdef __cplusplus
}
#endif

#endif /* __PY32F0XX_IT_H */
