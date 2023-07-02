#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
// #include <RobotClass_LiquidCrystal.h>
#include <LiquidCrystal_1602_RUS.h>
#include <ArduinoJson.h>
#include <STM32RTC.h>
#include "utils.h"

#define costVolume 0.1 // число литров на один импульс

void myISRn();
void myISR();
void myISRc();
void _myISRc();
void rtc_SecondsCB(void *data);
void rtc_Alarm(void *data);

const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);
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
volatile unsigned long volumeTicksC = 0;
volatile float volumeAll = 0;
volatile float volumeCalculate = 0;
volatile unsigned long Mills10 = 0;

void setup()
{
	SystemClock_Config();
	SerialCommand.begin(9600);
	while (!SerialCommand)
		;

	pinMode(PC13, OUTPUT);

	pinMode(PB9, OUTPUT);
	pinMode(PB8, OUTPUT);
	pinMode(PA7, OUTPUT);

	pinMode(PA0, INPUT_PULLDOWN);
	pinMode(PA1, INPUT_PULLDOWN);
	pinMode(PA2, INPUT_PULLDOWN);

	lcd.begin(16, 2);
	lcd.command(192);
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Акватехника");

	// while (!digitalRead(PA0))
	// {
	//   delay(100);
	// }
	// delay(50);
	// while (digitalRead(PA0))
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
	//   while (!digitalRead(PA0))
	//   {
	//     delay(100);
	//   }
	//   delay(250);
	//   while (digitalRead(PA0))
	//   {
	//     delay(100);
	//   }
	// }

	attachInterrupt(digitalPinToInterrupt(PA0), myISR, RISING); // trigger when button pressed, but not when released.
	attachInterrupt(digitalPinToInterrupt(PA1), myISRn, RISING);
	attachInterrupt(digitalPinToInterrupt(PA2), myISRc, FALLING);
	// attachInterrupt(digitalPinToInterrupt(PA2), _myISRc, RISIN);

	rtc.setClockSource(STM32RTC::HSE_CLOCK);

	rtc.begin(STM32RTC::HOUR_24);
	// rtc.begin();
	rtc.attachSecondsInterrupt(rtc_SecondsCB);
	// rtc.attachInterrupt(rtc_Alarm, &atime);

	RTC_HandleTypeDef hrtc;
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = 62500 / 100;	  // прерывание секундное будет срабатывать 100 раз в секунду
	hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE; // RTC_OUTPUTSOURCE_SECOND;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	ms_1 = millis();
	ms_2 = millis();
	ms_3 = millis();
	ms_4 = millis();
	digitalWrite(PC13, 1);
}
// {"start":1,"speedMidle":0,"volume":0}
void loop()
{
	uint8_t sec = 0;
	uint8_t mn = 0;
	uint8_t hrs = 0;
	uint32_t subs = 0;

	rtc.getTime(&hrs, &mn, &sec, &subs);
	lcd.setCursor(0, 0);
	lcd.print("Vэт");
	lcd.setCursor(3, 0);
	lcd.printf("=%09.6f", volumeAll / 1000);
	lcd.setCursor(14, 0);
	lcd.print("м3");

	digitalWrite(PB8, !start);

	lcd.setCursor(0, 1);
	lcd.printf("Q1=%08.6f м3/ч", volumeSpeed * 3.6);
	lcd.setCursor(12, 1);
	lcd.print("м3/ч");
	delay(1000 / 24);

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
		digitalWrite(PA7, 1);
		ms_4 = millis();
	}
}
void myISRc()
{

	if ((millis() - ms_3) > 20)
	{
		// digitalWrite(PA7,digitalRead(PA2));
		if (speedPulse != 0)
		{
			volumeSpeed += (costVolume / (speedPulse / 100.0) - volumeSpeed) / 2.0;
			volumeAll += costVolume;
		}
		speedPulse = 0;
		// volumeAll += costVolume;
		ms_3 = millis();
		// digitalWrite(PA7,0);
	}
}
/* callback function on each second interrupt */
void rtc_SecondsCB(void *data)
{
	UNUSED(data);

	Mills10++;

	// if (++_10SecPulse >= 10)
	//   _10SecPulse = 0;
	// if (++_60SecPulse >= 60)
	//   _60SecPulse = 0;

	if (++_100SecPulse >= 100)
	{
		_100SecPulse = 0;
		digitalWrite(PB9, !digitalRead(PB9));
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

	// if (_100SecPulse == 0)
	// {
	//   // volumeSpeed += (speedPulse - volumeSpeed) * 0.4;
	//   // speedPulse = 0;
	// }
	if (start)
		volumeTicks++;
}
void rtc_Alarm(void *data)
{
	UNUSED(data);
}
