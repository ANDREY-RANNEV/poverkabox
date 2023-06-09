#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
// #include <RobotClass_LiquidCrystal.h>
#include <LiquidCrystal_1602_RUS.h>
#include <ArduinoJson.h>
#include <STM32RTC.h>
#include "utils.h"

// TODO определения макро подстановок
#define costVolume 0.1 // число литров на один импульс
#define LED PC13
#define LEDGREEN PB9
#define LEDBLUE PB8
#define TESTPIN1 PA7
#define COUNTER PA2
#define BTN1 PA0
#define BTN2 PA1

void myISRn();
void myISR();
void myISRc();
void _myISRc();
void rtc_SecondsCB(void *data);
void rtc_Alarm(void *data);


const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;

// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);
DynamicJsonDocument doc(1024);
//                           RX   TX
HardwareSerial SerialCommand(PB7, PB6);
LiquidCrystal_1602_RUS lcd(rs, en, d4, d5, d6, d7);
volatile bool start = false;
volatile unsigned int co = 0;
const unsigned int debonuse_MS = 250;
volatile uint32_t ms_1, ms_2, ms_3, ms_4;
static unsigned char _10SecPulse;
static unsigned char _60SecPulse;
static unsigned char _100SecPulse;
volatile unsigned char Sec;
volatile unsigned char Min;
volatile unsigned char hr;
static unsigned int speedPulse;
float volumeSpeed = 0;

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
volatile unsigned long volumeTicksC = 0;
volatile float volumeAll = 0;
volatile float volumeCalculate = 0;
volatile unsigned long Mills10 = 0;

void setup()
{
	SystemClock_Config();	   // определяем частоты работы микроконтроллера из STMCubeMX
	SerialCommand.begin(9600); // BlueTooth serial порт
	while (!SerialCommand)	   // ожидаем инициализации BlueTooth
		;
	// пины на выход
	pinMode(LED, OUTPUT);
	pinMode(LEDGREEN, OUTPUT);
	pinMode(LEDBLUE, OUTPUT);
	pinMode(TESTPIN1, OUTPUT);
	// пины на вход
	pinMode(BTN1, INPUT_PULLDOWN);
	pinMode(BTN2, INPUT_PULLDOWN);
	pinMode(COUNTER, INPUT_PULLDOWN);
	// инициализация индикатора
	lcd.begin(16, 2);
	// lcd.command(192);
	lcd.clear();
	lcd.setCursor(0, 0);

	lcd.print("Акватехника");
	// TODO проверка кодовой таблицы индикатора
	// while (!digitalRead(BTN1))
	// {
	//   delay(100);
	// }
	// delay(50);
	// while (digitalRead(BTN1))
	// {
	//   delay(100);
	// }

	// for (int iy = 0; iy < 8; iy++)
	// {
	//   for (int ix = 0; ix < 16; ix++)
	//   {
	//     lcd.setCursor(ix, 0);
	//     lcd.print(char(ix+32*iy));
	//     lcd.setCursor(ix, 1);
	//     lcd.print(char(ix +32*iy+ 16));
	//   }
	//   while (!digitalRead(BTN1))
	//   {
	//     delay(100);
	//   }
	//   delay(250);
	//   while (digitalRead(BTN1))
	//   {
	//     delay(100);
	//   }
	// }
	// подключение прерываний к пинам входа
	attachInterrupt(digitalPinToInterrupt(BTN1), myISR, RISING); // trigger when button pressed, but not when released.
	attachInterrupt(digitalPinToInterrupt(BTN2), myISRn, RISING);
	attachInterrupt(digitalPinToInterrupt(COUNTER), myISRc, FALLING);
	// attachInterrupt(digitalPinToInterrupt(COUNTER), _myISRc, RISIN);

	rtc.setClockSource(STM32RTC::HSE_CLOCK);   // источник частоты контроллера реального времени
	rtc.begin(STM32RTC::HOUR_24);			   // цикл часов 24 часа
	rtc.attachSecondsInterrupt(rtc_SecondsCB); // назначаем прерывание от часов реального времени

	RTC_HandleTypeDef hrtc;
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = 62500 / 2500;	  // прерывание секундное будет срабатывать 2500 раз в секунду
	hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE; // RTC_OUTPUTSOURCE_SECOND;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	// стартовая позиция подавление дребезга
	ms_1 = millis();
	ms_2 = millis();
	ms_3 = millis();
	ms_4 = millis();
	digitalWrite(LED, 1); // отключаем LED светодиод
}
// {"start":1,"speedMidle":0,"volume":0}
void loop()
{
	// uint8_t sec = 0;
	// uint8_t mn = 0;
	// uint8_t hrs = 0;
	// uint32_t subs = 0;

	// rtc.getTime(&hrs, &mn, &sec, &subs);
	lcd.setCursor(0, 0);
	lcd.print("Vэт");
	lcd.setCursor(3, 0);
	lcd.printf("=%09.6f", volumeAll / 1000);
	lcd.setCursor(14, 0);
	lcd.print("м3");

	digitalWrite(LEDBLUE, !start);

	lcd.setCursor(0, 1);
	lcd.printf("Q1=%08.6f м3/ч", volumeSpeed * 3.6);
	lcd.setCursor(12, 1);
	lcd.print("м3/ч");
	delay(1000 / 24);

	// digitalWrite(LED, !digitalRead(LED));
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
		{
			volumeTicks = 0;
			volumeAll = 0;
		}
		start = !start;
		DynamicJsonDocument command(1024);
		String input = "{\"start\":true,\"speedMidle\":13518.24120,\"volume\":48.756080}";
		deserializeJson(command, input);
		command["start"] = start;
		command["speedMidle"] = volumeSpeed;
		command["volume"] = volumeAll;
		String output;
		serializeJson(command, output);

		SerialCommand.println(output);
	}
}
void myISRn()
{
	if ((millis() - ms_2) > debonuse_MS)
	{
		speedPulse = 0;
		volumeSpeed = 0;
		volumeAll = 0;
		volumeCalculate = 0;
		ms_2 = millis();
	}
}
void _myISRc()
{
	if ((millis() - ms_4) > 10)
	{
		digitalWrite(TESTPIN1, 1);
		ms_4 = millis();
	}
}
// TODO прерывание от счетчика по ноге COUNTER
void myISRc()
{

	if ((millis() - ms_3) > 10) // 10 милисекунд нечуствительность к импульсам короче
	{
		// digitalWrite(TESTPIN1,digitalRead(COUNTER));
		if (speedPulse != 0)
		{
			volumeSpeed += (Cost(speedPulse) / (speedPulse / 2500.0) - volumeSpeed) / 2.0;
			volumeAll += Cost(speedPulse);
		}
		speedPulse = 0;
		// volumeAll += costVolume;
		ms_3 = millis();
		// digitalWrite(TESTPIN1,0);
	}
}
/* TODO callback function on each 1/100 second interrupt */
void rtc_SecondsCB(void *data)
{
	UNUSED(data);

	Mills10++;

	if (++_100SecPulse >= 100)
	{
		_100SecPulse = 0;
		digitalWrite(LEDGREEN, !digitalRead(LEDGREEN));
		if (++Sec >= 60)
		{
			Sec = 0;
			if (++Min >= 60)
			{
				Min = 0;
				if (++hr >= 24)
					hr = 0;
			}
		}
	}

	if (speedPulse >= 5000)
	{
		volumeSpeed = 0;
		speedPulse = 0;
	}
	else
		speedPulse++;

	if (start)
		volumeTicks++;
}
void rtc_Alarm(void *data)
{
	UNUSED(data);
}
