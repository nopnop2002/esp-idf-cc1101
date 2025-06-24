/* The example of CC1101
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mdns.h"

#include <cc1101.h>

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event */
/* - are we connected to the AP with an IP? */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "MAIN";

static int s_retry_num = 0;

MessageBufferHandle_t xMessageBufferTrans;
MessageBufferHandle_t xMessageBufferRecv;

// The total number of bytes (not single messages) the message buffer will be able to hold at any one time.
size_t xBufferSizeBytes = 1024;
// The size, in bytes, required to hold each item in the message,
size_t xItemSize = 64; // Maximum Payload size of CC1101 is 64

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

esp_err_t wifi_init_sta(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
		ESP_EVENT_ANY_ID,
		&event_handler,
		NULL,
		&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
		IP_EVENT_STA_GOT_IP,
		&event_handler,
		NULL,
		&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	esp_err_t ret_value = ESP_OK;
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
		ret_value = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret_value = ESP_FAIL;
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
	return ret_value;
}

esp_err_t query_mdns_host(const char * host_name, char *ip)
{
	ESP_LOGD(__FUNCTION__, "Query A: %s", host_name);

	struct esp_ip4_addr addr;
	addr.addr = 0;

	esp_err_t err = mdns_query_a(host_name, 10000,	&addr);
	if(err){
		if(err == ESP_ERR_NOT_FOUND){
			ESP_LOGW(__FUNCTION__, "%s: Host was not found!", host_name);
		} else {
			ESP_LOGE(__FUNCTION__, "Query Failed: %s", esp_err_to_name(err));
		}
		return ESP_FAIL;
	}

	ESP_LOGD(__FUNCTION__, "Query A: %s.local resolved to: " IPSTR, host_name, IP2STR(&addr));
	sprintf(ip, IPSTR, IP2STR(&addr));
	return ESP_OK;
}

void convert_mdns_host(char * from, char * to)
{
	ESP_LOGI(__FUNCTION__, "from=[%s]",from);
	strcpy(to, from);
	char *sp;
	sp = strstr(from, ".local");
	if (sp == NULL) return;

	int _len = sp - from;
	ESP_LOGD(__FUNCTION__, "_len=%d", _len);
	char _from[128];
	strcpy(_from, from);
	_from[_len] = 0;
	ESP_LOGI(__FUNCTION__, "_from=[%s]", _from);

	char _ip[128];
	esp_err_t ret = query_mdns_host(_from, _ip);
	ESP_LOGI(__FUNCTION__, "query_mdns_host=%d _ip=[%s]", ret, _ip);
	if (ret != ESP_OK) return;

	strcpy(to, _ip);
	ESP_LOGI(__FUNCTION__, "to=[%s]", to);
}

void initialize_mdns(void)
{
	//initialize mDNS
	ESP_ERROR_CHECK( mdns_init() );
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set(CONFIG_MDNS_HOSTNAME) );
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);

#if 0
	//set default mDNS instance name
	ESP_ERROR_CHECK( mdns_instance_name_set("ESP32 with mDNS") );
#endif
}

#if CONFIG_SENDER
void tx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CCPACKET packet;
	while(1) {
		packet.length = xMessageBufferReceive(xMessageBufferRecv, packet.data, sizeof(packet.data), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "packet.length=%d", packet.length);
		sendData(packet);
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
		vTaskDelay(1); // Avoid WatchDog alerts
	} // end while
	vTaskDelete( NULL );
}
#endif // CONFIG_RECEIVER

void ws_client(void *pvParameters);
void ws_server(void *pvParameters);

void app_main()
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Initialize WiFi
	ESP_ERROR_CHECK(wifi_init_sta());

	// Create MessageBuffer
	xMessageBufferTrans = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferTrans );
	xMessageBufferRecv = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferRecv );

	// Initialize mDNS
	initialize_mdns();

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
	ret = init(freq, mode);
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

	// Get the local IP address
	esp_netif_ip_info_t ip_info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info));
	char cparam0[64];
	sprintf(cparam0, IPSTR, IP2STR(&ip_info.ip));
	ESP_LOGI(TAG, "cparam0=[%s]", cparam0);

#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
	xTaskCreate(&ws_server, "WS_SERVER", 1024*4, (void *)cparam0, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
	xTaskCreate(&ws_client, "WS_CLIENT", 1024*4, NULL, 5, NULL);
#endif

	while(1) {
		vTaskDelay(10);
	}
}

