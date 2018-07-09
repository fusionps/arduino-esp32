/*
   LCDST7032SPI.cpp  - Library for SPI-type LCDs

   Created by Leonardo Urrego @trecetp, 2016

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "LCDST7032SPI.h"


LCDST7032SPI::LCDST7032SPI(uint8_t reset, uint8_t register_select, uint8_t cs){
    _rst = reset;
    _rs = register_select;
    _cs = cs;

    pinMode(_rst, OUTPUT);
    pinMode(_rs, OUTPUT);
    pinMode(_cs, OUTPUT);

    digitalWrite(_rst, HIGH);
    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, HIGH);

}

void LCDST7032SPI::init(){
//    Serial.println("startInit");
    digitalWrite(_rst, LOW);     // RESET
    delay(2);
    digitalWrite(_rst, HIGH);
    delay(20);
    command(_LCD_FUNCTIONSET_ | _LCD_8BITMODE_);  // LCD WAKEUP 0x30
    delay(2);
    command(_LCD_FUNCTIONSET_ | _LCD_8BITMODE_);  // LCD WAKEUP 0x30
    command(_LCD_FUNCTIONSET_ | _LCD_8BITMODE_);  // LCD WAKEUP 0x30
    command(_LCD_FUNCTIONSET_ | _LCD_8BITMODE_ | _LCD_2LINE_ | _LCD_INSTRUCTIONTABLE_1_);     // 0x39
    command(_LCD_BIASINTOSCSET_ | _LCD_BIAS_1_5_ | _LCD_OSC_183HZ_);                          // 0x14
    command(_LCD_POWICONCONTRASTHSET_ | _LCD_BOOST_ON_ | _LCD_CONTRAST_C5_ON_);               // 0x56
    command(_LCD_FOLLOWERCONTROLSET_ | _LCD_FOLLOWER_ON_ | _LCD_RAB_2_50_);                   // 0x6D

    command(_LCD_CONTRASTSET_);                                                               // 0x70
    command(_LCD_DISPLAYCONTROLSET_ | _LCD_DISPLAYON_ | _LCD_CURSOROFF_ | _LCD_BLINKOFF_);    // 0x0C
    command(_LCD_ENTRYMODESET_ | _LCD_ENTRYLEFT_ | _LCD_ENTRYSHIFTDECREMENT_);                // 0x06
    command(_LCD_CLEARDISPLAY_);   // clear

    delay(10);

    _displayentrymode = _LCD_ENTRYMODESET_ | _LCD_ENTRYLEFT_ | _LCD_ENTRYSHIFTDECREMENT_;
    _displayfunction = _LCD_FUNCTIONSET_ | _LCD_8BITMODE_ | _LCD_2LINE_ | _LCD_INSTRUCTIONTABLE_1_;
    _displaycontrol = _LCD_DISPLAYCONTROLSET_ | _LCD_DISPLAYON_ | _LCD_CURSOROFF_ | _LCD_BLINKOFF_;
//     Serial.println("finishInit");
}

void LCDST7032SPI::command(uint8_t data){
//    Serial.println("startCommand");
    SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    digitalWrite(_rs, LOW);
    delayMicroseconds(10);
    SPI.transfer(data);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    delayMicroseconds(50);
//    Serial.println("finishCommand");
}

size_t LCDST7032SPI::write(uint8_t data){
    SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    digitalWrite(_rs, HIGH);
    delayMicroseconds(10);
    SPI.transfer(data);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    delayMicroseconds(50);
    return 1; // assume sucess
}

void LCDST7032SPI::home(){
    command(_LCD_RETURNHOME_);
    delay(2);
}

void LCDST7032SPI::clear(){
    command(_LCD_CLEARDISPLAY_);
    delay(2);
}

void LCDST7032SPI::setCursor(uint8_t col, uint8_t row){
    row = (row == 0)? 0 : 0x40;
    command(_LCD_SETDDRAM_ADDR_ | (row + col));
}



// Turns the underline cursor on/off
void LCDST7032SPI::noCursor() {
  _displaycontrol &= ~_LCD_CURSORON_;
  command(_displaycontrol);
}
void LCDST7032SPI::cursor() {
  _displaycontrol |= _LCD_CURSORON_;
  command(_displaycontrol);
}

// Turn on and off the blinking cursor
void LCDST7032SPI::noBlink() {
  _displaycontrol &= ~_LCD_BLINKON_;
  command(_displaycontrol);
}
void LCDST7032SPI::blink() {
  _displaycontrol |= _LCD_BLINKON_;
  command(_displaycontrol);
}

// Turn the display on/off (quickly)
void LCDST7032SPI::noDisplay() {
  _displaycontrol &= ~_LCD_DISPLAYON_;
  command(_displaycontrol);
}
void LCDST7032SPI::display() {
  _displaycontrol |= _LCD_DISPLAYON_;
  command(_displaycontrol);
}

void printByte(byte b){
  for(int i = 0; i < 8; i++){
    Serial.print(b >> 7-i & 0x1 ? '1' : '0');
  }
  Serial.print(" ");
}

void LCDST7032SPI::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  // _LCD_SETDDRAM_ADDR_
  // _LCD_SETCGRAMADDR_ 
  Serial.println();
  command(_LCD_SETCGRAMADDR_ | (location << 3));
  for (int i=0; i<8; i++) {
    
    write(charmap[i]);
    printByte(_LCD_SETCGRAMADDR_ | (location << 3) | i);
    printByte(charmap[i]);
    Serial.println();
  }
}

