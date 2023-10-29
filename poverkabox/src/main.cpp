#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
// #include <RobotClass_LiquidCrystal.h>
#include <LiquidCrystal_1602_RUS.h>
#include <ArduinoJson.h>
#include <STM32RTC.h>
#include "utils.h"
#include <FlashStorage_STM32.h>

// TODO определения макро подстановок
#define costVolume 0.1 // число литров на один импульс
#define LED PC13
#define LEDGREEN PB9
#define LEDBLUE PB8
#define TESTPIN1 PA7
#define COUNTER PA2//PA5
#define COUNTER_E PA6
#define BTN1 PA0
#define BTN2 PA1
#define BTN3 PA10
#define Ainput PA3
#define Binput PA4

void myISRn();
void myISR();
void myISRc();
void myISRd();
void _myISRc();
void myISRce();
void rtc_SecondsCB(void *data);
void rtc_Alarm(void *data);
unsigned int dev_rtc = 2500;
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
volatile uint32_t ms_1, ms_2, ms_3, ms_4, ms_5, ms_6;
static unsigned char _10SecPulse;
static unsigned char _60SecPulse;
static unsigned int _100SecPulse;
volatile unsigned char Sec;
volatile unsigned char Min;
volatile unsigned char hr;
static unsigned int speedPulse, speedPulse_E;
volatile float volumeSpeed = 0;
// volatile float d0 = 0.0, d1 = 0.0, d2 = 0.0, d3 = 0.0;
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
volatile unsigned char display = 0;
struct Settings
{
	unsigned long NumRec;
	float d0 = 0.0, d1 = 0.0, d2 = 0.0, d3 = 0.0;
	float dv0 = 0.0, dv1 = 0.0, dv2 = 0.0, dv3 = 0.0;
};
bool dispSettings = false;
Settings setti = {};

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
	pinMode(BTN3, INPUT_PULLDOWN);
	pinMode(COUNTER, INPUT_PULLDOWN);
	pinMode(COUNTER_E, INPUT_PULLDOWN);

	// инициализация индикатора
	lcd.begin(20, 4);
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
	// TODO подключение прерываний к пинам входа
	attachInterrupt(digitalPinToInterrupt(BTN1), myISR, RISING); // trigger when button pressed, but not when released.
	attachInterrupt(digitalPinToInterrupt(BTN2), myISRn, RISING);
	attachInterrupt(digitalPinToInterrupt(BTN3), myISRd, RISING);
	attachInterrupt(digitalPinToInterrupt(COUNTER), myISRc, FALLING);
	attachInterrupt(digitalPinToInterrupt(COUNTER_E), myISRce, FALLING);

	rtc.setClockSource(STM32RTC::HSE_CLOCK);   // источник частоты контроллера реального времени
	rtc.begin(STM32RTC::HOUR_24);			   // цикл часов 24 часа
	rtc.attachSecondsInterrupt(rtc_SecondsCB); // назначаем прерывание от часов реального времени

	RTC_HandleTypeDef hrtc;
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = 62500 / dev_rtc; // прерывание секундное будет срабатывать dev_rtc раз в секунду
	hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE; // RTC_OUTPUTSOURCE_SECOND;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	// TODO предустановка веса импульса в мл
	// setti.d0 = 10.0 + 3.0 * analogRead(PA3) / 1020;
	// setti.d1 = 10.0 + 3.0 * analogRead(PA4) / 1020;
	// setti.d2 = 10.0 + 3.0 * analogRead(PA5) / 1020;
	// setti.d3 = 10.0 + 3.0 * analogRead(PA6) / 1020;
	// стартовая позиция подавление дребезга
	ms_1 = millis();
	ms_2 = millis();
	ms_3 = millis();
	ms_4 = millis();
	ms_5 = millis();
	ms_6 = millis();

	delay(8000);

	SerialCommand.print(F("\nStart FlashStoreAndRetrieve on "));
	SerialCommand.println(BOARD_NAME);
	SerialCommand.println(FLASH_STORAGE_STM32_VERSION);

	SerialCommand.print("EEPROM length: ");
	SerialCommand.println(EEPROM.length());

	int eeAddress = 0;
	EEPROM.get(eeAddress, setti);

	SerialCommand.printf("Размер установок(байт) %4d \nчисло циклов записи в FLASH %7d\n", sizeof(Settings), setti.NumRec);
	SerialCommand.printf("Диапазон 1 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d0, setti.dv0, (int)((setti.dv0 * 277.778) / setti.d0), dev_rtc / (int)((setti.dv0 * 277.778) / setti.d0));
	SerialCommand.printf("Диапазон 2 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d1, setti.dv1, (int)((setti.dv1 * 277.778) / setti.d1), dev_rtc / (int)((setti.dv1 * 277.778) / setti.d1));
	SerialCommand.printf("Диапазон 3 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d2, setti.dv2, (int)((setti.dv2 * 277.778) / setti.d2), dev_rtc / (int)((setti.dv2 * 277.778) / setti.d2));
	SerialCommand.printf("Диапазон 4 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d3, setti.dv3, (int)((setti.dv3 * 277.778) / setti.d3), dev_rtc / (int)((setti.dv3 * 277.778) / setti.d3));
	SerialCommand.printf("");
	if (setti.NumRec > 100000)
	{
		setti.NumRec = 0;
		EEPROM.put(eeAddress, setti);
		SerialCommand.println("Init Settings");
		EEPROM.get(eeAddress, setti);

		SerialCommand.println(setti.NumRec);
	}

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
	digitalWrite(LEDBLUE, !start);

	if (display == 0)
	{
		// lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Vиз");
		lcd.setCursor(3, 0);
		lcd.printf("=%09.6f", volumeCalculate / 1000000);
		lcd.setCursor(14, 0);
		lcd.print("м3");
		lcd.setCursor(0, 1);
		lcd.printf("Q1=%08.6f м3/ч", volumeSpeed * 3.6 / 1000.0);
		lcd.setCursor(12, 1);
		lcd.print("м3/ч");
	}
	else if (display == 1)
	{
		if (dispSettings)
		{
			setti.d0 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv0 = (3.0 * analogRead(Binput) / 1020);
		}

		lcd.setCursor(0, 0);
		lcd.print("Ди1 мл/имп=");
		lcd.printf("%05.2f", setti.d0);
		lcd.setCursor(0, 1);
		lcd.print("м3/ч=");
		lcd.printf("%09.6f", setti.dv0);
	}
	else if (display == 2)
	{
		if (dispSettings)
		{
			setti.d1 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv1 = (3.0 * analogRead(Binput) / 1020);
		}

		lcd.setCursor(0, 0);
		lcd.print("Ди2 мл/имп=");
		lcd.printf("%05.2f", setti.d1);
		lcd.setCursor(0, 1);
		lcd.print("м3/ч=");
		lcd.printf("%09.6f", setti.dv1);
	}
	else if (display == 3)
	{
		if (dispSettings)
		{
			setti.d2 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv2 = (3.0 * analogRead(Binput) / 1020);
		}

		lcd.setCursor(0, 0);
		lcd.print("Ди3 мл/имп=");
		lcd.printf("%05.2f", setti.d2);
		lcd.setCursor(0, 1);
		lcd.print("м3/ч=");
		lcd.printf("%09.6f", setti.dv2);
	}
	else if (display == 4)
	{
		if (dispSettings)
		{
			setti.d3 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv3 = (3.0 * analogRead(Binput) / 1020);
		}

		lcd.setCursor(0, 0);
		lcd.print("Ди4 мл/имп=");
		lcd.printf("%05.2f", setti.d3);
		lcd.setCursor(0, 1);
		lcd.print("м3/ч = ");
		lcd.printf("%09.6f", setti.dv3);
	}
	else
	{
		// lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Дисплей");
		lcd.printf("%02d", display);
		lcd.setCursor(0, 1);
		lcd.print(" пока пуст");
	}
	delay(1000 / 24);

	// digitalWrite(LED, !digitalRead(LED));
	if (SerialCommand.available())
	{
		String input;
		input = SerialCommand.readStringUntil('\n');
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
		if (display == 0)
		{
			if (!start)
			{
				volumeTicks = 0;
				// volumeAll = 0.0;
				volumeCalculate = 0.0;
				volumeCalculate = 0.0;
			}
			start = !start;
			DynamicJsonDocument command(1024);
			String input = "{\"start\":true,\"speedMidle\":5.1,\"volumeAll\":4.3,\"volumeMeasurment\":4.3}";
			deserializeJson(command, input);
			command["start"] = start;
			command["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
			command["volumeAll"] = volumeAll / 1000000;
			command["volumeMeasurment"] = volumeCalculate / 1000000;
			String output;
			serializeJson(command, output);

			SerialCommand.println(output);
		}
		else
		{
			dispSettings = !dispSettings;
		}
	}
}
// TODO обнуление или сохранение значений настроек диапазонов
void myISRn()
{
	if ((millis() - ms_2) > debonuse_MS)
	{
		if (display == 0)
		{
			speedPulse = 0;
			volumeSpeed = 0;
			volumeAll = 0;
			volumeCalculate = 0;
			display = 0;
		}
		else
		{
			setti.NumRec++;
			EEPROM.put(0, setti);
			EEPROM.get(0, setti);
			dispSettings = false;
			lcd.noBlink();
			dispSettings = false;
		}

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
void myISRd()
{
	if ((millis() - ms_5) > 250)
	{
		if (++display > 4)
			display = 0;
		dispSettings = false;
		lcd.noBlink();
		lcd.clear();
		ms_5 = millis();
	}
}
// TODO прерывание от счетчика по ноге COUNTER внешнего датчика
void myISRce()
{
	if ((millis() - ms_6) > 10)
	{
		if (start)
			speedPulse_E++;
		ms_6 = millis();
	}
}
// TODO прерывание от счетчика по ноге COUNTER
void myISRc()
{

	if ((millis() - ms_3) > 10) // 10 милисекунд нечуствительность к импульсам короче
	{
		if (speedPulse != 0)
		{
			volumeSpeed = (volumeSpeed + Cost(speedPulse) / (speedPulse / 2500.0)) / 2.0;
			// (Cost(speedPulse)/(speedPulse / 2500.0) - volumeSpeed) / 2.0;
			volumeAll += Cost(speedPulse);
			if (start)
				volumeCalculate += Cost(speedPulse);
			if (start)
				volumeCalculate += Cost(speedPulse);
		}
		speedPulse = 0;
		ms_3 = millis();
		// digitalWrite(TESTPIN1,0);
	}
}
/* TODO callback function on each 1/100 second interrupt */
void rtc_SecondsCB(void *data)
{
	UNUSED(data);

	Mills10++;

	if (++_100SecPulse >= 2500)
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
