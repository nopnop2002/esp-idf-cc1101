# esp-idf-cc1101
CC1101 Low-Power Sub-1 GHz RF Transceiver driver for esp-idf.

I ported from [this](https://github.com/veonik/arduino-cc1101).

![CC1101](https://user-images.githubusercontent.com/6020549/158111751-76c97267-0fb9-4f43-946e-2640fa5f3733.JPG)

# Software requirements
esp-idf v4.4 or later.   
This is because this version supports ESP32-C3.   

# Installation

```Shell
git clone https://github.com/nopnop2002/esp-idf-cc1101
cd esp-idf-cc1101/basic
idf.py set-target {esp32/esp32s2/esp32c3}
idf.py menuconfig
idf.py flash
```

__Note for ESP32C3__   
For some reason, there are development boards that cannot use GPIO06, GPIO08, GPIO09, GPIO19 for SPI clock pins.   
According to the ESP32C3 specifications, these pins can also be used as SPI clocks.   
I used a raw ESP-C3-13 to verify that these pins could be used as SPI clocks.   


# Configuration for Transceiver   
![config-cc1101-1](https://user-images.githubusercontent.com/6020549/158705737-c52a7c83-89f9-483f-aa4a-fe4e3d433001.jpg)
![config-cc1101-2](https://user-images.githubusercontent.com/6020549/158705741-3fa58c21-c0f5-4a34-8ca4-2dad58103a45.jpg)


# Wirering

|CC1101||ESP32|ESP32-S2|ESP32-C3|
|:-:|:-:|:-:|:-:|:-:|
|CSN|--|GPIO5|GPIO34|GPIO9|
|MISO|--|GPIO19|GPIO33|GPIO18|
|MOSI|--|GPIO23|GPIO35|GPIO19|
|SCK|--|GPIO18|GPIO36|GPIO10|
|DGO0|--|GPIO15|GPIO37|GPIO8|
|DGO2|--|N/C|N/C|N/C|
|GND|--|GND|GND|GND|
|VCC|--|3.3V|3.3V|3.3V|

__You can change it to any pin using menuconfig.__   

