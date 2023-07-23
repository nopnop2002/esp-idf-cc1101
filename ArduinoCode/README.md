# Example of Arduino environment   
I use [this](https://github.com/veonik/arduino-cc1101) library.   

# Wirering

|CC1101||ATmega328|ATmega2560|
|:-:|:-:|:-:|:-:|
|CSN|--|D10(*2)|D10(*2)|
|MISO|--|D12|D12|
|MOSI|--|D11(*2)|D11(*2)|
|SCK|--|D13(*2)|D13(*2)|
|GD0|--|D2(*2)|D19(*2)|
|GD2|--|N/C|N/C|
|VCC|--|3.3V(*1)|3.3V|
|GND|--|GND|GND|

(*1)   
UNO's 3.3V output can only supply 50mA.   
In addition, the output current capacity of UNO-compatible devices is smaller than that of official products.   
__So this module may not work normally when supplied from the on-board 3v3.__   

(*2)    
CC1101 is not 5V tolerant.   
You need level shift from 5V to 3.3V.   
I used [this](https://www.ti.com/lit/ds/symlink/txs0108e.pdf?ts=1647593549503) for a level shift.   

