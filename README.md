# esp-idf-cc1101
CC1100 Low-Power Sub-1 GHz RF Transceiver driver for esp-idf.

I based on [this](https://github.com/loboris/ESP32_CC1101).

![CC1101](https://user-images.githubusercontent.com/6020549/158111751-76c97267-0fb9-4f43-946e-2640fa5f3733.JPG)


# Changes from the original   
- Replace custom SPI driver with standard SPI driver.   
- Added support for ESP32S2 and ESP32C3.   
- Rewrite the sample code.   

# Software requirements
esp-idf v4.4 or later.   
This is because this version supports ESP32-C3.   

# Installation

```Shell
git clone https://github.com/nopnop2002/esp-idf-cc1101
cd esp-idf-cc1101
idf.py set-target {esp32/esp32s2/esp32c3}
idf.py menuconfig
idf.py flash
```

__Note for ESP32C3__   
For some reason, there are development boards that cannot use GPIO06, GPIO08, GPIO09, GPIO19 for SPI clock pins.   
According to the ESP32C3 specifications, these pins can also be used as SPI clocks.   
I used a raw ESP-C3-13 to verify that these pins could be used as SPI clocks.   


# Configuration for Transceiver   
![config-cc1101-1](https://user-images.githubusercontent.com/6020549/158111642-146a2ed5-4463-4519-948b-db2f0391f8a4.jpg)
![config-cc1101-2](https://user-images.githubusercontent.com/6020549/158111647-d0276f26-5251-47b6-a97c-55af2e6ee7a9.jpg)


# Configuration for Application   
![config-app-1](https://user-images.githubusercontent.com/6020549/158111726-a0b57086-809d-4fc6-a9bf-225f8871a3df.jpg)
![config-app-2](https://user-images.githubusercontent.com/6020549/158111729-a9bbeee7-8242-4ee5-8ac9-fc33ab5b2fd7.jpg)


# Wirering

|CC1101||ESP32|ESP32-S2|ESP32-C3|
|:-:|:-:|:-:|:-:|:-:|
|CSN|--|GPIO5|GPIO34|GPIO9|
|MISO|--|GPIO19|GPIO33|GPIO18|
|MOSI|--|GPIO23|GPIO35|GPIO19|
|SCK|--|GPIO18|GPIO36|GPIO10|
|DGO2|--|GPIO15|GPIO37|GPIO8|
|DGO0|--|N/C|N/C|N/C|
|GND|--|GND|GND|GND|
|VCC|--|3.3V|3.3V|3.3V|

__You can change it to any pin using menuconfig.__   


# Screen Shot   
![screen-tx](https://user-images.githubusercontent.com/6020549/158112359-e1424239-38e0-4642-8259-bbd653639b3c.jpg)
![screen-rx](https://user-images.githubusercontent.com/6020549/158112361-3ae2c4e7-4ff0-4617-bca6-9ae51de67625.jpg)
