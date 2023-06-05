#include <Arduino.h>
// #include <LiquidCrystal.h>
// #include <CyrLCDconverter.h>
#include <RobotClass_LiquidCrystal.h>
// #include <Wire.h>
// #include <U8g2lib.h>

// U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// #define I2C_SDA PB7
// #define I2C_SCL PB6

const int rs = PA8, en = PA9, d4 = PB15, d5 = PB14, d6 = PB13, d7 = PB12;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
RobotClass_LiquidCrystal lcd(rs, en, d4, d5, d6, d7, CP_CP1251);

void setup()
{
  // ConvertToCyrLCD converter = ConvertToCyrLCD(UTF8);
  pinMode(PC13, OUTPUT);
  pinMode(PA0, INPUT_PULLDOWN);
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
  // lcd.print(converter.convert (F("Акватехника")));
  lcd.print( "�����Ũ���������" );
  lcd.setCursor(0,1);
  lcd.print( "���������������" );
  delay(3000);

  lcd.setCursor(0, 0);
  lcd.print( "����������������" );
  lcd.setCursor(0,1);
  lcd.print( "����������������" );
  delay(3000);
}

void loop()
{
  delay(500);
  digitalWrite(PC13, !digitalRead(PC13));
}
