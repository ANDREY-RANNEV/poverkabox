/*
 * LiquidCrystal library override for cyrillic LCD
 * Written by Oleg Evsegneev for RobotClass (2022).  
 * e-mail: oleg.evsegneev@gmail.com
*/
 
#include <LiquidCrystal.h>

#define CP_UTF8 0
#define CP_CP1251 1

class RobotClass_LiquidCrystal: public LiquidCrystal {
    public:
        RobotClass_LiquidCrystal(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cp);
        size_t print( const String &s );
    private:
        uint8_t codepage;
        String utf8rus(String source);
        String convertCyr( const String &s );
        uint8_t getCharCyr( uint8_t ch );
};