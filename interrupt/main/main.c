/* The example of CC1101
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <cc1101.h>

#define GPIO_INPUT_IO CONFIG_INPUT_GPIO
#define GPIO_INPUT_PIN_SEL	(1ULL<<GPIO_INPUT_IO)
#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "MAIN";

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void gpio(void *pvParameters)
{
	uint32_t io_num;
	while (1) {
		if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
			ESP_LOGW(pcTaskGetName(NULL), "GPIO[%"PRIu32"] intr, val: %d", io_num, gpio_get_level(io_num));
		} else {
			ESP_LOGE(pcTaskGetName(NULL), "xQueueReceive fail");
			break;
		}
	}
	vTaskDelete( NULL );
}

#if CONFIG_SENDER
void tx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CCPACKET packet;
	while(1) {
		packet.length = sprintf((char *)packet.data, "Hello World %"PRIu32, xTaskGetTickCount());
		sendData(packet);
		ESP_LOGI(pcTaskGetName(NULL), "Sent packet. length=%d", packet.length);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	} // end while

	// never reach here
	vTaskDelete( NULL );
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
					}
				}
			} // end receiveData
		} // end packet_available
		vTaskDelay(1);
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_RECEIVER

void app_main()
{
	//zero-initialize the config structure.
	gpio_config_t io_conf = {};
	//interrupt of rising edge
	io_conf.intr_type = GPIO_INTR_POSEDGE;
	//set as input mode
	io_conf.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	//create a queue to handle gpio event from isr
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

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

	// Initialize CC1101
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
	
	// Asserts when RX FIFO is filled at or above the RX FIFO threshold or the end of packet is reached.
	// De-asserts when the RX FIFO is empty.
	//setAssert(0x01);

	// Asserts when sync word has been sent / received, and de-asserts at the end of the packet.
	setAssert(0x06);

	// Asserts when a packet has been received with CRC OK.
	// De-asserts when the first byte is read from the RX FIFO.
	//setAssert(0x07);

	// Install gpio isr service
	// The interrupt service is already enabled in the init function.
	//gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	// Hook isr handler for specific gpio pin
	gpio_isr_handler_add(GPIO_INPUT_IO, gpio_isr_handler, (void*) GPIO_INPUT_IO);

	// Start gpio interrupt task
	xTaskCreate(&gpio, "GPIO", 1024*2, NULL, 5, NULL);

#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
#endif
}
