#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
#include <RobotClass_LiquidCrystal.h>
#include <ArduinoJson.h>
// #include <STM32RTC.h>
#include <stm32f1_rtc.h>

void myISRn();
void myISR();
void myISRc();
void rtc_SecondsCB(void *data);
void rtc_Alarm(void *data);

const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);

volatile unsigned int co = 0;
const unsigned int debonuse_MS = 250;
volatile uint32_t ms_1, ms_2, ms_3;

DynamicJsonDocument doc(1024);

// #if defined(RTC_SSR_SS)
// static uint32_t atime = 678;
// #else
// static uint32_t atime = 1000;
// #endif
// volatile int alarmMatch_counter = 0;
// #ifdef RTC_ALARM_B
// volatile int alarmBMatch_counter = 0;
// #endif
// STM32RTC &rtc = STM32RTC::getInstance();
extern "C" void SystemClock_Config(void)
{
  // clock init code here...
  // https://community.platformio.org/t/changing-clock-settings-in-arduino-for-stm32/23091
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

void setup()
{
  SystemClock_Config();
  pinMode(PC13, OUTPUT);
  pinMode(PA0, INPUT_PULLDOWN);
  pinMode(PA1, INPUT_PULLDOWN);
  pinMode(PA2, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(PA0), myISR, RISING); // trigger when button pressed, but not when released.
  attachInterrupt(digitalPinToInterrupt(PA1), myISRn, RISING);
  attachInterrupt(digitalPinToInterrupt(PA2), myISRc, RISING);

  // rtc.setClockSource(STM32RTC::HSE_CLOCK);
  // rtc.begin(STM32RTC::HOUR_24);
  // rtc.attachSecondsInterrupt(rtc_SecondsCB);
  // rtc.attachInterrupt(rtc_Alarm, &atime);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("AquaTech"));

  ms_1 = millis();
  ms_2 = millis();
  ms_3 = millis();
}

void loop()
{
  lcd.setCursor(0, 1);
  lcd.printf("%5d", co);
  delay(100);
  digitalWrite(PC13, !digitalRead(PC13));
}
void myISR()
{
  if ((millis() - ms_1) > debonuse_MS)
  {
    co++;
    ms_1 = millis();
  }
}
void myISRn()
{
  if ((millis() - ms_2) > debonuse_MS)
  {
    co--;
    ms_2 = millis();
  }
}
void myISRc()
{
  if ((millis() - ms_3) > debonuse_MS)
  {
    co++;
    ms_3 = millis();
  }
}
/* callback function on each second interrupt */
void rtc_SecondsCB(void *data)
{
  UNUSED(data);
  digitalWrite(PC13, !digitalRead(PC13));
}
void rtc_Alarm(void *data)
{
  UNUSED(data);
}
