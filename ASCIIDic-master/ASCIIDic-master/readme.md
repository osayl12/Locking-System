Arduino ASCII Dictionary for 3641 4x 7 segments
=======================================

This library is done to simplify the nx7seg library and make you able to do your own implementation or change the display of the nx7seg without modifying the nx7seg library.


This library gives you the binary values to light on 7 segments with the convention : 

```
A = 00000001
B = 00000010
C = 00000100
D = 00001000
E = 00010000
F = 00100000
G = 01000000
DP = 10000000
```

Values are in global vars : 

```c++
byte dic_numbers[10];

byte dic_letters[36];

byte dic_comma[1];
```

The `dic_letters[0]` is the byte to write the 'A' on a 7 segment display.

The actual library is for the 3641BS, so to light on the A, you have to write 11111110.

***TODO*** : Reverse mode in defines or in a second lib ?


Credits
-------
Alexis Paques (@AlexisTM)