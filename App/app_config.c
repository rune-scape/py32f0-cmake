#include "app_config.h"
#include "app_drivers.h"
#include "main.h"


void APP_RCC_ClearResetFlags() {
  RCC_CSR_xRSTF_Flags = 0;
  RCC_CSR_xRSTF_Flags = RCC->CSR & 0xff000000;
  __HAL_RCC_CLEAR_RESET_FLAGS();
}

void APP_IWDG_Config() {
  IWDG_Handle.Instance       = IWDG;
  IWDG_Handle.Init.Prescaler = IWDG_PRESCALER_32;
  IWDG_Handle.Init.Reload    = 4000;
  if (HAL_IWDG_Init(&IWDG_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_SystemClockConfig() {
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;
  RCC_OscInitStruct.HSEState            = RCC_HSE_OFF;
  RCC_OscInitStruct.LSIState            = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState            = RCC_LSE_OFF;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    APP_ErrorHandler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    APP_ErrorHandler();
  }

  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM;
  RCC_PeriphCLKInitStruct.Comp1ClockSelection  = RCC_LPTIMCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_GPIO_Config() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_ADC_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct;

  // TB1 coil PWM config
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM1;
  GPIO_InitStruct.Pin       = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // TB2 coil PWM config
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // LED ring blue PWM config
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM3;
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // LED ring green PWM config
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // LED ring red PWM config
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Pressure input config
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin   = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  // CHRG input config
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin   = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  // TB2 coil voltage measure config
  GPIO_InitStruct.Pin  = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // TB1 coil voltage measure config
  GPIO_InitStruct.Pin  = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Switch input config
  GPIO_InitStruct.Pin  = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Display output config
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin   = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_9;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

void APP_TIM1_Config() {
  __HAL_RCC_TIM1_CLK_ENABLE();

  TIM1_Handle.Instance               = TIM1;
  TIM1_Handle.Init.Period            = COIL_PWM_PERIOD - 1;
  TIM1_Handle.Init.Prescaler         = 48 - 1;
  TIM1_Handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  TIM1_Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIM1_Handle.Init.RepetitionCounter = 0;
  TIM1_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&TIM1_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }

  TIM1_OC.OCMode       = TIM_OCMODE_PWM1;
  TIM1_OC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  TIM1_OC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  TIM1_OC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  TIM1_OC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  TIM1_OC.OCFastMode   = TIM_OCFAST_DISABLE;
  TIM1_OC.Pulse        = COIL_PWM_PERIOD;
  if (HAL_TIM_PWM_ConfigChannel(&TIM1_Handle, &TIM1_OC, TIM_CHANNEL_3) != HAL_OK) {
    APP_ErrorHandler();
  }

  TIM1_OC.OCMode       = TIM_OCMODE_PWM1;
  TIM1_OC.OCPolarity   = TIM_OCPOLARITY_LOW;
  TIM1_OC.OCNPolarity  = TIM_OCNPOLARITY_LOW;
  TIM1_OC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  TIM1_OC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  TIM1_OC.OCFastMode   = TIM_OCFAST_DISABLE;
  TIM1_OC.Pulse        = 0;
  if (HAL_TIM_PWM_ConfigChannel(&TIM1_Handle, &TIM1_OC, TIM_CHANNEL_4) != HAL_OK) {
    APP_ErrorHandler();
  }

  if (HAL_TIM_PWM_Start(&TIM1_Handle, TIM_CHANNEL_3) != HAL_OK) {
    APP_ErrorHandler();
  }

  if (HAL_TIM_PWM_Start(&TIM1_Handle, TIM_CHANNEL_4) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_TIM3_Config() {
  __HAL_RCC_TIM3_CLK_ENABLE();
  
  TIM3_Handle.Instance = TIM3;
  TIM3_Handle.Init.Period            = LED_RING_PWM_PERIOD - 1;
  TIM3_Handle.Init.Prescaler         = 48 - 1;
  TIM3_Handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  TIM3_Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIM3_Handle.Init.RepetitionCounter = 0;
  TIM3_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&TIM3_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }

  TIM3_OC.OCMode       = TIM_OCMODE_PWM1;
  TIM3_OC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  TIM3_OC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  TIM3_OC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  TIM3_OC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  TIM3_OC.OCFastMode   = TIM_OCFAST_DISABLE;
  TIM3_OC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&TIM3_Handle, &TIM3_OC, TIM_CHANNEL_1) != HAL_OK) {
    APP_ErrorHandler();
  }

  TIM3_OC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&TIM3_Handle, &TIM3_OC, TIM_CHANNEL_2) != HAL_OK) {
    APP_ErrorHandler();
  }

  TIM3_OC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&TIM3_Handle, &TIM3_OC, TIM_CHANNEL_3) != HAL_OK) {
    APP_ErrorHandler();
  }

  if (HAL_TIM_PWM_Start(&TIM3_Handle, TIM_CHANNEL_1) != HAL_OK) {
    APP_ErrorHandler();
  }

  if (HAL_TIM_PWM_Start(&TIM3_Handle, TIM_CHANNEL_2) != HAL_OK) {
    APP_ErrorHandler();
  }

  if (HAL_TIM_PWM_Start(&TIM3_Handle, TIM_CHANNEL_3) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_TIM16_Config() {
  __HAL_RCC_TIM16_CLK_ENABLE();

  TIM16_Handle.Instance               = TIM16;
  TIM16_Handle.Init.Period            = 1000;
  TIM16_Handle.Init.Prescaler         = 48 - 1;
  TIM16_Handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  TIM16_Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIM16_Handle.Init.RepetitionCounter = 0;
  TIM16_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&TIM16_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }

  HAL_NVIC_SetPriority(TIM16_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(TIM16_IRQn);
  if (HAL_TIM_Base_Start_IT(&TIM16_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_ADC_Config() {
  ADC_AnalogWDGConfTypeDef ADC_AnalogWDGConf = {0};
  
  do {
    __HAL_RCC_ADC_FORCE_RESET();
    __HAL_RCC_ADC_RELEASE_RESET();
    __HAL_RCC_ADC_CLK_ENABLE();

    ADC_Handle.Instance = ADC1;
    CLEAR_BIT(ADC_Handle.Instance->CCSR, ADC_CCSR_CALSMP);
    SET_BIT(ADC_Handle.Instance->CCSR, ADC_CALIBSAMPLETIME_8CYCLES);
    if (HAL_ADC_Calibration_Start(&ADC_Handle) != HAL_OK) {
      APP_ErrorHandler();
    }
  } while (READ_BIT(ADC_Handle.Instance->CCSR, ADC_CCSR_CALFAIL | ADC_CCSR_CALON));
  
  ADC_Handle.Instance                   = ADC1;
  ADC_Handle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
  ADC_Handle.Init.Resolution            = ADC_RESOLUTION_12B;
  ADC_Handle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  ADC_Handle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_BACKWARD;
  ADC_Handle.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
  ADC_Handle.Init.LowPowerAutoWait      = ENABLE;
  ADC_Handle.Init.ContinuousConvMode    = ENABLE;
  ADC_Handle.Init.DiscontinuousConvMode = DISABLE;
  ADC_Handle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  ADC_Handle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  ADC_Handle.Init.DMAContinuousRequests = ENABLE;
  ADC_Handle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
  ADC_Handle.Init.SamplingTimeCommon    = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_Init(&ADC_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }

  HAL_NVIC_SetPriority(ADC_COMP_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC_COMP_IRQn);

  ADC_AnalogWDGConf.WatchdogMode  = ADC_ANALOGWATCHDOG_SINGLE_REG;
  ADC_AnalogWDGConf.HighThreshold = 1755;
  ADC_AnalogWDGConf.LowThreshold  = 0;
  ADC_AnalogWDGConf.ITMode        = ENABLE;
  ADC_AnalogWDGConf.Channel       = ADC_CHANNEL_VREFINT;
  if (HAL_ADC_AnalogWDGConfig(&ADC_Handle, &ADC_AnalogWDGConf) != HAL_OK) {
    APP_ErrorHandler();
  }

  ADC_ChannelConf.Rank    = ADC_RANK_CHANNEL_NUMBER;
  ADC_ChannelConf.Channel = ADC_CHANNEL_3;
  if (HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_ChannelConf) != HAL_OK) {
    APP_ErrorHandler();
  }

  ADC_ChannelConf.Rank    = ADC_RANK_CHANNEL_NUMBER;
  ADC_ChannelConf.Channel = ADC_CHANNEL_6;
  if (HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_ChannelConf) != HAL_OK) {
    APP_ErrorHandler();
  }

  ADC_ChannelConf.Rank    = ADC_RANK_CHANNEL_NUMBER;
  ADC_ChannelConf.Channel = ADC_CHANNEL_7;
  if (HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_ChannelConf) != HAL_OK) {
    APP_ErrorHandler();
  }

  ADC_ChannelConf.Channel = ADC_CHANNEL_VREFINT;
  if (HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_ChannelConf) != HAL_OK) {
    APP_ErrorHandler();
  }

  if (HAL_ADC_Start_DMA(&ADC_Handle, (uint32_t *)ADC_DMA_Buffer, ADC_DMA_BUFFER_SIZE) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_LPTIM_Config() {
  __HAL_RCC_LPTIM_CLK_ENABLE();

  LPTIM_Handle.Instance        = LPTIM;
  LPTIM_Handle.Init.Prescaler  = LPTIM_PRESCALER_DIV128;
  LPTIM_Handle.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  if (HAL_LPTIM_Init(&LPTIM_Handle) != HAL_OK) {
    APP_ErrorHandler();
  }

  HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
}

void APP_MainConfig() {
  APP_RCC_ClearResetFlags();
  APP_SystemClockConfig();
  APP_IWDG_Config();
  APP_GPIO_Config();
  delay44x(200);
  //unknown_empty_fn_07ec();
  APP_TIM1_Config();
  APP_TIM3_Config();
  APP_TIM16_Config();
  APP_ADC_Config();
  //unknown_empty_fn_3be0();
  //unknown_empty_fn_5e3c();
  //unknown_empty_fn_4ffc();
  APP_LPTIM_Config();
}

void Charger_Enable() {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void Charger_Disable() {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, GPIO_PIN_SET);
}

void SWD_Enable() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SWJ;
  GPIO_InitStruct.Pin       = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SWJ;
  GPIO_InitStruct.Pin       = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SWD_Disable() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void APP_SystemClockConfig_Sleep() {
  RCC_ClkInitTypeDef RCC_ClkInit;
  RCC_OscInitTypeDef RCC_OscInit = {0};

  RCC_ClkInit.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInit.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInit.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInit.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInit, FLASH_LATENCY_1) != HAL_OK) {
    APP_ErrorHandler();
  }

  RCC_OscInit.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSI;
  RCC_OscInit.HSIState = RCC_HSI_ON;
  RCC_OscInit.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInit.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;
  RCC_OscInit.HSEState = RCC_HSE_OFF;
  RCC_OscInit.LSIState = RCC_LSI_ON;
  RCC_OscInit.LSEState = RCC_LSE_OFF;
  RCC_OscInit.PLL.PLLState = RCC_PLL_OFF;
  RCC_OscInit.PLL.PLLSource = RCC_PLLSOURCE_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK) {
    APP_ErrorHandler();
  }
}

void APP_GPIO_Config_Sleep() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_2, GPIO_PIN_RESET);

  Charger_Enable();

  // configures pin as output and turns off coil 1
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

  // configures pin as output and turns off coil 2
  GPIO_InitStruct.Pin       = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);

  // configures pin as output and turns off LEDRing blue
  GPIO_InitStruct.Pin       = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);

  // configures pin as output and turns off LEDRing green
  GPIO_InitStruct.Pin       = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  // configures pin as output and turns off LEDRing red
  GPIO_InitStruct.Pin       = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  WRITE_REG(EXTI->PR, EXTI_IMR_IM1);
  WRITE_REG(EXTI->PR, EXTI_IMR_IM3);
  WRITE_REG(EXTI->PR, EXTI_IMR_IM7);

  GPIO_InitStruct.Mode      = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Mode      = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Mode      = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin       = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);

  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 1, 0);

  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
}
