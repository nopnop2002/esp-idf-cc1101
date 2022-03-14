/* PPPoS Client Example with GSM
 *	(tested with SIM800)
 *	Author: LoBo (loboris@gmail.com, loboris.github)

	 This example code is in the Public Domain (or CC0 licensed, at your option.)

	 Unless required by applicable law or agreed to in writing, this
	 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	 CONDITIONS OF ANY KIND, either express or implied.
 */


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "libcc1100.h"

#define TX_INTERVAL 10000			// Transmit interval in milliseconds

static const char tag[] = "[CC1101 Demo]";

//Global CC1100 address
static uint8_t My_addr;



#if CONFIG_RECEIVER

//===============================
static void CC_RX_task(void* arg)
{
	ESP_LOGI(pcTaskGetTaskName(0), "=== CC1101 RX TASK STARTED on core %d ===", xPortGetCoreID());

	uint8_t Rx_fifo[FIFOBUFFER];
	uint32_t n_rx = 0, nrx_ok = 0, nrx_err = 0;
	uint32_t rf_timecode = 0;
	uint8_t rx_addr, sender, pktlen;
	uint32_t time_stamp;

	while(1) {
		// ==== GDO2 Low->High change detected ====
		if (packet_available() == 1) {
			// === valid package is received ===
			printf("\r\n");
			ESP_LOGW(pcTaskGetTaskName(0), "---------------------------RX---------------------------");
			n_rx++;
			time_stamp = clock();	// generate new time stamp
			uint8_t res = get_payload(Rx_fifo, &pktlen, &rx_addr, &sender);
			time_stamp = clock()-time_stamp;
			if (res == 1) {
				nrx_ok++;
				rf_timecode = ((uint32_t)Rx_fifo[3] << 24) +
					((uint32_t)Rx_fifo[4] << 16) +
					((uint16_t)Rx_fifo[5] <<	8) +
					Rx_fifo[6];

				ESP_LOGI(pcTaskGetTaskName(0), "[%u/%u] Rx_Time: %u ms TX_Timecode: %d ms RSSI: %d LQI: %d CRC: %s",
						nrx_ok, n_rx, time_stamp, rf_timecode, last_rssi_dbm, last_lqi, ((last_crc) ? "OK" : "BAD"));

			}
			else {
				nrx_err++;
				ESP_LOGE(pcTaskGetTaskName(0), "[%u/%u] Error %d", nrx_ok, n_rx, res);
			}
			ESP_LOGW(pcTaskGetTaskName(0), "^^^^^^^^^^^^^^^^^^^^^^^^^^^RX^^^^^^^^^^^^^^^^^^^^^^^^^^^");
		}
		vTaskDelay(1);

	} // end while

	ESP_LOGW(pcTaskGetTaskName(0), "=== CC1101 RX TASK ENDED ===");
	vTaskDelete(NULL);
}
#endif // CONFIG_RECEIVER

#if CONFIG_TRANSMITTER
//===============================
static void CC_TX_task(void* arg)
{
	ESP_LOGI(pcTaskGetTaskName(0), "=== CC1101 TX TASK STARTED on core %d ===", xPortGetCoreID());

	uint32_t time_stamp;
	uint32_t n_tx = 0, n_tx_ok = 0, n_tx_err = 0;
	uint8_t Tx_fifo[FIFOBUFFER];
	uint8_t Rx_addr, Pktlen;

	while (1) {
		printf("\r\n");
		ESP_LOGW(pcTaskGetTaskName(0), "---------------------------TX---------------------------");

		n_tx++;
		Rx_addr = CONFIG_RECEIVER_ADDRESS;	// receiver address

		time_stamp = clock();								// generate new time stamp
		Tx_fifo[3] = (uint8_t)(time_stamp >> 24);			// split 32-Bit time stamp to 4 byte array
		Tx_fifo[4] = (uint8_t)(time_stamp >> 16);
		Tx_fifo[5] = (uint8_t)(time_stamp >> 8);
		Tx_fifo[6] = (uint8_t)(time_stamp);

		Pktlen = 0x07;						// set packet length

		uint8_t res = send_packet(My_addr, Rx_addr, Tx_fifo, Pktlen, 0);	// send package over air. ACK is received via GPIO polling

		time_stamp = clock()-time_stamp;
		if (res) {
			n_tx_ok++;
			ESP_LOGI(pcTaskGetTaskName(0), "[%u/%u] OK; Tx_Time: %u ms [RSSI: %d LQI: %d CRC: %s]", n_tx_ok, n_tx, time_stamp, last_rssi_dbm, last_lqi, ((last_crc) ? "OK" : "BAD"));
		}
		else {
			n_tx_err++;
			ESP_LOGE(pcTaskGetTaskName(0), "[%u/%u] Error", n_tx_err, n_tx);
		}
		ESP_LOGW(pcTaskGetTaskName(0), "^^^^^^^^^^^^^^^^^^^^^^^^^^^TX^^^^^^^^^^^^^^^^^^^^^^^^^^^");
		vTaskDelay(TX_INTERVAL/portTICK_RATE_MS);

	}

	ESP_LOGW(pcTaskGetTaskName(0), "=== CC1101 TX TASK ENDED ===");
	vTaskDelete(NULL);
}
#endif // CONFIG_TRANSMITTER


//=============
void app_main()
{
	vTaskDelay(3000 / portTICK_RATE_MS);

	uint8_t res = cc_setup(&My_addr, 0);
	ESP_LOGI(tag, "cc_setup=%d", res);
	if (res > 0) {
		ESP_LOGI(tag, "My_addr=0x%02x", My_addr);
		//set_output_power_level(-10);

		// === Start tasks ===
#if CONFIG_RECEIVER
		xTaskCreatePinnedToCore(CC_RX_task, "CC_RX_task", 3*1024, NULL, 9, NULL, 1);
#endif
#if CONFIG_TRANSMITTER
		xTaskCreatePinnedToCore(CC_TX_task, "CC_TX_task", 3*1024, NULL, 8, NULL, 1);
#endif
	}
	else {
		ESP_LOGE(tag, "ERROR initializing CC1101.");
	}

	while(1)
	{
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
