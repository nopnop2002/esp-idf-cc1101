# esp-idf-cc1101
CC1101 Low-Power Sub-1 GHz RF Transceiver driver for esp-idf.

I based it on [this](https://github.com/veonik/arduino-cc1101).

- EBYTE E07-400M10S 433MHz(1.27mm pitch)   
 Support frequencies from 410MHz to 450MHz.   
 ![Image](https://github.com/user-attachments/assets/e49d90fb-6bf8-4976-9b6e-f15d75e20c6d)

- 433MHz module(2.54mm pitch)   
 The supported frequency bands are unknown.   
 ![CC1101-433MHz-2 54mm-1](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/b5b0f350-5e1f-4487-b00c-2e1b2d7d696d)

- EBYTE E07-M1101D-SMA 433MHz module(2.54mm pitch)   
 The official Product Descriptio is [here](https://www.cdebyte.com/products/E07-M1101D-SMA/4#Downloads).   
 Support frequencies from 387MHz to 464MHz.   
 ![CC1101-433MHz-2 54mm-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/0ece5072-8645-49df-a494-c6470e75cafc)
 ![CC1101-433MHz-2 54mm-2-pinout](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/0ccba0a5-dcc4-4687-93d2-5ce02940a179)

- 433MHz module(1.27mm pitch)   
 The supported frequency bands are unknown.   
 ![CC1101-433MHz-1 27mm-1](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/414c1b08-43c3-4998-ac81-f0d3006d2699)
 ![CC1101-433MHz-1 27mm-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/9b555215-848d-4b85-a8ce-9e6481c9d953)

- 868MHz module(2mm pitch)   
 868MHz transceivers typically support frequencies from 855MHz to 925MHz.   
 ![CC1101-868MHz](https://user-images.githubusercontent.com/6020549/162899066-cc1234a0-7e9d-4427-a779-601b107ddf87.JPG)
 ![CC1101-868MHz-pinout](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/f153ecad-5e96-4e27-a61f-95b3bc1b8214)

# Changes from the original   
- The 918MHz setting has been abolished.   
- Added 315MHz setting.   
- TX power setting was changed.   

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   
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
![config-cc1101-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/ea7a5389-bd92-414d-9456-9b4b0491592d)

# Frequency selection   
![config-cc1101-3](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/ab635915-350f-49d0-81af-ea001b22abd1)

# Modulation speed selection   
![config-cc1101-4](https://user-images.githubusercontent.com/6020549/162887865-28919f76-5401-4742-872a-7a16f6d269f7.jpg)

# Output power Selection   
![config-cc1101-6](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/b3e776cd-426a-4726-9c57-d6f8ee0bc587)

Default power level.   
||315MHz|433MHz|868MHz|915MHz|
|:-:|:-:|:-:|:-:|:-:|
|Output Power|8.5dBm|7.8dBm|8.5dBm|7.2dBm|
|Current Consumption|24.4mA|25.2mA|29.5mA|27.4mA|

Minimum power level is -30dBm.   
||315MHz|433MHz|868MHz|915MHz|
|:-:|:-:|:-:|:-:|:-:|
|Current Consumption|10.9mA|11.9mA|12.1mA|12.0mA|

Power lebel is 0dBm.   
||315MHz|433MHz|868MHz|915MHz|
|:-:|:-:|:-:|:-:|:-:|
|Current Consumption|15.0mA|15.9mA|16.9mA|16.7mA|

Maximum power level is 10dBm.   
||315MHz|433MHz|868MHz|915MHz|
|:-:|:-:|:-:|:-:|:-:|
|Current Consumption|26.9mA|29.1mA|32.4mA|31.8mA|

# The frequency used by the transceiver   
The frequency used by the transceiver is determined by the XOSC (crystal oscillator) implemented in the hardware.   
The XOSC (Crystal Oscillator) is a small silver component on the board.   
Many transceivers are equipped with a 26MHz XOSC.   
![280566405-0ece5072-8645-49df-a494-c6470e75cafc](https://github.com/user-attachments/assets/dd3f9079-517b-4bae-a8d8-72e857acd028)

The frequency used by the transceiver is determined by the XOSC frequency and registers value.   
```
Fcarrier = { Fxosc / 2**16 } * FREQ[23:0]

Fcarrier is the frequency used by the transceiver
Fxosc is the XOSC frequency
FREQ[23:0] is the register value
```

This library uses the following register values:
- 315MHz   
 Fcarrier = { 26MHz / 2**16 } * 0x0C1D89 = 314.999664MHz

- 433MHz   
 Fcarrier = { 26MHz / 2**16 } * 0x10A762 = 432.999817MHz

- 868MHz   
 Fcarrier = { 26MHz / 2**16 } * 0x216276 = 867.999939MHz

- 915MHz   
 Fcarrier = { 26MHz / 2**16 } * 0x23313B = 914.999969MHz

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

# EBYTE SMD PCB   
EBYTE's SPI module is a 1.27mm pitch SMD module.   
So I designed a conversion board that converts from 1.27mm pitch to 2.54mm pitch.   
You can order the PCB yourself using the publicly available Gerber files.   

![Image](https://github.com/user-attachments/assets/44e33542-1f0a-4356-8b22-fc4ed0933bd3)

You can download gerbers file from [here](https://www.pcbway.com/project/shareproject/EBYTE_SPI_LoRa_module_pitch_converter_6c8c2ec2.html).    
You can repeat my order from [here](https://www.pcbway.com/project/shareproject/EBYTE_SPI_LoRa_module_pitch_converter_6c8c2ec2.html).    


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

