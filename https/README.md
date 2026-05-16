# HTTPS Example   
This is cc1101 and HTTPS gateway application.   
Receive from cc1101 and send to HTTPS Server.   
ESP32 acts as HTTPS Client.   

```
            +-----------+           +-----------+            +------------+
            |           |           |           |            |            |
==(Radio)==>|  cc1101   |--(SPI)--->|   ESP32   |--(HTTPS)-->|HTTPS Server|
            |           |           |           |            |            |
            +-----------+           +-----------+            +------------+
```

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-cc1101
cd esp-idf-cc1101/https/
chmod 777 mkkey.sh
./mkkey.sh
idf.py menuconfig
idf.py flash
```

mkkey.sh creates a server certificate file.   
The server certificate file is associated with the HTTPS server's IP address.   
mkkey.sh automatically retrieves the IP address of the server on which the script is executed and treats that address as an HTTPS server.   
To manually configure the HTTPS server's IP address, modify the script as follows:   
```
IP="192.168.0.123"
openssl req -x509 -new -nodes -key server.key -subj "/CN=${IP}" -days 10000 -out server.crt
```


# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/61781d23-2936-444b-84db-5da11b1f6d89" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/2ae4b24b-2237-45ee-b2e7-4893c6c890ed" />

## WiFi Setting
Set the information of your access point.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/fd5d6094-303e-4849-bc6a-18e8721fbb1c" />

## Server Setting
Set the information of your HTTPS server.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/21bca4ef-7dd7-4bc6-a617-08743ed30db0" />

# Start the HTTPS server
```
python3 https-server.py
```
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/e267560a-c908-4ad1-817b-df6bf1d78d2a" />

