# PingPong Example   
Send data from primary to secondary.   
In the secondary, the characters are converted and sent back.   

- ESP32 is Primary   
```
+-----------+           +-----------+             +-----------+           +-----------+
|           |           |           |             |           |           |           |
|  Primary  |===(SPI)==>|  CC1101   |---(Radio)-->|  CC1101   |===(SPI)==>| Secondary |
|   ESP32   |           |           |             |           |           |           |
|           |           |           |             |           |           |           |
|           |<==(SPI)===|           |<--(Radio)---|           |<==(SPI)===|           |
|           |           |           |             |           |           |           |
+-----------+           +-----------+             +-----------+           +-----------+
```

- ESP32 is Secondary   

```
+-----------+           +-----------+             +-----------+           +-----------+
|           |           |           |             |           |           |           |
|  Primary  |===(SPI)==>|  CC1101   |---(Radio)-->|  CC1101   |===(SPI)==>| Secondary |
|           |           |           |             |           |           |   ESP32   |
|           |           |           |             |           |           |           |
|           |<==(SPI)===|           |<--(Radio)---|           |<==(SPI)===|           |
|           |           |           |             |           |           |           |
+-----------+           +-----------+             +-----------+           +-----------+
```

# Configuration   
![config-pingpong-1](https://user-images.githubusercontent.com/6020549/158708163-9bacbe80-bb37-4705-a3f7-ef4c1d098f97.jpg)
![config-pingpong-2](https://user-images.githubusercontent.com/6020549/158708129-7037e259-105d-47ee-b49f-e30eb4f7aa47.jpg)

# Communication with the Arduino environment   
- ESP32 is the primary   
I tested it with [this](https://github.com/nopnop2002/esp-idf-cc1101/tree/main/ArduinoCode/CC1101_pong).   

- ESP32 is the secondary   
I tested it with [this](https://github.com/nopnop2002/esp-idf-cc1101/tree/main/ArduinoCode/CC1101_ping).   

# Screen Shot   
![ScreenShot-pingpong-1](https://user-images.githubusercontent.com/6020549/167343204-58dbf11b-a0ff-4e9f-a6ff-def1a3405e36.jpg)
![ScreenShot-pingpong-2](https://user-images.githubusercontent.com/6020549/167343197-895c8aa2-b7b5-46b6-9b34-3deb3d92ffba.jpg)

