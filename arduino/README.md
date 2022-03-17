# Example of Arduino environment   
I use [this](https://github.com/veonik/arduino-cc1101).   

# Wirering

|CC1101||UNO|MEGA||
|:-:|:-:|:-:|:-:|:-:|
|CSN|--|D10|D10|*1|
|MISO|--|D12|D12||
|MOSI|--|D11|D11|*1|
|SCK|--|D13|D13|*1|
|GD0|--|D2|D19|*1|
|GD2|--|N/C|N/C||
|VCC|--|3.3V|3.3V||
|GND|--|GND|GND||

(*1)    
CC1101 is not 5V tolerant.   
You need level shift from 5V to 3.3V.   

