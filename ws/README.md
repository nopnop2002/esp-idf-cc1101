# WebSocket Example   
This is cc1101 and WebSocket gateway application.   
```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            | WS Client |-(Socket)->|   ESP32   |--(SPI)--->|  cc1101   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+

            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  cc1101   |--(SPI)--->|   ESP32   |-(Socket)->| WS Server |
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```



# Configuration
![config-top](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/ee7911c4-4407-4bfe-bde2-baa0dbefef15)
![config-app](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/e73d4e95-542c-4ab5-b4db-12240a13fdf4)

## WiFi Setting
Set the information of your access point.   
![config-wifi](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/21dc01ff-61c2-4e23-8571-32065fb7b360)

## Radioi Setting
Set the wireless communication direction.   

### WS to Radio
Receive from WebSocket and send to Radio.   
ESP32 acts as WebSocket Server.   
You can use ws-client.py as WS Client.   
```python3 ws-client.py```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            | WS Client |-(Socket)->|   ESP32   |--(SPI)--->|  cc1101   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![config-radio-1](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/081b3b57-e3aa-4926-939a-15ebb095ff13)

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-cc1101/tree/main/ArduinoCode/CC1101_receive).   

### Radio to WS
Receive from Radio and send to WebSocket.   
ESP32 acts as WebSocket Client.   
Use [this](https://components.espressif.com/components/espressif/esp_websocket_client) component.   
You can use ws-server.py as WS Server.   
```python3 ws-server.py```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  cc1101   |--(SPI)--->|   ESP32   |-(Socket)->| WS Server |
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![config-radio-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/1d22d218-8f3b-45ec-acb9-a3266f9cc492)


Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-cc1101/tree/main/ArduinoCode/CC1101_transmitte).   

### Specifying an WebSocket Server   
You can specify your WebSocket Server in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```ws-server.local```   
- Fully Qualified Domain Name   
 ```ws-server.public.io```


