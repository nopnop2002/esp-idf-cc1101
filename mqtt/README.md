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
![Image](https://github.com/user-attachments/assets/33017a92-f11e-4462-b434-209ef13cea0f)
![Image](https://github.com/user-attachments/assets/24580b39-2688-4c10-ae49-15dbc90942bf)

## WiFi Setting
Set the information of your access point.   
![config-wifi](https://github.com/nopnop2002/esp-idf-cc1101/assets/6020549/b5fd7791-b350-4831-bb03-b87d473f7bcf)

## Radioi Setting
Set the wireless communication direction.   
![Image](https://github.com/user-attachments/assets/5a0719b7-bc83-424d-94f9-df65dd1ff409)

### MQTT to Radio   
Subscribe with MQTT and send to Radio.   
You can use mosquitto_pub as Publisher.   
```mosquitto_pub -h broker.emqx.io -p 1883 -t "/topic/radio/test" -m "test"```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|  cc1101  |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```


Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-cc1101/tree/main/ArduinoCode/CC1101_receive).   

### Radio to MQTT   
Receive from Radio and publish as MQTT.   
You can use mosquitto_sub as Subscriber.   
```mosquitto_sub -d -h broker.emqx.io  -t "/topic/radio/test"```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|  cc1101  |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```


Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-cc1101/tree/main/ArduinoCode/CC1101_transmitte).   

## Broker Setting
Set the information of your MQTT broker.   
![Image](https://github.com/user-attachments/assets/7096e297-1d2c-4469-a08f-41254490de6c)

### Select Transport   
This project supports TCP,SSL/TLS,WebSocket and WebSocket Secure Port.   

- Using TCP Port.   
	TCP Port uses the MQTT protocol.   

- Using SSL/TLS Port.   
	SSL/TLS Port uses the MQTTS protocol instead of the MQTT protocol.   

- Using WebSocket Port.   
	WebSocket Port uses the WS protocol instead of the MQTT protocol.   

- Using WebSocket Secure Port.   
	WebSocket Secure Port uses the WSS protocol instead of the MQTT protocol.   

__Note for using secure port.__   
The default MQTT server is ```broker.emqx.io```.   
If you use a different server, you will need to modify ```getpem.sh``` to run.   
```
chmod 777 getpem.sh
./getpem.sh
```

WebSocket/WebSocket Secure Port may differ depending on the broker used.   
If you use a different MQTT server than the default, you will need to change the port number from the default.   

### Specifying an MQTT Broker   
You can specify your MQTT broker in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

You can use this as broker.   
https://github.com/nopnop2002/esp-idf-mqtt-broker

### Select MQTT Protocol   
This project supports MQTT Protocol V3.1.1/V5.   
![Image](https://github.com/user-attachments/assets/2de382f7-969b-45e4-8d45-db90f091622f)

### Enable Secure Option   
Specifies the username and password if the server requires a password when connecting.   
[Here's](https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-debian-10) how to install and secure the Mosquitto MQTT messaging broker on Debian 10.   
![Image](https://github.com/user-attachments/assets/e2112fbe-8af0-4c31-baf1-7a5c055bfb68)
