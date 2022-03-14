/*
 * ESP32 driver library for TI CC1100 Low-Power Sub-1 GHz RF Transceiver
 *
 * Based on https://github.com/SpaceTeddy/CC1101, by Christian Weithe
 *
 * Modified and adapted for ESP32 by:
 * LoBo, https://github.com/loboris;	06/2017
 *
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "libcc1100.h"

#if CONFIG_IDF_TARGET_ESP32
#define LCD_HOST HSPI_HOST
#elif CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST SPI2_HOST
#elif defined CONFIG_IDF_TARGET_ESP32C3
#define LCD_HOST SPI2_HOST
#endif

int8_t last_rssi_dbm = 0;
uint8_t last_lqi = 0;
uint8_t last_crc = 0;
uint8_t debug_level = 1;

static spi_device_handle_t handle = NULL;

static const char tag[] = "[CC1101]";

static const char *strFreq[4] = {"315", "433", "868", "915" };
static const char *strMode[6] = {"GFSK 1.2 KBd", "GFSK 38.4 KBd", "GFSK 100 KBd", "MSK 250 KBd", "MSK 500 KBd", "OOK 4.8 KBd" };

//==== Global default settings for 868 MHz ============

//-------------------------------------------------
static uint8_t cc1100_GFSK_1_2_kb[CFG_REGISTER] = {
	0x07,  // IOCFG2				GDO2 Output Pin Configuration (Asserts when a packet has been received with CRC OK. De-asserts when the first byte is read from the RX FIFO.)
	0x2E,  // IOCFG1				GDO1 Output Pin Configuration (High impedance (3-state))
	0x80,  // IOCFG0				GDO0 Output Pin Configuration (Enable analog temperature sensor.)
	0x07,  // FIFOTHR				RX FIFO and TX FIFO Thresholds (33 in TX FIFO, 32 in RX FIFO)
	0x57,  // SYNC1					Sync Word, High Byte
	0x43,  // SYNC0					Sync Word, Low Byte
	0x3E,  // PKTLEN				Packet Length
	0x0E,  // PKTCTRL1			Packet Automation Control (sync word is always accepted & CRC_AUTOFLUSH=ON & APPEND_STATUS=ON & Address check and 0 (0x00) broadcast)
	0x45,  // PKTCTRL0			Packet Automation Control (WHITE_DATA=ON & CRC=ON & Variable packet length mode)
	0xFF,  // ADDR					Device Address
	0x00,  // CHANNR				Channel Number
	0x08,  // FSCTRL1				Frequency Synthesizer Control
	0x00,  // FSCTRL0				Frequency Synthesizer Control
	0x21,  // FREQ2					Frequency Control Word, High Byte
	0x65,  // FREQ1					Frequency Control Word, Middle Byte
	0x6A,  // FREQ0					Frequency Control Word, Low Byte
	0xF5,  // MDMCFG4				Modem Configuration
	0x83,  // MDMCFG3				Modem Configuration
	0x13,  // MDMCFG2				Modem Configuration
	0xA0,  // MDMCFG1				Modem Configuration
	0xF8,  // MDMCFG0				Modem Configuration
	0x15,  // DEVIATN				Modem Deviation Setting
	0x07,  // MCSM2					Main Radio Control State Machine Configuration
	0x0C,  // MCSM1					Main Radio Control State Machine Configuration
	0x19,  // MCSM0					Main Radio Control State Machine Configuration
	0x16,  // FOCCFG				Frequency Offset Compensation Configuration
	0x6C,  // BSCFG					Bit Synchronization Configuration
	0x03,  // AGCCTRL2			AGC Control
	0x40,  // AGCCTRL1			AGC Control
	0x91,  // AGCCTRL0			AGC Control
	0x02,  // WOREVT1				High Byte Event0 Timeout
	0x26,  // WOREVT0				Low Byte Event0 Timeout
	0x09,  // WORCTRL				Wake On Radio Control
	0x56,  // FREND1				Front End RX Configuration
	0x17,  // FREND0				Front End TX Configuration
	0xA9,  // FSCAL3				Frequency Synthesizer Calibration
	0x0A,  // FSCAL2				Frequency Synthesizer Calibration
	0x00,  // FSCAL1				Frequency Synthesizer Calibration
	0x11,  // FSCAL0				Frequency Synthesizer Calibration
	0x41,  // RCCTRL1				RC Oscillator Configuration
	0x00,  // RCCTRL0				RC Oscillator Configuration
	0x59,  // FSTEST				Frequency Synthesizer Calibration Control,
	0x7F,  // PTEST					Production Test
	0x3F,  // AGCTEST				AGC Test
	0x81,  // TEST2					Various Test Settings
	0x3F,  // TEST1					Various Test Settings
	0x0B	 // TEST0					Various Test Settings
};

//--------------------------------------------------
static uint8_t cc1100_GFSK_38_4_kb[CFG_REGISTER] = {
	0x07,  // IOCFG2				GDO2 Output Pin Configuration
	0x2E,  // IOCFG1				GDO1 Output Pin Configuration
	0x80,  // IOCFG0				GDO0 Output Pin Configuration
	0x07,  // FIFOTHR				RX FIFO and TX FIFO Thresholds
	0x57,  // SYNC1					Sync Word, High Byte
	0x43,  // SYNC0					Sync Word, Low Byte
	0x3E,  // PKTLEN				Packet Length
	0x0E,  // PKTCTRL1			Packet Automation Control
	0x45,  // PKTCTRL0			Packet Automation Control
	0xFF,  // ADDR					Device Address
	0x00,  // CHANNR				Channel Number
	0x06,  // FSCTRL1				Frequency Synthesizer Control
	0x00,  // FSCTRL0				Frequency Synthesizer Control
	0x21,  // FREQ2					Frequency Control Word, High Byte
	0x65,  // FREQ1					Frequency Control Word, Middle Byte
	0x6A,  // FREQ0					Frequency Control Word, Low Byte
	0xCA,  // MDMCFG4				Modem Configuration
	0x83,  // MDMCFG3				Modem Configuration
	0x13,  // MDMCFG2				Modem Configuration
	0xA0,  // MDMCFG1				Modem Configuration
	0xF8,  // MDMCFG0				Modem Configuration
	0x34,  // DEVIATN				Modem Deviation Setting
	0x07,  // MCSM2					Main Radio Control State Machine Configuration
	0x0C,  // MCSM1					Main Radio Control State Machine Configuration
	0x19,  // MCSM0					Main Radio Control State Machine Configuration
	0x16,  // FOCCFG				Frequency Offset Compensation Configuration
	0x6C,  // BSCFG					Bit Synchronization Configuration
	0x43,  // AGCCTRL2			AGC Control
	0x40,  // AGCCTRL1			AGC Control
	0x91,  // AGCCTRL0			AGC Control
	0x02,  // WOREVT1				High Byte Event0 Timeout
	0x26,  // WOREVT0				Low Byte Event0 Timeout
	0x09,  // WORCTRL				Wake On Radio Control
	0x56,  // FREND1				Front End RX Configuration
	0x17,  // FREND0				Front End TX Configuration
	0xA9,  // FSCAL3				Frequency Synthesizer Calibration
	0x0A,  // FSCAL2				Frequency Synthesizer Calibration
	0x00,  // FSCAL1				Frequency Synthesizer Calibration
	0x11,  // FSCAL0				Frequency Synthesizer Calibration
	0x41,  // RCCTRL1				RC Oscillator Configuration
	0x00,  // RCCTRL0				RC Oscillator Configuration
	0x59,  // FSTEST				Frequency Synthesizer Calibration Control,
	0x7F,  // PTEST					Production Test
	0x3F,  // AGCTEST				AGC Test
	0x81,  // TEST2					Various Test Settings
	0x3F,  // TEST1					Various Test Settings
	0x0B	 // TEST0					Various Test Settings
};

//-------------------------------------------------
static uint8_t cc1100_GFSK_100_kb[CFG_REGISTER] = {
	0x07,  // IOCFG2				GDO2 Output Pin Configuration
	0x2E,  // IOCFG1				GDO1 Output Pin Configuration
	0x80,  // IOCFG0				GDO0 Output Pin Configuration
	0x07,  // FIFOTHR				RX FIFO and TX FIFO Thresholds
	0x57,  // SYNC1					Sync Word, High Byte
	0x43,  // SYNC0					Sync Word, Low Byte
	0x3E,  // PKTLEN				Packet Length
	0x0E,  // PKTCTRL1			Packet Automation Control
	0x45,  // PKTCTRL0			Packet Automation Control
	0xFF,  // ADDR					Device Address
	0x00,  // CHANNR				Channel Number
	0x08,  // FSCTRL1				Frequency Synthesizer Control
	0x00,  // FSCTRL0				Frequency Synthesizer Control
	0x21,  // FREQ2					Frequency Control Word, High Byte
	0x65,  // FREQ1					Frequency Control Word, Middle Byte
	0x6A,  // FREQ0					Frequency Control Word, Low Byte
	0x5B,  // MDMCFG4				Modem Configuration
	0xF8,  // MDMCFG3				Modem Configuration
	0x13,  // MDMCFG2				Modem Configuration
	0xA0,  // MDMCFG1				Modem Configuration
	0xF8,  // MDMCFG0				Modem Configuration
	0x47,  // DEVIATN				Modem Deviation Setting
	0x07,  // MCSM2					Main Radio Control State Machine Configuration
	0x0C,  // MCSM1					Main Radio Control State Machine Configuration
	0x18,  // MCSM0					Main Radio Control State Machine Configuration
	0x1D,  // FOCCFG				Frequency Offset Compensation Configuration
	0x1C,  // BSCFG					Bit Synchronization Configuration
	0xC7,  // AGCCTRL2			AGC Control
	0x00,  // AGCCTRL1			AGC Control
	0xB2,  // AGCCTRL0			AGC Control
	0x02,  // WOREVT1				High Byte Event0 Timeout
	0x26,  // WOREVT0				Low Byte Event0 Timeout
	0x09,  // WORCTRL				Wake On Radio Control
	0xB6,  // FREND1				Front End RX Configuration
	0x17,  // FREND0				Front End TX Configuration
	0xEA,  // FSCAL3				Frequency Synthesizer Calibration
	0x0A,  // FSCAL2				Frequency Synthesizer Calibration
	0x00,  // FSCAL1				Frequency Synthesizer Calibration
	0x11,  // FSCAL0				Frequency Synthesizer Calibration
	0x41,  // RCCTRL1				RC Oscillator Configuration
	0x00,  // RCCTRL0				RC Oscillator Configuration
	0x59,  // FSTEST				Frequency Synthesizer Calibration Control,
	0x7F,  // PTEST					Production Test
	0x3F,  // AGCTEST				AGC Test
	0x81,  // TEST2					Various Test Settings
	0x3F,  // TEST1					Various Test Settings
	0x0B	 // TEST0					Various Test Settings
};

//------------------------------------------------
static uint8_t cc1100_MSK_250_kb[CFG_REGISTER] = {
	0x07,  // IOCFG2				GDO2 Output Pin Configuration
	0x2E,  // IOCFG1				GDO1 Output Pin Configuration
	0x80,  // IOCFG0				GDO0 Output Pin Configuration
	0x07,  // FIFOTHR				RX FIFO and TX FIFO Thresholds
	0x57,  // SYNC1					Sync Word, High Byte
	0x43,  // SYNC0					Sync Word, Low Byte
	0x3E,  // PKTLEN				Packet Length
	0x0E,  // PKTCTRL1			Packet Automation Control
	0x45,  // PKTCTRL0			Packet Automation Control
	0xFF,  // ADDR					Device Address
	0x00,  // CHANNR				Channel Number
	0x0B,  // FSCTRL1				Frequency Synthesizer Control
	0x00,  // FSCTRL0				Frequency Synthesizer Control
	0x21,  // FREQ2					Frequency Control Word, High Byte
	0x65,  // FREQ1					Frequency Control Word, Middle Byte
	0x6A,  // FREQ0					Frequency Control Word, Low Byte
	0x2D,  // MDMCFG4				Modem Configuration
	0x3B,  // MDMCFG3				Modem Configuration
	0x73,  // MDMCFG2				Modem Configuration
	0xA0,  // MDMCFG1				Modem Configuration
	0xF8,  // MDMCFG0				Modem Configuration
	0x00,  // DEVIATN				Modem Deviation Setting
	0x07,  // MCSM2					Main Radio Control State Machine Configuration
	0x0C,  // MCSM1					Main Radio Control State Machine Configuration
	0x18,  // MCSM0					Main Radio Control State Machine Configuration
	0x1D,  // FOCCFG				Frequency Offset Compensation Configuration
	0x1C,  // BSCFG					Bit Synchronization Configuration
	0xC7,  // AGCCTRL2			AGC Control
	0x00,  // AGCCTRL1			AGC Control
	0xB2,  // AGCCTRL0			AGC Control
	0x02,  // WOREVT1				High Byte Event0 Timeout
	0x26,  // WOREVT0				Low Byte Event0 Timeout
	0x09,  // WORCTRL				Wake On Radio Control
	0xB6,  // FREND1				Front End RX Configuration
	0x17,  // FREND0				Front End TX Configuration
	0xEA,  // FSCAL3				Frequency Synthesizer Calibration
	0x0A,  // FSCAL2				Frequency Synthesizer Calibration
	0x00,  // FSCAL1				Frequency Synthesizer Calibration
	0x11,  // FSCAL0				Frequency Synthesizer Calibration
	0x41,  // RCCTRL1				RC Oscillator Configuration
	0x00,  // RCCTRL0				RC Oscillator Configuration
	0x59,  // FSTEST				Frequency Synthesizer Calibration Control,
	0x7F,  // PTEST					Production Test
	0x3F,  // AGCTEST				AGC Test
	0x81,  // TEST2					Various Test Settings
	0x3F,  // TEST1					Various Test Settings
	0x0B	 // TEST0					Various Test Settings
};

//------------------------------------------------
static uint8_t cc1100_MSK_500_kb[CFG_REGISTER] = {
	0x07,  // IOCFG2				GDO2 Output Pin Configuration
	0x2E,  // IOCFG1				GDO1 Output Pin Configuration
	0x80,  // IOCFG0				GDO0 Output Pin Configuration
	0x07,  // FIFOTHR				RX FIFO and TX FIFO Thresholds
	0x57,  // SYNC1					Sync Word, High Byte
	0x43,  // SYNC0					Sync Word, Low Byte
	0x3E,  // PKTLEN				Packet Length
	0x0E,  // PKTCTRL1			Packet Automation Control
	0x45,  // PKTCTRL0			Packet Automation Control
	0xFF,  // ADDR					Device Address
	0x00,  // CHANNR				Channel Number
	0x0C,  // FSCTRL1				Frequency Synthesizer Control
	0x00,  // FSCTRL0				Frequency Synthesizer Control
	0x21,  // FREQ2					Frequency Control Word, High Byte
	0x65,  // FREQ1					Frequency Control Word, Middle Byte
	0x6A,  // FREQ0					Frequency Control Word, Low Byte
	0x0E,  // MDMCFG4				Modem Configuration
	0x3B,  // MDMCFG3				Modem Configuration
	0x73,  // MDMCFG2				Modem Configuration
	0xA0,  // MDMCFG1				Modem Configuration
	0xF8,  // MDMCFG0				Modem Configuration
	0x00,  // DEVIATN				Modem Deviation Setting
	0x07,  // MCSM2					Main Radio Control State Machine Configuration
	0x0C,  // MCSM1					Main Radio Control State Machine Configuration
	0x18,  // MCSM0					Main Radio Control State Machine Configuration
	0x1D,  // FOCCFG				Frequency Offset Compensation Configuration
	0x1C,  // BSCFG					Bit Synchronization Configuration
	0xC7,  // AGCCTRL2			AGC Control
	0x40,  // AGCCTRL1			AGC Control
	0xB2,  // AGCCTRL0			AGC Control
	0x02,  // WOREVT1				High Byte Event0 Timeout
	0x26,  // WOREVT0				Low Byte Event0 Timeout
	0x09,  // WORCTRL				Wake On Radio Control
	0xB6,  // FREND1				Front End RX Configuration
	0x17,  // FREND0				Front End TX Configuration
	0xEA,  // FSCAL3				Frequency Synthesizer Calibration
	0x0A,  // FSCAL2				Frequency Synthesizer Calibration
	0x00,  // FSCAL1				Frequency Synthesizer Calibration
	0x19,  // FSCAL0				Frequency Synthesizer Calibration
	0x41,  // RCCTRL1				RC Oscillator Configuration
	0x00,  // RCCTRL0				RC Oscillator Configuration
	0x59,  // FSTEST				Frequency Synthesizer Calibration Control,
	0x7F,  // PTEST					Production Test
	0x3F,  // AGCTEST				AGC Test
	0x81,  // TEST2					Various Test Settings
	0x3F,  // TEST1					Various Test Settings
	0x0B	 // TEST0					Various Test Settings
};

//------------------------------------------------
static uint8_t cc1100_OOK_4_8_kb[CFG_REGISTER] = {
	0x06,  // IOCFG2				GDO2 Output Pin Configuration
	0x2E,  // IOCFG1				GDO1 Output Pin Configuration
	0x06,  // IOCFG0				GDO0 Output Pin Configuration
	0x47,  // FIFOTHR				RX FIFO and TX FIFO Thresholds
	0x57,  // SYNC1					Sync Word, High Byte
	0x43,  // SYNC0					Sync Word, Low Byte
	0xFF,  // PKTLEN				Packet Length
	0x04,  // PKTCTRL1			Packet Automation Control
	0x05,  // PKTCTRL0			Packet Automation Control
	0x00,  // ADDR					Device Address
	0x00,  // CHANNR				Channel Number
	0x06,  // FSCTRL1				Frequency Synthesizer Control
	0x00,  // FSCTRL0				Frequency Synthesizer Control
	0x21,  // FREQ2					Frequency Control Word, High Byte
	0x65,  // FREQ1					Frequency Control Word, Middle Byte
	0x6A,  // FREQ0					Frequency Control Word, Low Byte
	0x87,  // MDMCFG4				Modem Configuration
	0x83,  // MDMCFG3				Modem Configuration
	0x3B,  // MDMCFG2				Modem Configuration
	0x22,  // MDMCFG1				Modem Configuration
	0xF8,  // MDMCFG0				Modem Configuration
	0x15,  // DEVIATN				Modem Deviation Setting
	0x07,  // MCSM2					Main Radio Control State Machine Configuration
	0x30,  // MCSM1					Main Radio Control State Machine Configuration
	0x18,  // MCSM0					Main Radio Control State Machine Configuration
	0x14,  // FOCCFG				Frequency Offset Compensation Configuration
	0x6C,  // BSCFG					Bit Synchronization Configuration
	0x07,  // AGCCTRL2			AGC Control
	0x00,  // AGCCTRL1			AGC Control
	0x92,  // AGCCTRL0			AGC Control
	0x87,  // WOREVT1				High Byte Event0 Timeout
	0x6B,  // WOREVT0				Low Byte Event0 Timeout
	0xFB,  // WORCTRL				Wake On Radio Control
	0x56,  // FREND1				Front End RX Configuration
	0x17,  // FREND0				Front End TX Configuration
	0xE9,  // FSCAL3				Frequency Synthesizer Calibration
	0x2A,  // FSCAL2				Frequency Synthesizer Calibration
	0x00,  // FSCAL1				Frequency Synthesizer Calibration
	0x1F,  // FSCAL0				Frequency Synthesizer Calibration
	0x41,  // RCCTRL1				RC Oscillator Configuration
	0x00,  // RCCTRL0				RC Oscillator Configuration
	0x59,  // FSTEST				Frequency Synthesizer Calibration Control
	0x7F,  // PTEST					Production Test
	0x3F,  // AGCTEST				AGC Test
	0x81,  // TEST2					Various Test Settings
	0x35,  // TEST1					Various Test Settings
	0x09,  // TEST0					Various Test Settings
};

//PAtable index: -30	-20- -15	-10		0		 5		7		 10 dBm
static uint8_t patable_power_315[8] = {0x17,0x1D,0x26,0x69,0x51,0x86,0xCC,0xC3};
static uint8_t patable_power_433[8] = {0x6C,0x1C,0x06,0x3A,0x51,0x85,0xC8,0xC0};
static uint8_t patable_power_868[8] = {0x03,0x17,0x1D,0x26,0x50,0x86,0xCD,0xC0};
static uint8_t patable_power_915[8] = {0x0B,0x1B,0x6D,0x67,0x50,0x85,0xC9,0xC1};
//static uint8_t patable_power_2430[8] = {0x44,0x84,0x46,0x55,0xC6,0x6E,0x9A,0xFE};

// === Two different functions for micr seconds delay ====

// ================================================================

static portMUX_TYPE microsMux = portMUX_INITIALIZER_UNLOCKED;

//------------------------------
unsigned long IRAM_ATTR micros()
{
	static unsigned long lccount = 0;
	static unsigned long overflow = 0;
	unsigned long ccount;
	portENTER_CRITICAL_ISR(&microsMux);
	__asm__ __volatile__ ( "rsr			%0, ccount" : "=a" (ccount) );
	if(ccount < lccount){
		overflow += UINT32_MAX / CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ;
	}
	lccount = ccount;
	portEXIT_CRITICAL_ISR(&microsMux);
	return overflow + (ccount / CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
}

//-------------------------------------------
void IRAM_ATTR delayMicroseconds(uint32_t us)
{
	portDISABLE_INTERRUPTS();
	uint32_t m = micros();
	if (us) {
		uint32_t e = (m + us);
		if (m > e) { //overflow
			while (micros() > e) ;
		}
		while(micros() < e) ;
	}
	portENABLE_INTERRUPTS();
}
// ================================================================

// ================================================================
/*
//--------------------------------------------------
static void IRAM_ATTR delayMicroseconds(uint32_t us)
{
	portDISABLE_INTERRUPTS();
	ets_delay_us(us);
	portENABLE_INTERRUPTS();
}
*/
// ================================================================

// Milli second delay
//---------------------------------
static void IRAM_ATTR delay(int ms)
{
	vTaskDelay(ms / portTICK_PERIOD_MS);
}



// =================== SPI Functions ===================

//==== SPI Initialization for CC1100 ====
static uint8_t spi_begin(void)
{
	esp_err_t ret;

	gpio_reset_pin(CONFIG_CSN_GPIO);
	gpio_set_direction(CONFIG_CSN_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(CONFIG_CSN_GPIO, 1);

	spi_bus_config_t buscfg = {
		.sclk_io_num = CONFIG_SCK_GPIO, // set SPI CLK pin
		.mosi_io_num = CONFIG_MOSI_GPIO, // set SPI MOSI pin
		.miso_io_num = CONFIG_MISO_GPIO, // set SPI MISO pin
		.quadwp_io_num=-1,
		.quadhd_io_num=-1,
		.max_transfer_sz = 1024 // max transfer size is 1024 bytes
	};

	ret = spi_bus_initialize( LCD_HOST, &buscfg, SPI_DMA_CH_AUTO );
	ESP_LOGI(tag, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);

	// =================================================================
	// === We set SPI clock to 5MHz or less so that no delay between ===
	// === address and data byte, or between data bytes is needed		 ===
	// =================================================================
	spi_device_interface_config_t devcfg = {
		.clock_speed_hz=5000000, // SPI clock is 5 MHz!
		.queue_size = 7,
		.mode=0, // SPI mode 0
		.spics_io_num=-1 // we will use manual CS control
	};

	ret = spi_bus_add_device( LCD_HOST, &devcfg, &handle);
	ESP_LOGI(tag, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);

	return ret;
}

//==== De initialize the SPI interface ===
//-----------------------
static void spi_end(void)
{
	spi_bus_remove_device(handle);
	handle = NULL;
}


//==== SPI Transmission ====
//---------------------------------------------------------
//static void IRAM_ATTR spi_send(uint8_t *data, uint32_t len)
static void spi_send(uint8_t *data, uint32_t len)
{
	if (handle == NULL) return;
	if (len == 0) return;

#if 0
	printf("spi_send data out\n");
	for(int i=0;i<len;i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
#endif

	gpio_set_level(CONFIG_CSN_GPIO, 0);
	spi_transaction_t SPITransaction;
	memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
	SPITransaction.length = len * 8;
	SPITransaction.tx_buffer = data;
	SPITransaction.rx_buffer = data;
	spi_device_transmit( handle, &SPITransaction );
	//spi_device_polling_transmit( handle, &SPITransaction );
	gpio_set_level(CONFIG_CSN_GPIO, 1);

#if 0
	printf("spi_send data in\n");
	for(int i=0;i<len;i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
#endif
}

// Write strobe command
//-------------------------------------------------------
//static void IRAM_ATTR spi_write_strobe(uint8_t spi_instr)
static void spi_write_strobe(uint8_t spi_instr)
{
	uint8_t instr = spi_instr;
	spi_send(&instr, 1);
}

// Read one Register
//-----------------------------------------------------------
//static uint8_t IRAM_ATTR spi_read_register(uint8_t spi_instr)
static uint8_t spi_read_register(uint8_t spi_instr)
{
	uint8_t buf[2];
	buf[0] = spi_instr | READ_SINGLE_BYTE;
	buf[1] = 0xFF;

	spi_send(buf, 2);
	return buf[1];
}

// Write one Register
//------------------------------------------------------------------------
//static void IRAM_ATTR spi_write_register(uint8_t spi_instr, uint8_t value)
static void spi_write_register(uint8_t spi_instr, uint8_t value)
{
	uint8_t buf[2];
	buf[0] = spi_instr | WRITE_SINGLE_BYTE;
	buf[1] = value;

	spi_send(buf, 2);
}

// Write several consecutive registers at once
//-----------------------------------------------------------------------------------------------+------
//static void IRAM_ATTR spi_write_burst(uint8_t spi_instr, uint8_t *pArr, uint8_t length, uint8_t *rdback)
static void spi_write_burst(uint8_t spi_instr, uint8_t *pArr, uint8_t length, uint8_t *rdback)
{
	uint8_t buf[length+1];
	buf[0] = spi_instr | WRITE_BURST;
	memcpy(buf+1, pArr, length);

	spi_send(buf, length+1);
	if (rdback) {
		memcpy(rdback, buf+1, length);
	}
}


// CC1100 reset function
// Reset as defined in cc1100 data sheet
//==============
void reset(void)
{
	gpio_set_level(CONFIG_CSN_GPIO, 0);
	delayMicroseconds(10);
	gpio_set_level(CONFIG_CSN_GPIO, 1);
	delayMicroseconds(40);

	spi_write_strobe(SRES);
	delay(1);
}

// Set Power Down
// Put CC1101 into SLEEP MODE
//==================
void powerdown(void)
{
	sidle();
	spi_write_strobe(SPWD); // CC1100 Power Down
}

// WakeUp from SLEEP MODE
//===============
void wakeup(void)
{
	gpio_set_level(CONFIG_CSN_GPIO, 0);
	delayMicroseconds(10);
	gpio_set_level(CONFIG_CSN_GPIO, 1);
	delayMicroseconds(10);

	receive();	// go to RX Mode
}


// =========================== CC1100 initialization	functions ===========================

//---------------------------------------------------------------------------------------------------------------------------
static uint8_t cc_begin(uint8_t addr, uint8_t cc1100_freq, uint8_t cc1100_mode, uint8_t cc1100_channel, uint8_t cc1100_power)
{
	//initialize SPI Interface
	if (spi_begin() != ESP_OK) {
		ESP_LOGE(tag, "SPI initialization error");
		return 0;
	}
	ESP_LOGI(tag, "SPI initialized");

	// CC1100 initial reset
	reset();
	ESP_LOGI(tag, "CC1100 Reset");

	spi_write_strobe(SFTX); delayMicroseconds(100); //flush the TX_fifo content
	spi_write_strobe(SFRX); delayMicroseconds(100); //flush the RX_fifo content

	uint8_t partnum = spi_read_register(PARTNUM); //reads CC1100 part number
	uint8_t version = spi_read_register(VERSION); //reads CC1100 version number
	ESP_LOGI(tag, "Partnumber: %02x Version: %02x", partnum, version);
	//Checks if valid Chip ID is found. Usually 0x03 or 0x14. if not -> abort
	//if (version == 0x00 || version == 0xFF){
	if (version != 0x14){
		ESP_LOGE(tag, "no CC11xx found! [%02x]", version);
		// CC1100 Power down and disable SPI bus
		//cc_end();
		// Disable SPI bus
		spi_end();
		return 0;
	}

	//set modulation mode
	set_mode(cc1100_mode);

	//set ISM band
	set_ISM(cc1100_freq);

	//set channel
	set_channel(cc1100_channel);

	//set output power amplifier
	set_output_power_level(cc1100_power);

	//set my receiver address 'My_Addr' from NVS to global variable
	set_myaddr(addr);

	ESP_LOGI(tag, "Initialization done.");

	//set CC1100 in receive mode
	//receive();
	return 1;
}


//=============================================
uint8_t cc_setup(uint8_t *My_addr, uint8_t dbg)
{
	debug_level = dbg & 7;

	// === Configure input on GDO2 ===
	gpio_config_t io_conf;

	//interrupt disabled
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//bit mask of the GDO2 pin
	io_conf.pin_bit_mask = (1<<CONFIG_GDO2_GPIO);
	//set as input mode
	io_conf.mode = GPIO_MODE_INPUT;
	//enable pull-up mode
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);


	//uint8_t old_ver, ver = 0;
	uint8_t addr, cc1100_freq, cc1100_mode, cc1100_channel, cc1100_power;

	addr = DEFAULT_CC1100_MYADDRESS;
	cc1100_freq = DEFAULT_CC1100_FREQUENCY;
	cc1100_mode = DEFAULT_CC1100_MODE;
	cc1100_channel = DEFAULT_CC1100_CHANNEL;
	cc1100_power = DEFAULT_CC1100_POWER;

	*My_addr = addr;

	// Initialize CC1101 RF-module
	uint8_t res = cc_begin(addr, cc1100_freq, cc1100_mode, cc1100_channel, cc1100_power);
	ESP_LOGI(tag, "cc_begin=%d", res);
	if (res == 0) return 0;

	sidle(); //set to ILDE first
	//set_output_power_level(0); //set PA level in dbm

	spi_write_register(IOCFG2, 0x06); //set module in sync mode detection mode

	show_main_settings(); //shows setting
	show_register_settings(); //shows current CC1101 register values
	ESP_LOGI(tag, "Started.");

	//set to RECEIVE mode
	receive();

	return 1;
}

// Finish the CC1100 operation
//===============
void cc_end(void)
{
	powerdown();	//power down CC1100
	spi_end();		//disable SPI Interface
}

// Show all CC1100 registers
//===============================
void show_register_settings(void)
{
	uint8_t config_reg_verify[CFG_REGISTER+1] = {0xFF};
	uint8_t Patable_verify[9] = {0xFF};
	config_reg_verify[0] = READ_BURST | READ_BURST;
	Patable_verify[0] = PATABLE_BURST | READ_BURST;

	spi_send(config_reg_verify, CFG_REGISTER+1); //reads all 47 config register
	spi_send(Patable_verify, 9); //reads output power settings

	//show_main_settings();
	if (debug_level) {
		printf("Cfg_reg:\r\n");
		for (uint8_t i = 1 ; i < (CFG_REGISTER+1); i++) {
			printf("%02x ", config_reg_verify[i]);
			if ((i % 10) == 0) printf("\r\n");
		}
		printf("\r\nPaTable:\r\n");
		for(uint8_t i = 1 ; i < 9; i++) {
			printf("%02x ", Patable_verify[i]);
		}
		printf("\r\n");
	}
}

// Show settings
//===========================
void show_main_settings(void)
{
	uint8_t ver = CC1100_VERSION;
	printf("Library  : Ver %d.%d\r\n", ver >> 4, ver & 0x0F);
	printf("Mode     : %d %s\r\n", DEFAULT_CC1100_MODE, strMode[DEFAULT_CC1100_MODE-1]);
	printf("Frequency: %d %s MHz\r\n", DEFAULT_CC1100_FREQUENCY, strFreq[DEFAULT_CC1100_FREQUENCY-1]);
	printf("Channel  : %d\r\n", DEFAULT_CC1100_CHANNEL);
	printf("Power    : %d dB\r\n", DEFAULT_CC1100_POWER);
	printf("My_Addr  : %d\r\n", DEFAULT_CC1100_MYADDRESS);
}

// Set idle mode
//=================
uint8_t sidle(void)
{
	uint8_t marcstate;

	spi_write_strobe(SIDLE);

	marcstate = 0xFF; //set unknown/dummy state value
	while (marcstate != 0x01)	//0x01 = sidle
	{
		marcstate = (spi_read_register(MARCSTATE) & 0x1F);
	}
	return 1;
}

// Transmit mode
//====================
uint8_t transmit(void)
{
	uint8_t marcstate, res = 1;

	spi_write_strobe(STX); //sends the data over air

	marcstate = 0xFF; //set unknown/dummy state value
	while( marcstate != 0x01) //0x01 = ILDE after sending data
	{
		//read out state of cc1100 to be sure in IDLE and TX is finished
		marcstate = (spi_read_register(MARCSTATE) & 0x1F);
		if (marcstate == 0x16) {
			//flush the TX_fifo content
			spi_write_strobe(SFTX);
			res = 0;
		}
	}
	return res;
}

// Receive mode
//===================
uint8_t receive(void)
{
	uint8_t marcstate;

	sidle(); //sets to idle first.
	spi_write_strobe(SRX); //writes receive strobe (receive mode)

	marcstate = 0xFF; //set unknown/dummy state value
	while (marcstate != 0x0D)	//0x0D = RX
	{
		//read out state of cc1100 to be sure in RX
		marcstate = (spi_read_register(MARCSTATE) & 0x1F);
	}
	return 1;
}

// TX_payload_burst
//========================================================================================
void tx_payload_burst(uint8_t my_addr, uint8_t rx_addr, uint8_t *txbuffer, uint8_t length)
{
	txbuffer[0] = length-1;
	txbuffer[1] = rx_addr;
	txbuffer[2] = my_addr;

	//TX_fifo debug out
	if (debug_level) {
		printf("TX_FIFO: ");
		for (uint8_t i = 0 ; i < length; i++) {
			printf("%02x ", txbuffer[i]);
		}
		printf("\r\n");
	}

	// writes TX_Buffer +1 because of pktlen must be also transfered
	spi_write_burst(TXFIFO_BURST,txbuffer,length, NULL);
}

// RX_payload_burst - package received
//========================================================
void rx_payload_burst(uint8_t rxbuffer[], uint8_t *pktlen)
{
	uint8_t bytes_in_RXFIFO = spi_read_register(RXBYTES); //reads the number of bytes in RXFIFO
	ESP_LOGD(tag, "rx_payload_burst bytes_in_RXFIFO=0x%02x", bytes_in_RXFIFO);

	rxbuffer[0] = 0;
	*pktlen = 0;
	if ((bytes_in_RXFIFO & 0x7F) && !(bytes_in_RXFIFO & 0x80)) {
		*pktlen = spi_read_register(RXFIFO_SINGLE_BYTE); //received pktlen +1 for complete TX buffer
		rxbuffer[0] = *pktlen;
		for (uint8_t i = 1;i < *pktlen + 3;i++) { //+3 because of i=1 and RSSI and LQI
			rxbuffer[i] = spi_read_register(RXFIFO_SINGLE_BYTE);
		}

		//shows rx_buffer for debug
		if (debug_level) {
			printf("RX_FIFO: ");
			for(uint8_t i = 0 ; i < *pktlen + 3; i++) {
				printf("%02x ", rxbuffer[i]);
			}
			printf("\r\n");
		}
	} else {
		ESP_LOGE(tag, "RX_FIFO: bad RX buffer [%02x]", bytes_in_RXFIFO);
	}

	sidle();
	spi_write_strobe(SFRX);
	delayMicroseconds(100);
	receive();
}

// Send packet
//===========================================================================================================
uint8_t send_packet(uint8_t my_addr, uint8_t rx_addr, uint8_t *txbuffer, uint8_t pktlen,	uint8_t tx_retries)
{
	uint8_t pktlen_ack;
	uint8_t rxbuffer[FIFOBUFFER];
	uint8_t tx_retries_count = 0;
	uint32_t ackWaitCounter = 0;

	// === Send package with retries ===
	do
	{
		tx_payload_burst(my_addr, rx_addr, txbuffer, pktlen); //loads the data in cc1100 buffer
		//send data over air
		if (transmit() == 0) {
			ESP_LOGE(tag, "TX Underflow (%d)", tx_retries_count);
			receive(); //receive mode
			return 0;
		}
		receive(); //receive mode

		if (rx_addr == BROADCAST_ADDRESS) {
			ESP_LOGI(tag, "SEND to BROADCAST_ADDRESS OK.");
			return 1; //successfully sent to BROADCAST_ADDRESS, no ack expected
		}

		// Wait for an acknowledge
		while (ackWaitCounter < (ACK_TIMEOUT*1000))
		{
			if (packet_available() == 1) //if RF package received check package acknowledge
			{
				uint8_t from_sender = rx_addr; //the original message sender address
				rx_fifo_erase(rxbuffer);
				//read packet into buffer
				rx_payload_burst(rxbuffer, &pktlen_ack);
				uint8_t res = check_acknowledge(rxbuffer, pktlen_ack, from_sender, my_addr);
				if (res == 0) {
					ESP_LOGE(tag, "SEND FAILED, bad ACK");
					break;
				}
				ESP_LOGD(tag, "SEND OK after %d retries.", tx_retries_count);
				return 1; //package successfully sent
			}
			else {
				ackWaitCounter += 10; //increment ACK wait counter
				delayMicroseconds(10); //delay to give receiver time
			}
		} // end while

		ackWaitCounter = 0; //resets the ACK_Timeout
		tx_retries_count++; //increase tx retry counter
		if (tx_retries_count < tx_retries) delay(100);

	} while (tx_retries_count < tx_retries); //while count of retries is reaches

	ESP_LOGE(tag, "SEND FAILED, too many retries (%d)", tx_retries);
	return 0; //send failed. too many retries
}

// Send Acknowledge
//=====================================================
void send_acknowledge(uint8_t my_addr, uint8_t tx_addr)
{
	uint8_t pktlen = 0x06; //complete Pktlen for ACK packet
	uint8_t tx_buffer[0x06]; //tx buffer array init

	tx_buffer[3] = 'A'; tx_buffer[4] = 'c'; tx_buffer[5] = 'k'; //fill buffer with ACK Payload

	tx_payload_burst(my_addr, tx_addr, tx_buffer, pktlen); //load payload to CC1100
	transmit(); //send package over the air
	receive(); //set CC1100 in receive mode

	ESP_LOGD(tag, "ACK sent.");
}

// Check if Packet is received
//========================
uint8_t packet_available()
{
	if (gpio_get_level(CONFIG_GDO2_GPIO) == 1) { //if RF package received
		if (spi_read_register(IOCFG2) == 0x06) //if sync word detect mode is used
		{
			while (gpio_get_level(CONFIG_GDO2_GPIO) == 1) ;
		}

		ESP_LOGD(tag, "Packet available");
		return 1;
	}
	return 0;
}

// Check Payload for Acknowledge or Data
//=========================================================================================
uint8_t get_payload(uint8_t rxbuffer[], uint8_t *pktlen, uint8_t *my_addr, uint8_t *sender)
{
	rx_fifo_erase(rxbuffer); //delete rx_fifo buffer
	rx_payload_burst(rxbuffer, pktlen); //read package in buffer

	if (*pktlen == 0x00) { //packet length must be > 0
		ESP_LOGE(tag, "bad packet!");
		return 9;
	}

	*my_addr = rxbuffer[1]; //set receiver address to my_addr
	*sender = rxbuffer[2];

	if (check_acknowledge(rxbuffer, *pktlen, *sender, *my_addr) == 1) {
		//Acknowledge received -> finish
		return 8;
	}
	else {
		//real data, and sent acknowledge
		last_rssi_dbm = rssi_convert(rxbuffer[*pktlen + 1]);//converts receiver strength to dBm
		last_lqi = lqi_convert(rxbuffer[*pktlen + 2]); //get rf quality indicator
		last_crc = check_crc(rxbuffer[*pktlen + 2]); //get packet CRC

		if (rxbuffer[1] == BROADCAST_ADDRESS) //if my receiver address is BROADCAST_ADDRESS
		{
			ESP_LOGW(tag, "BROADCAST message");
		}
		ESP_LOGD(tag, "RSSI: %d  LQI: %02x	CRC: %s", last_rssi_dbm, last_lqi, ((last_crc) ? "OK" : "BAD"));

		*my_addr = rxbuffer[1]; //set receiver address to my_addr
		*sender = rxbuffer[2]; //set from_sender address

		if (*my_addr != BROADCAST_ADDRESS) //send only ack if no BROADCAST_ADDRESS
		{
			send_acknowledge(*my_addr, *sender); //sending acknowledge to sender!
		}
		return 1;
	}
}

// Check ACKNOLAGE
//===========================================================================================
uint8_t check_acknowledge(uint8_t *rxbuffer, uint8_t pktlen, uint8_t sender, uint8_t my_addr)
{
	if ((((pktlen == 0x05) && (rxbuffer[1] == my_addr)) || (rxbuffer[1] == BROADCAST_ADDRESS)) &&
			(rxbuffer[2] == sender) && (rxbuffer[3] == 'A') &&
			(rxbuffer[4] == 'c') && (rxbuffer[5] == 'k')) //Acknowledge received!
	{
		if(rxbuffer[1] == BROADCAST_ADDRESS) { //if receiver address BROADCAST_ADDRESS skip acknowledge
			ESP_LOGW(tag, "BROADCAST ACK");
			return 0;
		}
		last_rssi_dbm = rssi_convert(rxbuffer[pktlen + 1]);
		last_lqi = lqi_convert(rxbuffer[pktlen + 2]);
		last_crc = check_crc(rxbuffer[pktlen + 2]);

		ESP_LOGD(tag, "[ACK!] RSSI: %d LQI: %02x CRC: %s", last_rssi_dbm, last_lqi, ((last_crc) ? "OK" : "BAD"));
		return 1;
	}
	return 0;
}

// Check if Packet is received within defined time in ms
//===========================================
uint8_t wait_for_packet(uint8_t milliseconds)
{
	for (uint8_t i = 0; i < milliseconds; i++)
	{
		delay(1); //delay till system has data available
		if (packet_available())return 1;

		if (i == milliseconds) {
			ESP_LOGW(tag, "No packet received!");
		}
	}
	return 0;
}

// TX_fifo_erase
//===================================
void tx_fifo_erase(uint8_t *txbuffer)
{
	memset(txbuffer, 0, sizeof(FIFOBUFFER));
}

// RX_fifo
//===================================
void rx_fifo_erase(uint8_t *rxbuffer)
{
	memset(rxbuffer, 0, sizeof(FIFOBUFFER));
}

// Set CC1100 address
//===========================
void set_myaddr(uint8_t addr)
{
	//store Address in the CC1100
	spi_write_register(ADDR,addr);
}

// Set channel
//===============================
void set_channel(uint8_t channel)
{
	//store the new channel # in the CC1100
	spi_write_register(CHANNR,channel);
}

// Set modulation mode
//=========================
void set_mode(uint8_t mode)
{
	uint8_t *Cfg_reg;

	switch (mode)
	{
		case 0x01:
			Cfg_reg = cc1100_GFSK_1_2_kb;	//sets up settings for GFSK 1.2 kbit mode/speed
			break;
		case 0x02:
			Cfg_reg = cc1100_GFSK_38_4_kb; //sets up settings for GFSK 38.4 kbit mode/speed
			break;
		case 0x03:
			Cfg_reg = cc1100_GFSK_100_kb;	//sets up settings for GFSK 100 kbit mode/speed
			break;
		case 0x04:
			Cfg_reg = cc1100_MSK_250_kb; //sets up settings for MSK 250 kbit mode/speed
			break;
		case 0x05:
			Cfg_reg = cc1100_MSK_500_kb; //sets up settings for MSK 500 kbit mode/speed
			break;
		case 0x06:
			Cfg_reg = cc1100_OOK_4_8_kb; //sets up settings for OOK 4.8 kbit mode/speed
			break;
		default:
			Cfg_reg = cc1100_GFSK_38_4_kb; //sets up settings for GFSK 38,4 kbit mode/speed
			mode = 0x02;
			break;
	}

	//write all 47 config register
	spi_write_burst(WRITE_BURST,Cfg_reg,CFG_REGISTER, NULL);
}

// Set ISM Band
//============================
void set_ISM(uint8_t ism_freq)
{
	uint8_t freq2, freq1, freq0;
	uint8_t *Patable;

	switch (ism_freq) //loads the RF freq which is defined in cc1100_freq_select
	{
		case 0x01: //315MHz
			freq2=0x0C;
			freq1=0x1D;
			freq0=0x89;
			Patable = patable_power_315;
			break;
		case 0x02: //433.92MHz
			freq2=0x10;
			freq1=0xB0;
			freq0=0x71;
			Patable = patable_power_433;
			break;
		case 0x03: //868.3MHz
			freq2=0x21;
			freq1=0x65;
			freq0=0x6A;
			Patable = patable_power_868;
			break;
		case 0x04: //915MHz
			freq2=0x23;
			freq1=0x31;
			freq0=0x3B;
			Patable = patable_power_915;
			break;
		/*
		case 0x05: //2430MHz
			freq2=0x5D;
			freq1=0x76;
			freq0=0x27;
			Patable = patable_power_2430;
			break;
		*/
		default: //default is 868.3MHz
			freq2=0x21;
			freq1=0x65;
			freq0=0x6A;
			Patable = patable_power_868;
			ism_freq = 0x03;
			break;
	}

	//store the new frequency setting for defined ISM band
	spi_write_register(FREQ2,freq2);
	spi_write_register(FREQ1,freq1);
	spi_write_register(FREQ0,freq0);

	//write output power settings to cc1100
	spi_write_burst(PATABLE_BURST,Patable,8, NULL);
}

// Set user frequency
//==========================
void set_freq(uint32_t freq)
{
	// this is split into 3 bytes that are written to 3 different registers on the CC1101
	uint32_t reg_freq = freq / (CRYSTAL_FREQUENCY>>16);

	uint8_t freq2 = (reg_freq>>16) & 0xFF; // high byte, bits 7..6 are always 0 for this register
	uint8_t freq1 = (reg_freq>>8) & 0xFF; // middle byte
	uint8_t freq0 = reg_freq & 0xFF; // low byte

	ESP_LOGW(tag, "FREQUENCY SET to %u [%02x %02x %02x]", freq, freq2, freq1, freq0);

	sidle();
	//store the new frequency setting for defined ISM band
	spi_write_register(FREQ2,freq2);
	spi_write_register(FREQ1,freq1);
	spi_write_register(FREQ0,freq0);
	receive();
}

// Set PATABLE
//====================================
void set_patable(uint8_t *patable_arr)
{
	//write output power settings to cc1100
	spi_write_burst(PATABLE_BURST,patable_arr,8, NULL);
}

// Set output power
//=====================================
void set_output_power_level(int8_t dBm)
{
	uint8_t pa = 0xC0;

	if (dBm <= -30) pa = 0x00;
	else if (dBm <= -20) pa = 0x01;
	else if (dBm <= -15) pa = 0x02;
	else if (dBm <= -10) pa = 0x03;
	else if (dBm <= 0) pa = 0x04;
	else if (dBm <= 5) pa = 0x05;
	else if (dBm <= 7) pa = 0x06;
	else if (dBm <= 10) pa = 0x07;

	spi_write_register(FREND0,pa);
}

// Set Modulation type: 2-FSK=0; GFSK=1; ASK/OOK=3; 4-FSK=4; MSK=7
//===================================
void set_modulation_type(uint8_t cfg)
{
	uint8_t data;
	data = spi_read_register(MDMCFG2);
	data = (data & 0x8F) | (((cfg) << 4) & 0x70);
	spi_write_register(MDMCFG2, data);
}

// Set preamble length
//================================
void set_preamble_len(uint8_t cfg)
{
	uint8_t data;
	data = spi_read_register(MDMCFG1);
	data = (data & 0x8F) | (((cfg) << 4) & 0x70);
	spi_write_register(MDMCFG1, data);
}

// Set modem data rate and deviation
//==================================================================
void set_datarate(uint8_t mdmcfg4, uint8_t mdmcfg3, uint8_t deviant)
{
	spi_write_register(MDMCFG4, mdmcfg4);
	spi_write_register(MDMCFG3, mdmcfg3);
	spi_write_register(DEVIATN, deviant);
}

// Set sync mode no sync=0
//=============================
void set_sync_mode(uint8_t cfg)
{
	uint8_t data;
	data = spi_read_register(MDMCFG2);
	data = (data & 0xF8) | (cfg & 0x07);
	spi_write_register(MDMCFG2, data);
}

// Enable or disable Forward Error Correction (FEC) with interleaving for packet payload
//=======================
void set_fec(uint8_t cfg)
{
	uint8_t data;
	data = spi_read_register(MDMCFG1);
	data = (data & 0x7F) | (((cfg) << 7) & 0x80);
	spi_write_register(MDMCFG1, data);
}

// Set data_whitening ON=TRUE; OFF=FALSE
void set_data_whitening(uint8_t cfg)
{
	uint8_t data;
	data = spi_read_register(PKTCTRL0);
	data = (data & 0xBF) | (((cfg) << 6) & 0x40);
	spi_write_register(PKTCTRL0, data);
}

// Set Manchester encoding ON=TRUE; OFF=FALSE]
//=======================================
void set_manchester_encoding(uint8_t cfg)
{
	uint8_t data;
	data = spi_read_register(MDMCFG2);
	data = (data & 0xF7) | (((cfg) << 3) & 0x08);
	spi_write_register(MDMCFG2, data);
}

// rssi_convert
//===================================
int8_t rssi_convert(uint8_t Rssi_hex)
{
	int8_t rssi_dbm = 0;
	int16_t Rssi_dec;

	Rssi_dec = Rssi_hex; //convert unsigned to signed

	if (Rssi_dec >= 128) rssi_dbm=((Rssi_dec-256)/2)-RSSI_OFFSET_868MHZ;
	else if (Rssi_dec<128) rssi_dbm=((Rssi_dec)/2)-RSSI_OFFSET_868MHZ;

	return rssi_dbm;
}

// lqi convert
//==============================
uint8_t lqi_convert(uint8_t lqi)
{
	return (lqi & 0x7F);
}

// check crc
//============================
uint8_t check_crc(uint8_t lqi)
{
	return (lqi & 0x80);
}

#if 0
// Get temperature
//===========================
float get_temp(uint16_t wait)
{
	if (adc_channel > 7) {
			if (debug_level > 0) printf("Temperature sensor not used.\r\n");
		return -999.9;
	}

	const uint8_t num_samples = 8;
	int adc_result = 0.0;
	float temperature = 0.0;

	sidle(); // set CC1100 into IDLE
	spi_write_register(PTEST,0xBF);	// enable temperature sensor
	delay(50); // wait a bit

	for (uint8_t i=0;i<num_samples;i++) { // sample analog temperature value
		adc_result += adc1_get_voltage(adc_channel);
		delay(1);
	}
	adc_result = adc_result / num_samples;

	temperature = ((((float)adc_result * CC1100_TEMP_ADC_MV) - CC1100_TEMP_V_AT_TEMP) / CC1100_TEMP_CELS_CO) + CC1100_KNOWN_TEMP;

	if (debug_level > 0) {
		printf("ADC: %d  Voltage: %4.1f mV\r\n", adc_result, (float)adc_result * CC1100_TEMP_ADC_MV);
		printf("Temperature: %3.1f\r\n", temperature);
	}

	uint16_t n = 0;
	while (n < wait) {
		vTaskDelay(1000 / portTICK_RATE_MS);
		n++;
	}
	spi_write_register(PTEST,0x7F);	//write 0x7F back to PTest (app. note)

	receive();
	return temperature;
}
#endif

