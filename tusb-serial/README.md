# USB Serial Device Example   
This is cc1101 and USB Serial Device gateway application.   

ESP-IDF supports USB Serial Device.   
USB Serial Device can communicate with the USB Serial Host.   
Windows or Linux terminal software acts as a USB Serial Host.   
This project use [this](https://docs.tinyusb.org/en/latest/) TinyUSB component as the USB stack.   
```
            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
            |USB Serial Host|--(USB)-->|     ESP32     |--(SPI)-->|     cc1101    |==(Radio)==>
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+

            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
==(Radio)==>|     cc1101    |--(SPI)-->|     ESP32     |--(USB)-->|USB Serial Host|
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+
```

# Hardware requirements
1. ESP32-S2/S3 Development board   
Because the ESP32-S2/S3 does support USB OTG.   

2. USB Connector   
I used this USB Mini femail:   
![usb-connector](https://user-images.githubusercontent.com/6020549/124848149-3714ba00-dfd7-11eb-8344-8b120790c5c5.JPG)

```
ESP32-S2/S3 BOARD          USB CONNECTOR
                           +--+
                           | || VCC
    [GPIO 19]    --------> | || D-
    [GPIO 20]    --------> | || D+
    [  GND  ]    --------> | || GND
                           +--+
```


# Configuration
![Image](https://github.com/user-attachments/assets/92b41987-8679-4af1-bfaf-e5bec05c76f8)

### USB to Radio
Receive from USB and send to Radio.   
ESP32 acts as USB Serial Device for reading.   
You can use this script as USB Serial Host for writing.   
```python3 ./write.py```

```
            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
            |USB Serial Host|--(USB)-->|     ESP32     |--(SPI)-->|     cc1101    |==(Radio)==>
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+
```

![Image](https://github.com/user-attachments/assets/f42de1bb-d8d0-46c0-89f9-bf7013eb0b5b)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\CC1101_receive   


### Radio to USB
Receive from Radio and send to  USB.   
ESP32 acts as USB Serial Device for writing.   
You can use this script as USB Serial Host for reading.   
```python3 ./read.py```

```
            +---------------+          +---------------+          +---------------+
            |               |          |               |          |               |
==(Radio)==>|     cc1101    |--(SPI)-->|     ESP32     |--(USB)-->|USB Serial Host|
            |               |          |               |          |               |
            +---------------+          +---------------+          +---------------+
```

![Image](https://github.com/user-attachments/assets/43baaab3-170c-4933-8baa-856596e29894)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\CC1101_transmitte   


# Using Windows Terminal Software
When you connect the USB cable to the USB port on your Windows machine and build the firmware, a new COM port will appear.   
Open a new COM port in the terminal software.   
When set to Radio to USB, the data received from Radio will be displayed on the terminal.   
When set to USB to Radio, input data from the keyboard is sent to Radio.   
I used TeraTerm as terminal software.   
![tusb-TeraTerm](https://github.com/user-attachments/assets/b5eea94e-5228-45b4-bcad-81cce8c52479)

# Using Linux Terminal Software
When you connect the USB cable to the USB port on your Linux machine and build the firmware, a new /dev/tty device will appear.   
Open a new tty device in the terminal software.   
Most occasions, the device is /dev/ttyACM0.   
When set to Radio to USB, the data received from Radio will be displayed on the terminal.   
When set to USB to Radio, input data from the keyboard is sent to Radio.   
I used screen as terminal software.   
![tusb-screen](https://github.com/user-attachments/assets/18a6e519-9250-4109-b05d-6bcd418bfb5b)

