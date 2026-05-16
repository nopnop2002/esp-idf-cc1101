# SSL Example   
This is cc1101 and SSL gateway application.   
Receive from cc1101 and send to SSL Server.   
ESP32 acts as SSL Client.   

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|  cc1101   |--(SPI)--->|   ESP32   |--(SSL)--->| SSL Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-cc1101
cd esp-idf-cc1101/ssl/
chmod 777 mkkey.sh
./mkkey.sh
idf.py menuconfig
idf.py flash
```

mkkey.sh creates a server certificate file.   
The server certificate file is associated with the SSL server's IP address.   
mkkey.sh automatically retrieves the IP address of the server on which the script is executed and treats that address as an SSL server.   
To manually configure the SSL server's IP address, modify the script as follows:   
```
IP="192.168.0.123"
openssl req -x509 -new -nodes -key server.key -subj "/CN=${IP}" -days 10000 -out server.crt
```


# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/7ffd698c-f7b9-4d3d-8381-08ff091acd71" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/761ee317-65a4-433b-bfd7-0de66ad8d6a0" />

## WiFi Setting
Set the information of your access point.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/51cab054-78b1-4d5e-8809-a653162f91b8" />

## Server Setting
Set the information of your SSL server.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/55db12e7-e22c-4c79-bc10-692facf209b9" />

# Start the SSL server
- C language
	```
	cd clang-tls-communication
	make
	./server
	```
	<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/c33df36e-05c7-43ad-9ad4-bc26ad74100c" />

- python script
	```
	cd python-tls-communication
	python3 server.py
	```
	<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/58fe9f0d-d621-4418-b244-2272e4eff103" />

