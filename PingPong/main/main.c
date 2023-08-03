/* The example of CC1101
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <cc1101.h>

#define TAG "MAIN"

// Get signal strength indicator in dBm.
// See: http://www.ti.com/lit/an/swra114d/swra114d.pdf
int rssi(char raw) {
	uint8_t rssi_dec;
	// TODO: This rssi_offset is dependent on baud and MHz; this is for 38.4kbps and 433 MHz.
	uint8_t rssi_offset = 74;
	rssi_dec = (uint8_t) raw;
	if (rssi_dec >= 128)
		return ((int)( rssi_dec - 256) / 2) - rssi_offset;
	else
		return (rssi_dec / 2) - rssi_offset;
}

// Get link quality indicator.
int lqi(char raw) {
	return 0x3F - raw;
}

#if CONFIG_PRIMARY
void primary_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CCPACKET packet_sent;
	CCPACKET packet_recv;
	while(1) {
		packet_sent.length = sprintf((char *)packet_sent.data, "Hello World %"PRIu32, xTaskGetTickCount());
		// We also need to include the 0 byte at the end of the string
		packet_sent.data[packet_sent.length] = 0;
		packet_sent.length = packet_sent.length + 1;
		ESP_LOGD(pcTaskGetName(0), "packet_sent.length=%d", packet_sent.length);
		sendData(packet_sent);

		// Wait for a response from the other party
		bool waiting = true;
		TickType_t startTick = xTaskGetTickCount();
		while(waiting) {
			if(packet_available()) {
				TickType_t respTick = xTaskGetTickCount() - startTick;
				if (receiveData(&packet_recv) > 0) {
					ESP_LOGI(pcTaskGetName(0), "Received packet...");
					if (!packet_recv.crc_ok) {
						ESP_LOGE(pcTaskGetName(0), "crc not ok");
					} else {
						ESP_LOGI(pcTaskGetName(0),"Responce time: %"PRIu32, respTick);
						ESP_LOGD(pcTaskGetName(0),"packet_recv.lqi: %d", lqi(packet_recv.lqi));
						ESP_LOGD(pcTaskGetName(0),"packet_recv.rssi: %ddBm", rssi(packet_recv.rssi));
						if (packet_recv.length > 0) {
							ESP_LOGI(pcTaskGetName(0),"packet_recv.length: %d", packet_recv.length);
							ESP_LOGI(pcTaskGetName(0),"[%s] --> [%s]", (char *) packet_sent.data, (char *) packet_recv.data);
						}
					}
					waiting = false;
				} // end receiveData
			} // end packet_available
			TickType_t diffTick = xTaskGetTickCount() - startTick;
			if (diffTick > 100) {
				ESP_LOGE(pcTaskGetName(0), "No responce from others");
				waiting = false;
			}
			vTaskDelay(1);
		} // end while
		vTaskDelay(1000/portTICK_PERIOD_MS);
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_PRIMARY

#if CONFIG_SECONDARY
void secondary_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CCPACKET packet;
	while(1) {
		if(packet_available()) {
			if (receiveData(&packet) > 0) {
				ESP_LOGI(pcTaskGetName(0), "Received packet...");
				if (!packet.crc_ok) {
					ESP_LOGE(pcTaskGetName(0), "crc not ok");
				}
				ESP_LOGI(pcTaskGetName(0),"lqi: %d", lqi(packet.lqi));
				ESP_LOGI(pcTaskGetName(0),"rssi: %ddBm", rssi(packet.rssi));

				if (packet.crc_ok && packet.length > 0) {
					ESP_LOGI(pcTaskGetName(0),"len: %d", packet.length);
					ESP_LOGI(pcTaskGetName(0),"data: %s", (const char *) packet.data);

					for (int i=0;i<packet.length;i++) {
						if (islower(packet.data[i])) {
							packet.data[i] = toupper(packet.data[i]);
						} else {
							packet.data[i] = tolower(packet.data[i]);
						}
					}
					sendData(packet);
					ESP_LOGI(pcTaskGetName(0),"send back....");

				}
			} // end receiveData
		} // end packet_available
		vTaskDelay(1);
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_SECONDARY

void app_main()
{
	uint8_t freq;
#if CONFIG_CC1101_FREQ_433
	freq = CFREQ_433;
	ESP_LOGW(TAG, "Set frequency to 433MHz");
#elif CONFIG_CC1101_FREQ_868
	freq = CFREQ_868;
	ESP_LOGW(TAG, "Set frequency to 868MHz");
#elif CONFIG_CC1101_FREQ_915
	freq = CFREQ_915;
	ESP_LOGW(TAG, "Set frequency to 915MHz");
#elif CONFIG_CC1101_FREQ_918
	freq = CFREQ_918;
	ESP_LOGW(TAG, "Set frequency to 918MHz");
#endif

	uint8_t mode;
#if CONFIG_CC1101_SPEED_4800
	mode = MODE_LOW_SPEED;
	ESP_LOGW(TAG, "Set speed to 4800bps");
#elif CONFIG_CC1101_SPEED_38400
	mode = MODE_HIGH_SPEED;
	ESP_LOGW(TAG, "Set speed to 38400bps");
#endif

	//init(CFREQ_433, 0);
	esp_err_t ret = init(freq, mode);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "CC1101 not installed");
		while(1) { vTaskDelay(1); }
	}

	uint8_t syncWord[2] = {199, 10};
	setSyncWordArray(syncWord);
	ESP_LOGW(TAG, "Set channel to %d", CONFIG_CC1101_CHANNEL);
	setChannel(CONFIG_CC1101_CHANNEL);
	disableAddressCheck();
#if CONFIG_CC1101_POWER_HIGH
	ESP_LOGW(TAG, "Set PA_LongDistance");
	setTxPowerAmp(PA_LongDistance);
#endif


#if CONFIG_PRIMARY
	xTaskCreate(&primary_task, "PRIMARY", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_SECONDARY
	xTaskCreate(&secondary_task, "SECONDARY", 1024*3, NULL, 5, NULL);
#endif
}

