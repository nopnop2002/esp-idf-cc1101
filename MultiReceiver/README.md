# How to communicate with multiple receivers

CC1101 can communicate with devices that have the same synchronization word.   
In other words, it cannot communicate with devices that have a different synchronization word.   
Using this function allows you to communicate with a specific device.   

- Receiver1
```
    uint8_t syncWord[2] = {199, 10};
    setSyncWordArray(syncWord);
```

- Receiver2
```
    uint8_t syncWord[2] = {199, 20};
    setSyncWordArray(syncWord);
```

- Sender
```
void tx_task(void *pvParameter)
{
    ESP_LOGI(pcTaskGetName(0), "Start");
    CCPACKET packet;
    int counter = 0;
    uint8_t syncWord1[2] = {199, 10};
    uint8_t syncWord2[2] = {199, 20};
    while(1) {
        if (counter == 0) {
            setSyncWordArray(syncWord1);
        } else {
            setSyncWordArray(syncWord2);
        }

        packet.length = sprintf((char *)packet.data, "Hello World %"PRIu32, xTaskGetTickCount());
        sendData(packet);
        ESP_LOGI(pcTaskGetName(0), "Sent packet. length=%d", packet.length);
        counter++;
        if (counter == 2) counter = 0;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    } // end while

    // never reach here
    vTaskDelete( NULL );
}
```

Now the messages will be sent alternately to the two receivers.
