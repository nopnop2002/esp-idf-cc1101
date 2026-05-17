# Web Form Example   
Data entered through the web form is sent to Radio.   
Data received from Radio is displayed in a web form.   
```
            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
            |  WebForm  |-(WebSocket)->|   ESP32   |----(SPI)---->|  cc1101   |==(Radio)==>
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+

            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
==(Radio)==>|  cc1101   |----(SPI)---->|   ESP32   |-(WebSocket)->|  WebForm  |
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+
```

I used [this](https://github.com/Molorius/esp32-websocket) component.   
This component can communicate directly with the browser.   
It's a great job.   

# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/78991da0-7d13-4ffc-b60a-7daf422c03a6" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/50703186-54c6-45e7-bed1-be91ebb00c1e" />

## WiFi Setting
Set the information of your access point.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/75f96b44-3066-4826-8405-48df4116004a" />

## Radio Setting
Set the wireless communication direction.   

### Web to LoRa
Data entered through the web form is sent to LoRa.   
```
            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
            |  WebForm  |-(WebSocket)->|   ESP32   |----(SPI)---->|  cc1101   |==(Radio)==>
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/ab57c99d-0c39-4e5d-8321-34845cda4c92" />


### LoRa to Web
Data received from LoRa is displayed in a web form.   

```
            +-----------+              +-----------+              +-----------+
            |           |              |           |              |           |
==(Radio)==>|  cc1101   |----(SPI)---->|   ESP32   |-(WebSocket)->|  WebForm  |
            |           |              |           |              |           |
            +-----------+              +-----------+              +-----------+
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/23aa7910-bb84-424c-82fd-e919726529b7" />


# Launch a web browser
Enter the following in the address bar of your web browser.   
```
http:://{IP of ESP32}/
or
http://esp32-server.local/
```

<img width="947" height="877" alt="Image" src="https://github.com/user-attachments/assets/be8d468b-38d5-4b36-8e37-8eabd8e289fa" />
<img width="947" height="877" alt="Image" src="https://github.com/user-attachments/assets/3f60f399-d2c7-42ce-8f92-3b71620b4c50" />

### Web to LoRa
Enter the data to send in the TextBox and press the Send button.   
<img width="947" height="877" alt="Image" src="https://github.com/user-attachments/assets/c2f27dd1-b8e5-4cc3-899e-182ead60014a" />

### LoRa to Web
The received data will be displayed in the TextBox.   
The Change button changes the number of lines displayed.   
The Copy button copies the received data to the clipboard.   
<img width="947" height="877" alt="Image" src="https://github.com/user-attachments/assets/9d5c15f6-ff11-4197-aaa7-d0d2c5665885" />
<img width="947" height="877" alt="Image" src="https://github.com/user-attachments/assets/28434741-6941-4893-ad33-b9d1588e1121" />

# WEB Pages
WEB Pages are stored in the html folder.   
I used [this](https://bulma.io/) open source css.   
You can change root.html as you like.   


