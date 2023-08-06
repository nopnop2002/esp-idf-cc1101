# HTTP Example   
This is cc1101 and HTTP gateway application.   
```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            |HTTP Client|--(HTTP)-->|   ESP32   |--(SPI)--->|  cc1101   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+

            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  cc1101   |--(SPI)--->|   ESP32   |--(HTTP)-->|HTTP Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```



# Configuration
![config-top](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/c44f8b44-c104-4f4b-af89-b483a55daeb4)
![config-app](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/b799d77b-8cea-46cc-9743-d5252a3fdab6)

## WiFi Setting

![config-wifi](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/5f59f5a9-ca9e-488a-90bb-4929065e43d0)


## Radioi Setting

### HTTP to Radio
Subscribe with HTTP and send to Radio.   
ESP32 acts as HTTP Server.   
You can use curl as HTTP Client.   
```sh ./http-client.sh```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            |HTTP Client|--(HTTP)-->|   ESP32   |--(SPI)--->|  cc1101   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![config-radio-1](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/26f4bf6f-a0e5-4038-a617-b4fb83badc96)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\CC1101_receive   


### Radio to HTTP
Receive from Radio and publish as HTTP.   
ESP32 acts as HTTP Client.   
You can use nc(netcat) as HTTP Server.   
```sh ./http-server.sh```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  cc1101   |--(SPI)--->|   ESP32   |--(HTTP)-->|HTTP Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![config-radio-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/85cfe7eb-e148-49b6-a9d6-524c321c13d5)

HTTP Server is specified by one of the following.   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```http-server.local```   
- Fully Qualified Domain Name   
 ```http-server.public.io```

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\CC1101_transmitte   



