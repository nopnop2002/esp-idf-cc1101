# esp-idf-cc1101
CC1101 Low-Power Sub-1 GHz RF Transceiver driver for esp-idf.

I ported from [this](https://github.com/veonik/arduino-cc1101).

- 433MHz module(2.54mm pitch)
![CC1101-433MHz-2 54mm](https://user-images.githubusercontent.com/6020549/162900437-b4b74e11-ff01-4890-b85e-491ab36d1247.JPG)

- 433MHz module(1.27mm pitch)
![CC1101-433MHz-1 27mm](https://user-images.githubusercontent.com/6020549/162900515-eece8362-8ae0-4c63-9ca4-2f639026fd79.JPG)

- 868MHz module(2mm pitch)
![CC1101-868MHz](https://user-images.githubusercontent.com/6020549/162899066-cc1234a0-7e9d-4427-a779-601b107ddf87.JPG)


# Software requirements
esp-idf v4.4 or later.   
This is because this version supports ESP32-C3.   

# Installation

```Shell
git clone https://github.com/nopnop2002/esp-idf-cc1101
cd esp-idf-cc1101/basic
idf.py set-target {esp32/esp32s2/esp32s3/esp32c3}
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
![config-cc1101-3](https://user-images.githubusercontent.com/6020549/162887858-daf51cef-f165-48ce-b5bd-cdb33faf3824.jpg)
![config-cc1101-4](https://user-images.githubusercontent.com/6020549/162887865-28919f76-5401-4742-872a-7a16f6d269f7.jpg)

# Wirering

|CC1101||ESP32|ESP32-S2/S3|ESP32-C3|
|:-:|:-:|:-:|:-:|:-:|
|MISO|--|GPIO19|GPIO37|GPIO18|
|SCK|--|GPIO18|GPIO36|GPIO10|
|MOSI|--|GPIO23|GPIO35|GPIO19|
|CSN|--|GPIO5|GPIO34|GPIO9|
|GDO0|--|GPIO15|GPIO33|GPIO8|
|GDO2|--|N/C|N/C|N/C|
|GND|--|GND|GND|GND|
|VCC|--|3.3V|3.3V|3.3V|

__You can change it to any pin using menuconfig.__   

__GDO0 needs to be able to use interrupts.__   
__Some GPIOs cannot use interrupts.__   


# Comparison of cc1101 and nRF24L01
||nRF24L01|cc1101|
|:-:|:-:|:-:|
|Frequency|2.4GHz|315/433/868/915MHz|
|Maximum Payload|32Byte|64Byte|
|FiFo for reception|32Byte*3|64Byte*1|
|FiFo for transmission|32Byte*3|64Byte*1|
|CRC Length|8/16bits|16bits|
|Acknowledgement Payload|Yes|No|
|Modulation format|GFSK|2-FSK/4-FSK/GFSK/ASK/OOK/MSK|


- GFSK  : Gaussian shaped Frequency Shift Keying
- 2-FSK : Binary Frequency Shift Keying
- 4-FSK : Quaternary Frequency Shift Keying
- ASK   : Amplitude Shift Keying
- OSK   : On-Off Keying
- MSI   : Minimum Shift Keying

nRF24L01 can tell whether the transmission was successful or unsuccessful on the transmitting side.   

cc1101 cannot tell whether the transmission was successful or unsuccessful on the transmitting side.   
