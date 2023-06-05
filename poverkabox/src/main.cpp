#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
#include <RobotClass_LiquidCrystal.h>
#include "Button2.h"
// #include <Wire.h>
// #include <U8g2lib.h>
void handler(Button2 &btn);
// U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// #define I2C_SDA PB7
// #define I2C_SCL PB6

const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);
unsigned int co = 0;
Button2 button_1, button_2;

void setup()
{
  // ConvertToCyrLCD converter = ConvertToCyrLCD(UTF8);
  pinMode(PC13, OUTPUT);
  // pinMode(PA0, INPUT_PULLDOWN);
  // pinMode(PA1, INPUT_PULLDOWN);
  lcd.begin(16, 2);
  // for (int thisLetter = 120; thisLetter <= 255; thisLetter++)
  // {
  //   lcd.clear();
  //   // loop over the columns:
  //   for (int thisRow = 0; thisRow < 2; thisRow++)
  //   {
  //     // loop over the rows:
  //     for (int thisCol = 0; thisCol < 16; thisCol++)
  //     {
  //       // set the cursor position:
  //       lcd.setCursor(thisCol, thisRow);
  //       // print the letter:
  //       lcd.write(thisLetter);
  //       delay(10);
  //     }
  //   }
  // }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("AquaTech"));
  // lcd.print(converter.convert (F("ÐÐºÐ²Ð°Ñ‚ÐµÑ…Ð½Ð¸ÐºÐ°")));
  // lcd.print( "ÀÁÂÃÄÅ¨ÆÇÈÊËÌÍÎÏ" );
  // lcd.setCursor(0,1);
  // lcd.print( "àáâãäå¸æçèêëìíîï" );
  // delay(3000);

  // lcd.setCursor(0, 0);
  // lcd.print( "ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß" );
  // lcd.setCursor(0,1);
  // lcd.print( "ðñòóôõö÷øùúûüýþÿ" );
  // delay(3000);
  button_1.begin(PA0);
  button_1.setDebounceTime(200);
  button_1.setChangedHandler(handler);
}

void loop()
{
  //   delay(500);
  //   digitalWrite(PC13, !digitalRead(PC13));
  button_1.loop();
}

void handler(Button2 &btn)
{
  // switch (btn.getType())
  // {
  // case single_click:
  //   break;
  // case double_click:
  //   Serial.print("double ");
  //   break;
  // }
  // Serial.print("click ");
  // Serial.print("on button #");
  // Serial.print((btn == button_1) ? "1" : "2");
  // Serial.println();
  co++;
  lcd.setCursor(6, 1);
  lcd.print("PA0 ch");
  lcd.setCursor(0, 1);
  lcd.printf("%d5", co);
}
