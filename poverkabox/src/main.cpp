#include <Arduino.h>
// #include <Arduino_JSON.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
#include <RobotClass_LiquidCrystal.h>
// #include <LiquidCrystal_1602_RUS.h>
#include <ArduinoJson.h>
#include <STM32RTC.h>
#include "utils.h"
#include <FlashStorage_STM32.h>

// TODO определения макро подстановок
#define costVolume 0.1 // число литров на один импульс
#define LED PC13	   /*not ft*/
#define LEDGREEN PB0   /*ft*/
// #define LEDGREEN PB9   /*ft*///чемодан
// #define LEDBLUE PB8	   /*ft*///чемодан
#define LEDBLUE PB1	  /*ft*/
#define TESTPIN1 PA7  /*not ft*/
#define COUNTER PA5	  /*not ft*/
#define COUNTER_E PA6 /*not ft*/
#define BTN1 PA12	  /*not ft*/
// #define BTN1 PA0	   /*not ft*/ // чемодан
#define BTN2 PA11 /*not ft*/
// #define BTN2 PA1	   /*not ft*/ //чемодан
#define BTN3 PA10 /*ft*/
// #define Ainput PA3	   /*not ft*/ //чемодан
#define Ainput PB10 /*not ft*/
#define Binput PA4	/*not ft*/

void myISRn();
void myISR();
void myISRc();
void myISRd();
void _myISRc();
void myISRce();
void rtc_SecondsCB(void *data);
void rtc_Alarm(void *data);
unsigned int dev_rtc = 2500;
// const int rs = B98 /*ft*/, en = PA9 /*ft*/, d4 = PB15 /*ft*/, d5 = PB14 /*ft*/, d6 = PB13 /*ft*/, d7 = PB12 /*ft*/; ..чемодан
const int rs = PB9 /*ft*/, en = PB8 /*ft*/, d4 = PA3 /*not ft*/, d5 = PA2 /*not ft*/, d6 = PA1 /*not ft*/, d7 = PA0 /*not ft*/;
// LiquidCrystal lcd(PB9, PB8, PA3, PA2, PA1, PA0);
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_UTF8);
// RobotClass_LiquidCrystal lcd(PB9, PB8, PA3, PA2, PA1, PA0, CP_UTF8);
// LiquidCrystal_1602_RUS lcd(rs, en, d4, d5, d6, d7);
DynamicJsonDocument doc(1024);
//                           RX   TX
HardwareSerial SerialCommand(PB7, PB6);

volatile bool start = false;
volatile unsigned int co = 0;
const unsigned int debonuse_MS = 500;
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
	// float d4 = 0.0, d5 = 0.0, d6 = 0.0, d7 = 0.0;
	float dv0 = 0.0, dv1 = 0.0, dv2 = 0.0, dv3 = 0.0;
	// float dv4 = 0.0, dv5 = 0.0, dv6 = 0.0, dv7 = 0.0;
	long numRanges = 4;
};
DynamicJsonDocument settiJ(2024);

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
	lcd.flush();
	// lcd.command(192);
	// lcd.clear();
	delay(1000);
	lcd.setCursor(0, 0);
	lcd.print("    Акватехника");
	// lcd.setCursor(0, 1);
	// lcd.print("verifier.akvatehnik.ru");
	lcd.setCursor(0, 2);
	lcd.print("Инициализация ......");
	lcd.setCursor(0, 3);
	lcd.print("---- Ожидайте  -----");
	display = 0;

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
	attachInterrupt(digitalPinToInterrupt(COUNTER_E), myISRce, FALLING); // TODO

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
	ms_1 = ms_2 = millis();
	ms_3 = millis();
	ms_4 = millis();
	ms_5 = millis();
	ms_6 = millis();

	delay(4000);
	lcd.clear();
	delay(400);

	SerialCommand.print(F("\nStart FlashStoreAndRetrieve on "));
	SerialCommand.println(BOARD_NAME);
	SerialCommand.println(FLASH_STORAGE_STM32_VERSION);

	SerialCommand.print("EEPROM length: ");
	SerialCommand.println(EEPROM.length());

	int eeAddress = 0;
	EEPROM.get(eeAddress, setti);
	SerialCommand.println("\n\nЗапуск программы измерителя\n\n");
	SerialCommand.printf("\n\nРазмер установок(байт) %4d \nчисло циклов записи в FLASH %7d\n\n", sizeof(Settings), setti.NumRec);
	SerialCommand.printf("Диапазон 1 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d0, setti.dv0, (int)((setti.dv0 * 277.778) / setti.d0), dev_rtc / (int)((setti.dv0 * 277.778) / setti.d0));
	SerialCommand.printf("Диапазон 2 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d1, setti.dv1, (int)((setti.dv1 * 277.778) / setti.d1), dev_rtc / (int)((setti.dv1 * 277.778) / setti.d1));
	SerialCommand.printf("Диапазон 3 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d2, setti.dv2, (int)((setti.dv2 * 277.778) / setti.d2), dev_rtc / (int)((setti.dv2 * 277.778) / setti.d2));
	SerialCommand.printf("Диапазон 4 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d3, setti.dv3, (int)((setti.dv3 * 277.778) / setti.d3), dev_rtc / (int)((setti.dv3 * 277.778) / setti.d3));
	// SerialCommand.printf("Диапазон 5 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d4, setti.dv4, (int)((setti.dv4 * 277.778) / setti.d4), dev_rtc / (int)((setti.dv4 * 277.778) / setti.d4));
	// SerialCommand.printf("Диапазон 6 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d5, setti.dv5, (int)((setti.dv5 * 277.778) / setti.d5), dev_rtc / (int)((setti.dv5 * 277.778) / setti.d5));
	// SerialCommand.printf("Диапазон 7 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d6, setti.dv6, (int)((setti.dv6 * 277.778) / setti.d6), dev_rtc / (int)((setti.dv6 * 277.778) / setti.d6));
	// SerialCommand.printf("Диапазон 8 Вес =%5.2f мл/имп Поток =%9.6f м3/ч частота =%4d Hz тиков =%4d\n", setti.d7, setti.dv7, (int)((setti.dv7 * 277.778) / setti.d7), dev_rtc / (int)((setti.dv7 * 277.778) / setti.d7));
	SerialCommand.printf("число диапазонов %4d \n", setti.numRanges);
	SerialCommand.printf("");

	if (setti.NumRec > 100000 || setti.numRanges < 2 || setti.numRanges > 8)
	{
		setti.NumRec = 1;
		setti.numRanges = 4;

		setti.d0 = 0.0;
		setti.d1 = 0.0;
		setti.d2 = 0.0;
		setti.d3 = 0.0;
		// setti.d4 = 0.0;
		// setti.d5 = 0.0;
		// setti.d6 = 0.0;
		// setti.d7 = 0.0;

		setti.dv0 = 0.0;
		setti.dv1 = 0.0;
		setti.dv2 = 0.0;
		setti.dv3 = 0.0;
		// setti.dv4 = 0.0;
		// setti.dv5 = 0.0;
		// setti.dv6 = 0.0;
		// setti.dv7 = 0.0;

		EEPROM.put(eeAddress, setti);
		SerialCommand.println("Init Settings");
		EEPROM.get(eeAddress, setti);

		SerialCommand.println(setti.NumRec);

		// String output;
		// serializeJson(setti, output);

		// SerialCommand.println(output);
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
		if (start)
		{
			lcd.setCursor(0, 0);
			lcd.print("---  Измерение   ---");
		}
		else
		{
			lcd.setCursor(0, 0);
			lcd.print("---   Ожидание   ---");
		}
		lcd.setCursor(0, 1);
		lcd.print("Vиз");
		lcd.setCursor(3, 1);
		lcd.printf(" = %09.6f", volumeCalculate / 1000000);
		lcd.setCursor(16, 1);
		lcd.print("м3");
		lcd.setCursor(0, 2);
		lcd.printf("Q1  = %09.6f", volumeSpeed * 3.6 / 1000.0);
		lcd.setCursor(16, 2);
		lcd.print("м3/ч");
		// lcd.setCursor(0, 3);
		// lcd.print("S2 пуск/остан S4 Реж");
	}
	else if (display == 1)
	{
		if (dispSettings)
		{
			setti.d0 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv0 = (3.0 * analogRead(Binput) / 1020);
			lcd.setCursor(0, 0);
			lcd.print("--- Правка Диап1 ---");
			lcd.setCursor(0, 3);
			lcd.print("S3 Сохр Ди1 S4 Режим");
		}
		else
		{
			lcd.setCursor(0, 0);
			lcd.print("--- Просмотр Ди1 ---");
			lcd.setCursor(0, 3);
			lcd.print("S2 Прав Ди1 S4 Режим");
		}

		lcd.setCursor(0, 1);
		lcd.print("Ди1 мл/имп=");
		lcd.printf("%05.2f", setti.d0);
		lcd.setCursor(0, 2);
		lcd.print("м3/ч=");
		lcd.printf("%09.6f", setti.dv0);
	}
	else if (display == 2)
	{
		if (dispSettings)
		{
			setti.d1 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv1 = (3.0 * analogRead(Binput) / 1020);
			lcd.setCursor(0, 0);
			lcd.print("--- Правка Диа2 ---");
			lcd.setCursor(0, 3);
			lcd.print("S3 Сохр Ди2 S4 Режим");
		}
		else
		{
			lcd.setCursor(0, 0);
			lcd.print("--- Просмотр Ди2 ---");
			lcd.setCursor(0, 3);
			lcd.print("S2 Прав Ди2 S4 Режим");
		}

		lcd.setCursor(0, 1);
		lcd.print("Ди2 мл/имп=");
		lcd.printf("%05.2f", setti.d1);
		lcd.setCursor(0, 2);
		lcd.print("м3/ч=");
		lcd.printf("%09.6f", setti.dv1);
	}
	else if (display == 3)
	{
		if (dispSettings)
		{
			setti.d2 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv2 = (3.0 * analogRead(Binput) / 1020);
			lcd.setCursor(0, 0);
			lcd.print("--- Правка Диа3 ---");
			lcd.setCursor(0, 3);
			lcd.print("S3 Сохр Ди3 S4 Режим");
		}
		else
		{
			lcd.setCursor(0, 0);
			lcd.print("--- Просмотр Ди3 ---");
			lcd.setCursor(0, 3);
			lcd.print("S2 Прав Ди3 S4 Режим");
		}

		lcd.setCursor(0, 1);
		lcd.print("Ди3 мл/имп=");
		lcd.printf("%05.2f", setti.d2);
		lcd.setCursor(0, 2);
		lcd.print("м3/ч=");
		lcd.printf("%09.6f", setti.dv2);
	}
	else if (display == 4)
	{
		if (dispSettings)
		{
			setti.d3 = (10.0 + 3.0 * analogRead(Ainput) / 1020);
			setti.dv3 = (3.0 * analogRead(Binput) / 1020);
			lcd.setCursor(0, 0);
			lcd.print("--- Правка Диа4 ---");
			// lcd.setCursor(0, 3);
			// lcd.print("S3 Сохр Ди4 S4 Режим");
		}
		else
		{
			lcd.setCursor(0, 0);
			lcd.print("--- Просмотр Ди4 ---");
			// lcd.setCursor(0, 3);
			// lcd.print("S2 Прав Ди4 S4 Режим");
		}

		lcd.setCursor(0, 1);
		lcd.print("Ди4 мл/имп=");
		lcd.printf("%05.2f", setti.d3);
		lcd.setCursor(0, 2);
		lcd.print("м3/ч = ");
		lcd.printf("%09.6f", setti.dv3);
	}
	else
	{
		// lcd.clear();
		// lcd.setCursor(0, 0);
		// lcd.print("Дисплей");
		// lcd.printf("%02d", display);
		// lcd.setCursor(0, 1);
		// lcd.print(" пока пуст");
	}
	delay(1000 / 10);

	// digitalWrite(LED, !digitalRead(LED));
	if (SerialCommand.available()) // TODO ОЬРАБотка команд
	{
		String input;
		DynamicJsonDocument command(1024);
		DynamicJsonDocument answer(1024);
		String msg;
		int diapazon=0;
		input = SerialCommand.readStringUntil('\n');
		// SerialCommand.print(input);
		DeserializationError error = deserializeJson(doc, input);

		if (error)
		{
			SerialCommand.print(F("deserializeJson() failed: "));
			SerialCommand.println(error.f_str());
			SerialCommand.println(input);
		}
		else
		{

			// start = doc["start"].as<bool>();

			int commandRecive = doc["command"].as<int>(); // TODO получаем пришедщую комманду если нет команды то 0
			lcd.setCursor(0, 3);
			lcd.print("Command = ");
			switch (commandRecive)
			{
			case 0: // TODO команда получить текушие значения измерений команда 0 или без команды
				lcd.printf("%d", commandRecive);
				answer["Command"] = 999; // команда 999 ответ с значениями измерений
				answer["start"] = start;
				answer["speedPulse_E"] = speedPulse_E;
				answer["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
				answer["volumeAll"] = volumeAll / 1000000;
				answer["volumeMeasurment"] = volumeCalculate / 1000000;
				break;
			case 1:						 // получить значения калибровки, параметр команды  номер диапазона
				answer["Command"] = 999; // команда 999 ответ с значениями измерений успешно 666 неуспешно и сообение в ответе
				diapazon = command["diapazon"].as<int>();
				answer["start"] = start;
				switch (diapazon)
				{
				case 0:
					answer["dv"] = setti.dv0;
					answer["d"] = setti.d0;
					break;
				case 1:
					answer["dv"] = setti.dv1;
					answer["d"] = setti.d1;
					break;
				case 2:
					answer["dv"] = setti.dv2;
					answer["d"] = setti.d2;
					break;
				case 3:
					answer["dv"] = setti.dv3;
					answer["d"] = setti.d3;
					break;
				default:
					answer["dv"] = setti.dv0;
					answer["d"] = setti.d0;
					break;
				}

				break;
			case 2: //TODO команда занесения значений калибровки

				answer["Command"] = 999; // команда 999 ответ с значениями измерений успешно 666 неуспешно и сообение в ответе
				diapazon = command["diapazon"].as<int>();
				answer["start"] = start;
				switch (diapazon)
				{
				case 0:
					setti.dv0 = answer["dv"].as<float>();
					setti.d0 = answer["d"].as<float>();
					break;
				case 1:
					setti.dv1 = answer["dv"].as<float>();
					setti.d1 = answer["d"].as<float>();
					break;
				case 2:
					setti.dv2 = answer["dv"].as<float>();
					setti.d2 = answer["d"].as<float>();
					break;
				case 3:
					setti.dv3 = answer["dv"].as<float>();
					setti.d3 = answer["d"].as<float>();
					break;
				default:
					setti.dv0 = answer["dv"].as<float>();
					setti.d0 = answer["d"].as<float>();
					break;
				}
			case 700: // TODO сохранить в пвмять установки
				setti.NumRec++;
				EEPROM.put(0, setti);
				EEPROM.get(0, setti);
				answer["NumRec"] = setti.NumRec;
				break;
			case 775: // TODO пуск
				volumeSpeed = 0.0;
				volumeAll = 0.0;
				volumeCalculate = 0.0;
				speedPulse_E = 0;
				start = true;
				answer["speedPulse_E"] = speedPulse_E;
				answer["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
				answer["volumeAll"] = volumeAll / 1000000;
				answer["volumeMeasurment"] = volumeCalculate / 1000000;
				break;
			case 776: // TODO останов
				start = false;
				answer["speedPulse_E"] = speedPulse_E;
				answer["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
				answer["volumeAll"] = volumeAll / 1000000;
				answer["volumeMeasurment"] = volumeCalculate / 1000000;
				break;
			case 777: // TODO отработка пуск/останов измерения триггерное переключение
				start = !start;
				answer["start"] = start;
				if (start) // если включаем то обнуляем значения для изерения  если будет отключение то посылаем ответ с измеренными значениями
				{
					volumeSpeed = 0.0;
					volumeAll = 0.0;
					volumeCalculate = 0.0;
					speedPulse_E = 0;
					answer["speedPulse_E"] = speedPulse_E;
					answer["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
					answer["volumeAll"] = volumeAll / 1000000;
					answer["volumeMeasurment"] = volumeCalculate / 1000000;
				}
				else
				{
					answer["speedPulse_E"] = speedPulse_E;
					answer["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
					answer["volumeAll"] = volumeAll / 1000000;
					answer["volumeMeasurment"] = volumeCalculate / 1000000;
				}

				break;
			case 999:
				answer["command"] = 999;

				msg = "Команды и параметры /r/n ";
				msg += "ответ 999 - успешно\n ";
				msg += "ответ 888 - ошибка читать в [message] /n/r ";
				msg += " 777 Пуск/останов переключение/r/n ";
				msg += " \n ";
				answer["message"] = msg;
				break;
			default:
				lcd.print("не опознано");
				break;
			}

			// DynamicJsonDocument command(1024);
			// String input = "{\"Command\":999,\"start\":true,\"speedMidle\":5.1,\"volumeAll\":4.3,\"volumeMeasurment\":4.3}";
			// deserializeJson(command, input);
			// command["Command"] = 999;
			// command["start"] = start;
			// command["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
			// command["volumeAll"] = volumeAll / 1000000;
			// command["volumeMeasurment"] = volumeCalculate / 1000000;

			String output;
			serializeJson(answer, output);
			// проверка длинны вывода в блютус
			// SerialCommand.print("Len str="); // 167
			// SerialCommand.println(output.length());

			SerialCommand.println(output); // завершаем вывод в bluetooth
			SerialCommand.flush();		   // посылаем команду если не ушла сама
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
			String input = "{\"Command\":999,\"start\":true,\"speedMidle\":5.1,\"volumeAll\":4.3,\"volumeMeasurment\":4.3}";
			deserializeJson(command, input);
			command["start"] = start;
			command["speedMidle"] = volumeSpeed * 3.6 / 1000.0;
			command["volumeAll"] = volumeAll / 1000000;
			command["volumeMeasurment"] = volumeCalculate / 1000000;
			command["c_numRanges"] = setti.numRanges;
			command["c_d0"] = setti.d0;
			command["c_dv0"] = setti.dv0;
			command["c_d1"] = setti.d1;
			command["c_dv1"] = setti.dv1;
			command["c_d2"] = setti.d2;
			command["c_dv2"] = setti.dv2;
			command["c_d3"] = setti.d3;
			command["c_dv3"] = setti.dv3;
			// command["c_d4"] = setti.d4;
			// command["c_dv4"] = setti.dv4;
			// command["c_d5"] = setti.d5;
			// command["c_dv5"] = setti.dv5;
			// command["c_d6"] = setti.d6;
			// command["c_dv6"] = setti.dv6;
			// command["c_d7"] = setti.d7;
			// command["c_dv7"] = setti.dv7;
			String output;
			serializeJson(command, output);
			SerialCommand.print("Len str="); // 167
			SerialCommand.println(output.length());
			for (int i = 0; i != output.length(); i++)
				SerialCommand.write(output[i]);
			// 	SerialCommand.print(output.substring(0, 25));
			// SerialCommand.print(output.substring(25, 50));
			// SerialCommand.print(output.substring(50, 75));
			// SerialCommand.print(output.substring(75, 100));
			// SerialCommand.print(output.substring(100, 125));

			SerialCommand.println("");
			SerialCommand.flush();
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
// нажатие кнопки S4 режимы работы переключение
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
			// if (start)
			// 	volumeCalculate += Cost(speedPulse);
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
