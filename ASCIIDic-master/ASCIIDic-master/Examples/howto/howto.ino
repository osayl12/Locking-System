/*
||
|| @author Alexis Paques
|| @mail alexis.paques@gmail.com
|| @description example to get values
|| @version 1.0
||
*/

#include "asciiDic.h"

void setup(){
  Serial.begin(9600);
}

void loop(){
  char data = '0';
  for(int i = 0; i<36; i++){
    int val = int('a' + i);
    data = (char)val;
    Serial.print(data);
    Serial.print(" : ");
    Serial.print(dic_letters[i], BIN);
    delay(100);
  }
}

