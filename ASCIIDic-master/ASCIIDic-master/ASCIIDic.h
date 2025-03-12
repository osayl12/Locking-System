/*
||
|| @author Alexis Paques
|| @mail alexis.paques@gmail.com
|| @description Thoses are numbers encoded for the 3641BS
|| @todo Add a #define to XOR numbers for 3641AS (1 = ON & 0 = OFF)
|| @version 1.0
||
*/

#ifndef ASCII_7_SEG_DIC_H
#define ASCII_7_SEG_DIC_H


#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

  const byte dic_letters[26] = {
      // 1 = OFF SEGMENT, 0 = ON SEGMENT
      // ASCII : A 65, 0x41 => 90 Z
      // ASCII : a 97, 0x61 => 122 z
      0b10001000, // A
      0b10000011,
      0b10100111, // c
      0b10100001,
      0b10000110, // e
      0b10001110,
      0b10010000, // g
      0b10001001,
      0b11111001, // i
      0b11100001,
      0b10001010, //
      0b11000111, //L
      0b11001000,
      0b10101011, // N
      0b10100011,
      0b10001100,
      0b10011000, // Q
      0b11001100, //
      0x92, // S
      0b10000111,
      0b11100011, //
      0b11000001, // V
      0b11000001, 
      0b10001001,
      0b10001101, // Y BUG ? 
      0xA4 // z
    };

    const byte dic_numbers[12] = {
      0xC0,
      0xF9,
      0xA4,
      0xB0,
      0x99,
      0x92,
      0x82,
      0xF8,
      0X80,
      0X90,
      0x7F,      // . is 10
      0b10111111 // - is 11
    };


#endif