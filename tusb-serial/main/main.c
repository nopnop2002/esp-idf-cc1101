/* The example of CC1101
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "esp_log.h"

#include <cc1101.h>

static const char *TAG = "MAIN";

MessageBufferHandle_t xMessageBufferTrans;
MessageBufferHandle_t xMessageBufferRecv;
QueueHandle_t xQueueTinyusb;

// The total number of bytes (not single messages) the message buffer will be able to hold at any one time.
size_t xBufferSizeBytes = 1024;

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
	/* initialization */
	size_t rx_size = 0;
	ESP_LOGD(TAG, "CONFIG_TINYUSB_CDC_RX_BUFSIZE=%d", CONFIG_TINYUSB_CDC_RX_BUFSIZE);

	/* read */
	uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE];
	esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
	if (ret == ESP_OK) {
		ESP_LOGD(TAG, "Data from channel=%d rx_size=%d", itf, rx_size);
		ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
		for(int i=0;i<rx_size;i++) {
			xQueueSendFromISR(xQueueTinyusb, &buf[i], NULL);
		}
	} else {
		ESP_LOGE(TAG, "tinyusb_cdcacm_read error");
	}
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
	int dtr = event->line_state_changed_data.dtr;
	int rts = event->line_state_changed_data.rts;
	ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

#if CONFIG_SENDER
void tx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CCPACKET packet;
	while(1) {
		packet.length = xMessageBufferReceive(xMessageBufferRecv, packet.data, sizeof(packet.data), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "xMessageBufferReceive packet.length=%d", packet.length);
		sendData(packet);
	} // end while

	// never reach here
	vTaskDelete( NULL );
}

void usb_rx(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	char buffer[64]; // Maximum Payload size of cc1101 is 64
	int index = 0;
	while(1) {
		char ch;
		if(xQueueReceive(xQueueTinyusb, &ch, portMAX_DELAY)) {
			ESP_LOGD(pcTaskGetName(NULL), "ch=0x%x",ch);
			if (ch == 0x0d || ch == 0x0a) {
				if (index > 0) {
					ESP_LOGI(pcTaskGetName(NULL), "[%.*s]", index, buffer);
					size_t spacesAvailable = xMessageBufferSpacesAvailable( xMessageBufferRecv );
					ESP_LOGI(pcTaskGetName(NULL), "spacesAvailable=%d", spacesAvailable);
					size_t sended = xMessageBufferSend(xMessageBufferRecv, buffer, index, 100);
					if (sended != index) {
						ESP_LOGE(pcTaskGetName(NULL), "xMessageBufferSend fail index=%d sended=%d", index, sended);
						break;
					}
					index = 0;
				}
			} else {
				if (index == 64) continue;
				buffer[index++] = ch;
			}
		}
	} // end while
	vTaskDelete(NULL);
}
#endif // CONFIG_SENDER

#if CONFIG_RECEIVER
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

void rx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CCPACKET packet;
	while(1) {
		if(packet_available()) {
			if (receiveData(&packet) > 0) {
				ESP_LOGI(pcTaskGetName(NULL), "Received packet...");
				if (!packet.crc_ok) {
					ESP_LOGE(pcTaskGetName(NULL), "crc not ok");
				} else {
					ESP_LOGI(pcTaskGetName(NULL),"packet.lqi: %d", lqi(packet.lqi));
					ESP_LOGI(pcTaskGetName(NULL),"packet.rssi: %ddBm", rssi(packet.rssi));
					ESP_LOGI(pcTaskGetName(NULL),"packet.length: %d", packet.length);
					if (packet.length > 0) {
						ESP_LOGI(pcTaskGetName(NULL),"data: %.*s", packet.length, (char *) packet.data);
						size_t spacesAvailable = xMessageBufferSpacesAvailable( xMessageBufferTrans );
						ESP_LOGI(pcTaskGetName(NULL), "spacesAvailable=%d", spacesAvailable);
						size_t sended = xMessageBufferSend(xMessageBufferTrans, packet.data, packet.length, 100);
						if (sended != packet.length) {
							ESP_LOGE(pcTaskGetName(NULL), "xMessageBufferSend fail packet.length=%d sended=%d", packet.length, sended);
							break;
						}
					}
				}
			} // end receiveData
		} // end packet_available
		vTaskDelay(1);
	} // end while
	vTaskDelete( NULL );
}

void usb_tx(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t buf[64]; // Maximum Payload size of cc1101 is 64
	uint8_t crlf[2] = { 0x0d, 0x0a };
	while(1) {
		size_t received = xMessageBufferReceive(xMessageBufferTrans, buf, sizeof(buf), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received:[%.*s]", received, received, buf);
		tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, buf, received);
		tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, crlf, 2);
		tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
	} // end while
	vTaskDelete(NULL);
}

#endif // CONFIG_RECEIVER

void app_main()
{
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = NULL
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

    // Create MessageBuffer
    xMessageBufferTrans = xMessageBufferCreate(xBufferSizeBytes);
    configASSERT( xMessageBufferTrans );
    xMessageBufferRecv = xMessageBufferCreate(xBufferSizeBytes);
    configASSERT( xMessageBufferRecv );

    // Create Queue
    xQueueTinyusb = xQueueCreate(100, sizeof(char));
    configASSERT( xQueueTinyusb );

	uint8_t freq;
#if CONFIG_CC1101_FREQ_315
	freq = CFREQ_315;
	ESP_LOGW(TAG, "Set frequency to 315MHz");
#elif CONFIG_CC1101_FREQ_433
	freq = CFREQ_433;
	ESP_LOGW(TAG, "Set frequency to 433MHz");
#elif CONFIG_CC1101_FREQ_868
	freq = CFREQ_868;
	ESP_LOGW(TAG, "Set frequency to 868MHz");
#elif CONFIG_CC1101_FREQ_915
	freq = CFREQ_915;
	ESP_LOGW(TAG, "Set frequency to 915MHz");
#endif

	uint8_t mode;
#if CONFIG_CC1101_SPEED_4800
	mode = CSPEED_4800;
	ESP_LOGW(TAG, "Set speed to 4800bps");
#elif CONFIG_CC1101_SPEED_9600
	mode = CSPEED_9600;
	ESP_LOGW(TAG, "Set speed to 38400bps");
#elif CONFIG_CC1101_SPEED_19200
	mode = CSPEED_19200;
	ESP_LOGW(TAG, "Set speed to 38400bps");
#elif CONFIG_CC1101_SPEED_38400
	mode = CSPEED_38400;
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
#if CONFIG_CC1101_POWER_MIN
	ESP_LOGW(TAG, "Set Minimum power level");
	setTxPowerAmp(POWER_MIN);
#elif CONFIG_CC1101_POWER_0db
	ESP_LOGW(TAG, "Set 0 dBm power level");
	setTxPowerAmp(POWER_0db);
#elif CONFIG_CC1101_POWER_MAX
	ESP_LOGW(TAG, "Set Maximum power level");
	setTxPowerAmp(POWER_MAX);
#endif

#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
	xTaskCreate(&usb_rx, "USB_RX", 1024*4, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
	xTaskCreate(&usb_tx, "USB_TX", 1024*4, NULL, 5, NULL);
#endif
}

