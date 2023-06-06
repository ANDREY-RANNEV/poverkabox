#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
#include <RobotClass_LiquidCrystal.h>
#include <ArduinoJson.h>

void myISRn();
void myISR();
void myISRc();

const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);

volatile unsigned int co = 0;
const unsigned int debonuse_MS = 250;
volatile uint32_t ms_1, ms_2, ms_3;

DynamicJsonDocument doc(1024);

void setup()
{

  pinMode(PC13, OUTPUT);
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