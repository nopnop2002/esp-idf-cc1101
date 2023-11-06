# esp-idf-cc1101
CC1101 Low-Power Sub-1 GHz RF Transceiver driver for esp-idf.

I ported from [this](https://github.com/veonik/arduino-cc1101).

- 433MHz module(2.54mm pitch)
![CC1101-433MHz-2 54mm](https://user-images.githubusercontent.com/6020549/162900437-b4b74e11-ff01-4890-b85e-491ab36d1247.JPG)
![CC1101-433MHz-2 54mm-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/0ece5072-8645-49df-a494-c6470e75cafc)

- 433MHz module(1.27mm pitch)
![CC1101-433MHz-1 27mm](https://user-images.githubusercontent.com/6020549/162900515-eece8362-8ae0-4c63-9ca4-2f639026fd79.JPG)

- 868MHz module(2mm pitch)
![CC1101-868MHz](https://user-images.githubusercontent.com/6020549/162899066-cc1234a0-7e9d-4427-a779-601b107ddf87.JPG)


# Software requirements
ESP-IDF V4.4/V5.x.   
ESP-IDF V5.0 is required when using ESP32-C2.   
ESP-IDF V5.1 is required when using ESP32-C6.   

# Installation

```Shell
git clone https://github.com/nopnop2002/esp-idf-cc1101
cd esp-idf-cc1101/basic
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash
```

__Note for ESP32C3__   
For some reason, there are development boards that cannot use GPIO06, GPIO08, GPIO09, GPIO19 for SPI clock pins.   
According to the ESP32C3 specifications, these pins can also be used as SPI clocks.   
I used a raw ESP-C3-13 to verify that these pins could be used as SPI clocks.   


# Configuration for Transceiver   
![config-cc1101-1](https://user-images.githubusercontent.com/6020549/158705737-c52a7c83-89f9-483f-aa4a-fe4e3d433001.jpg)
![config-cc1101-2](https://user-images.githubusercontent.com/6020549/167520047-c6924db8-55b3-441e-99c9-fda4d4e801a7.jpg)

# Frequency selection   
![config-cc1101-3](https://user-images.githubusercontent.com/6020549/162887858-daf51cef-f165-48ce-b5bd-cdb33faf3824.jpg)

# Modulation speed selection   
![config-cc1101-4](https://user-images.githubusercontent.com/6020549/162887865-28919f76-5401-4742-872a-7a16f6d269f7.jpg)

# Enable output high power
Default Power Setting is 0xC6.   
Enabling this results in 0xC0.   
|Setting|868 MHz|915 MHz|
|:-:|:-:|:-:|
|0xC6|29.4mA|28.7mA|
|0xC0|34.2mA|33.4mA|

![config-cc1101-6](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/72f130da-9317-49f7-b317-7f2d99dc73fb)

# SPI BUS selection   
![config-cc1101-5](https://user-images.githubusercontent.com/6020549/167520086-e74c1f25-5c5f-4349-a98f-5248ac9edf50.jpg)

The ESP32 series has three SPI BUSs.   
SPI1_HOST is used for communication with Flash memory.   
You can use SPI2_HOST and SPI3_HOST freely.   
When you use SDSPI(SD Card via SPI), SDSPI uses SPI2_HOST BUS.   
When using this module at the same time as SDSPI or other SPI device using SPI2_HOST, it needs to be changed to SPI3_HOST.   
When you don't use SDSPI, both SPI2_HOST and SPI3_HOST will work.   
Previously it was called HSPI_HOST / VSPI_HOST, but now it is called SPI2_HOST / SPI3_HOST.   

# Wirering

|CC1101||ESP32|ESP32-S2/S3|ESP32-C2/C3/C6|
|:-:|:-:|:-:|:-:|:-:|
|MISO|--|GPIO19|GPIO37|GPIO4|
|SCK|--|GPIO18|GPIO36|GPIO3|
|MOSI|--|GPIO23|GPIO35|GPIO2|
|CSN|--|GPIO5|GPIO34|GPIO1|
|GDO0|--|GPIO15|GPIO33|GPIO0|
|GDO2|--|N/C|N/C|N/C|
|GND|--|GND|GND|GND|
|VCC|--|3.3V|3.3V|3.3V|

__You can change it to any pin using menuconfig.__   

__GDO0 needs to be able to use interrupts.__   
__Some GPIOs cannot use interrupts.__   


# Communication with another library   
Basically, communication between applications with different register values is not guaranteed.   
These settings may differ.   
- Base frequency and channel spacing frequency
- Packet format configuration
- Manchester encoding configuration
- Wireless modulation method
- The data rate used when transmitting
- The data rate expected in receive

You need to use [this](https://github.com/veonik/arduino-cc1101) when communicating with the Arduino environment.   
Communication with other libraries is not guaranteed.   


# Comparison of cc2500 and cc1101
||cc2500|cc1101|
|:-:|:-:|:-:|
|Manufacturer|Texas Instrument|Texas Instrument|
|Frequency|2.4GHz|315/433/868/915MHz|
|Maximum Payload|64Byte|64Byte|
|FiFo for reception|64Byte*1|64Byte*1|
|FiFo for transmission|64Byte*1|64Byte*1|
|CRC Length|16bits|16bits|
|Acknowledgement Payload|No|No|
|Available Modulation format|2-FSK/GFSK/OOK/MSK|2-FSK/4-FSK/GFSK/ASK/OOK/MSK|


# Comparison of nRF905, cc1101 and si4432
||nRF905|cc1101|si4432|
|:-:|:-:|:-:|:-:|
|Manufacturer|Nordic Semiconductor|Texas Instrument|Silicon Labs|
|Frequency|433/868/915MHz|315/433/868/915MHz|315/433/868/915MHz|
|Maximum Payload|32Byte|64Byte|64Byte|
|FiFo for reception|32Byte*1|64Byte*1|64Byte*1|
|FiFo for transmission|32Byte*1|64Byte*1|64Byte*1|
|CRC Length|8/16bits|16bits|8/16bits|
|Acknowledgement Payload|No|No|No|
|Available Modulation format|GFSK|2-FSK/4-FSK/GFSK/ASK/OOK/MSK|FSK/GFSK/OOK|


- GFSK  : Gaussian shaped Frequency Shift Keying
- FSK   : Frequency Shift Keying
- 2-FSK : Binary Frequency Shift Keying
- 4-FSK : Quaternary Frequency Shift Keying
- ASK   : Amplitude Shift Keying
- OOK   : On-Off Keying
- MSI   : Minimum Shift Keying

