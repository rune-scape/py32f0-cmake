#include "main.h"
#include "app_config.h"

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_DMA_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_SYSCFG_DMA_Req(0);

  DMA_Handle.Instance = DMA1_Channel1;
  DMA_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
  DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
  DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
  DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  DMA_Handle.Init.Mode = DMA_CIRCULAR;
  DMA_Handle.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  HAL_DMA_DeInit(&DMA_Handle);
  HAL_DMA_Init(&DMA_Handle);

  hadc->DMA_Handle = &DMA_Handle;
  DMA_Handle.Parent = hadc;
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
}
