#include <Arduino.h>
#include <LiquidCrystal.h>
// #include <Wire.h>
// #include <U8g2lib.h>

// U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// #define I2C_SDA PB7
// #define I2C_SCL PB6

// const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
    pinMode(PC13, OUTPUT);
}

void loop()
{
    delay(500);
    digitalWrite(PC13,digitalRead(PC13));
}
