#include <Arduino.h>
extern volatile float d0, d1, d2, d4;
extern HardwareSerial SerialCommand;
struct Settings
{
  unsigned long NumRec;
  float d0 = 0.0, d1 = 0.0, d2 = 0.0, d3 = 0.0;
  float dv0 = 0.0, dv1 = 0.0, dv2 = 0.0, dv3 = 0.0;
};
extern Settings setti;
extern unsigned int dev_rtc;
extern "C" void SystemClock_Config(void)
{
  // clock init code here...
  // https://community.platformio.org/t/changing-clock-settings-in-arduino-for-stm32/23091
  // https://community.platformio.org/t/stm32-different-result-of-the-program-in-cubeide-and-platformio-stm32f103/19210/2
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);

  /** Enables the Clock Security System
   */
  HAL_RCC_EnableCSS();

  //*****************************************************************************
  // RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  // RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  // RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  // /** Initializes the RCC Oscillators according to the specified parameters
  //  * in the RCC_OscInitTypeDef structure.
  //  */
  // RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  // RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  // RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  // RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  // RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  // RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  // if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  // {
  //   Error_Handler();
  // }

  // /** Initializes the CPU, AHB and APB buses clocks
  //  */
  // RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  // RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  // RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  // {
  //   Error_Handler();
  // }
  // PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  // PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  // if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  // {
  //   Error_Handler();
  // }

  // /** Enables the Clock Security System
  //  */
  // HAL_RCC_EnableCSS();
}

float Cost(int val)
{
  int val_;
  // dv(i)*277.778 это значение мл/с =dd(i) из м3/ч
  // число импульсов в секунду dd(i)/d(i) =dd(i)s
  // dev_rtc число тиков в секунду
  // dev_rtc/dd(i)s число тиков на импульс =val(i)
  // f(val)=d(i-1) +((d(i)-d(i-1))/(val(i) - val(i-1)))*(val-val(i-1))

  float dd0 = (setti.dv0 * 277.778) / setti.d0;
  float dd1 = (setti.dv1 * 277.778) / setti.d1;
  float dd2 = (setti.dv2 * 277.778) / setti.d2;
  float dd3 = (setti.dv3 * 277.778) / setti.d3;
  unsigned int val0 = (unsigned int)(dev_rtc / dd0);
  unsigned int val1 = (unsigned int)(dev_rtc / dd1);
  unsigned int val2 = (unsigned int)(dev_rtc / dd2);
  unsigned int val3 = (unsigned int)(dev_rtc / dd3);

  SerialCommand.printf("val %d \n", val);

  if (val <= val0)
    return setti.d0;
  else if (val > val0 && val <= val1)
  {
    float ret = 0;
    ret = setti.d0 + ((setti.d1 - setti.d0) / (val1 - val0)) * (val - val0);
    SerialCommand.printf("dd = %8.5f \n", ret);
    return ret;
  }
  else if (val > val1 && val <= val2)
  {
    float ret = 0;
    ret = setti.d1 + ((setti.d2 - setti.d1) / (val2 - val1)) * (val - val1);
    SerialCommand.printf("dd = %8.5f \n", ret);
    return ret;
  }
  else if (val > val2 && val <= val3)
  {
    float ret = 0;
    ret = setti.d2 + ((setti.d3 - setti.d2) / (val3 - val2)) * (val - val2);
    SerialCommand.printf("dd = %8.5f \n", ret);
    return ret;
  }
  else
    return setti.d3;

  // if (val <= 35)
  //   return 11.5;
  // else if (val > 35 || val <= 92)
  //   return 11.3;
  // else if (val > 92 || val <= 363)
  //   return 11.3;
  // else if (val > 363 || val <= 1725)
  //   return 11.5;
  // else if (val > 1725 || val <= 3450)
  //   return 11.5;
  // else if (val > 3450 || val <= 6247)
  //   return 11.8;
  // else if (val > 6247 || val <= 9600)
  //   return 12.8;
  // else
  //   return 12.8;

  return setti.d0;
}