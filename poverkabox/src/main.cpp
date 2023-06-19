#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
#include <RobotClass_LiquidCrystal.h>
#include <ArduinoJson.h>
#include <STM32RTC.h>
#include "utils.h"

#define costVolume 1 / 10 // число литров на один импульс

void myISRn();
void myISR();
void myISRc();
void rtc_SecondsCB(void *data);
void rtc_Alarm(void *data);

const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);
volatile bool start = false;
volatile unsigned int co = 0;
const unsigned int debonuse_MS = 250;
volatile uint32_t ms_1, ms_2, ms_3;
static unsigned char _10SecPulse;
static unsigned char _60SecPulse;
static unsigned int speedPulse;
float volumeSpeed = 0;
DynamicJsonDocument doc(1024);
//                      RX    TX
HardwareSerial SerialCommand(PB7, PB6);
#if defined(RTC_SSR_SS)
static uint32_t atime = 678;
#else
static uint32_t atime = 1000;
#endif
volatile int alarmMatch_counter = 0;
#ifdef RTC_ALARM_B
volatile int alarmBMatch_counter = 0;
#endif
STM32RTC &rtc = STM32RTC::getInstance();
volatile unsigned int volumeTicks = 0;

void setup()
{
  SystemClock_Config();
  SerialCommand.begin(115200);
  while (!SerialCommand)
    ;

  pinMode(PC13, OUTPUT);

  pinMode(PB9, OUTPUT);
  pinMode(PB8, OUTPUT);

  pinMode(PA0, INPUT_PULLDOWN);
  pinMode(PA1, INPUT_PULLDOWN);
  pinMode(PA2, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(PA0), myISR, RISING); // trigger when button pressed, but not when released.
  attachInterrupt(digitalPinToInterrupt(PA1), myISRn, RISING);
  attachInterrupt(digitalPinToInterrupt(PA2), myISRc, RISING);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("AquaTech"));
  // lcd.setCursor(9, 0);
  // lcd.print(rtc.getClockSource() == LSI_CLOCK ? "LSI_CLOCK" : rtc.getClockSource() == LSE_CLOCK ? "LSE_CLOCK": "HSE_CLOCK");
  // SerialCommand.print("Привет!!!\n");

  rtc.setClockSource(STM32RTC::HSE_CLOCK);

  rtc.begin(STM32RTC::HOUR_24);
  // rtc.begin();
  rtc.attachSecondsInterrupt(rtc_SecondsCB);
  // rtc.attachInterrupt(rtc_Alarm, &atime);
  RTC_HandleTypeDef hrtc;
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = 62500/100;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;// RTC_OUTPUTSOURCE_SECOND;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  ms_1 = millis();
  ms_2 = millis();
  ms_3 = millis();
  digitalWrite(PC13, 1);
}

void loop()
{
  uint8_t sec = 0;
  uint8_t mn = 0;
  uint8_t hrs = 0;
  uint32_t subs = 0;
  rtc.getTime(&hrs, &mn, &sec, &subs);
  lcd.setCursor(9, 0);
  if (start)
  {
    lcd.print("start");
  }
  else
  {
    lcd.print("stop ");
  }
  digitalWrite(PB8, !start);
  lcd.setCursor(9, 1);
  lcd.printf("%2d", _10SecPulse);

  lcd.setCursor(12, 1);
  lcd.printf("%3d", speedPulse);
  // lcd.setCursor(8, 1);
  // if (digitalRead(PC13))
  // {
  //   // SerialCommand.printf("%02d:%02d:%02d\n\r", hrs, mn, sec);
  //   lcd.printf("%02d:%02d:%02d", hrs, mn, sec);
  // }
  // else
  // {
  //   // SerialCommand.printf("%02d %02d %02d\n\r", hrs, mn, sec);
  //   lcd.printf("%02d %02d %02d", hrs, mn, sec);
  // }
  lcd.setCursor(0, 1);
  lcd.printf("%05d", volumeTicks);
  delay(100);
  // digitalWrite(PC13, !digitalRead(PC13));
  if (SerialCommand.available())
  {
    String input;
    input = SerialCommand.readString();
    // SerialCommand.print(input);
    DeserializationError error = deserializeJson(doc, input);
    if (error)
    {
      SerialCommand.print(F("deserializeJson() failed: "));
      SerialCommand.println(error.f_str());
    }
    else
    {
      start = doc["start"].as<bool>();
    }
  }
}
void myISR()
{
  if ((millis() - ms_1) > debonuse_MS)
  {
    // co++;
    ms_1 = millis();
    if (!start)
      volumeTicks = 0;
    start = !start;
    DynamicJsonDocument command(1024);
    String input = "{\"start\":true,\"speedMidle\":13518.24120,\"volume\":48.756080}";
    deserializeJson(command, input);
    command["start"] = start;
    command["speedMidle"] = volumeSpeed;
    command["volume"] = 0.0;
    String output;
    serializeJson(command, output);

    SerialCommand.println(output);
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
    speedPulse++;

    ms_3 = millis();
  }
}
/* callback function on each second interrupt */
void rtc_SecondsCB(void *data)
{
  UNUSED(data);

  if (++_10SecPulse >= 10)
    _10SecPulse = 0;
  if (++_60SecPulse >= 60)
    _60SecPulse = 0;

  if (_10SecPulse == 0)
  {
    volumeSpeed += (speedPulse - volumeSpeed) * 0.3;
    speedPulse = 0;
  }
  if (start)
    volumeTicks++;
  digitalWrite(PB9, !digitalRead(PB9));
}
void rtc_Alarm(void *data)
{
  UNUSED(data);
}
