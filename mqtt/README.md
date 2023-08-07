# MQTT Example   
This is cc1101 and MQTT gateway application.   
```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|  cc1101  |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+

            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|  cc1101  |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```



# Configuration
![config-top](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/ffbe9b7d-04b3-4929-9f5a-b3f9c588bcfe)
![config-app](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/cf5df880-3012-4ca5-82fd-ef2cb445995a)


## WiFi Setting

![config-wifi](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/b5fd7791-b350-4831-bb03-b87d473f7bcf)


## Radioi Setting

### MQTT to Radio
Subscribe with MQTT and send to Radio.   
You can use mosquitto_pub as Publisher.   
```sh ./mqtt_pub.sh```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|  cc1101  |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```

![config-radio-1](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/db337006-a8ef-44e1-8463-317352997e4d)


Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\CC1101_receive   


### Radio to MQTT
Receive from Radio and publish as MQTT.   
You can use mosquitto_sub as Subscriber.   
```sh ./mqtt_sub.sh```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|  cc1101  |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```

![config-radio-2](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/50627a8b-8d86-41bd-9dc6-4bb438b71b84)


Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\CC1101_transmitte   


### Specifying an MQTT Broker   
MQTT broker is specified by one of the following.   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```


# MQTT Broker for ESP32   
https://github.com/nopnop2002/esp-idf-mqtt-broker
