# Interrupt Example   
On the CC1101, GDO0, GDO1, and GDO2 can be used as interrupt output signals.   
GDO1 is shared with the SO pin in the SPI interface.   
This library uses the GDO0 interrupt internally and exposes the GDO2 interrupt externally.   
The CC1101 supports the following interrupts.   
This project demonstrates how to use asserts when sync word has been sent / received, and de-asserts at the end of the packet.   

<table>
	<tbody>
		<tr>
			<td align="center">Assert value</td>
			<td align="center">Description</td>
		</tr>
		<tr>
			<td align="center">0x01</td>
			<td>Asserts when RX FIFO is filled at or above the RX FIFO threshold.<br />De-asserts when RX FIFO is drained below the same threshold.</td>
		</tr>
		<tr>
			<td align="center">0x02</td>
			<td>Asserts when RX FIFO is filled at or above the RX FIFO threshold or the end of packet is reached.<br />De-asserts when the RX FIFO is empty.</td>
		</tr>
		<tr>
			<td align="center">0x03</td>
			<td>Asserts when the TX FIFO is filled at or above the TX FIFO threshold.<br />De-asserts when the TX FIFO is below the same threshold.</td>
		</tr>
		<tr>
			<td align="center">0x04</td>
			<td>Asserts when the RX FIFO has overflowed.<br />De-asserts when the FIFO has been flushed.</td>
		</tr>
		<tr>
			<td align="center">0x05</td>
			<td>Asserts when the TX FIFO has underflowed.<br />De-asserts when the FIFO is flushed.</td>
		</tr>
		<tr>
			<td align="center">0x06</td>
			<td>Asserts when sync word has been sent / received, and de-asserts at the end of the packet.<br />In RX, the pin will also deassert when a packet is discarded due to address or maximum length filtering or when the radio enters RXFIFO_OVERFLOW state.<br />In TX the pin will de-assert if the TX FIFO underflows.</td>
		</tr>
		<tr>
			<td align="center">0x07</td>
			<td>Asserts when a packet has been received with CRC OK.<br />De-asserts when the first byte is read from the RX FIFO.</td>
		</tr>
		<tr>
			<td align="center">0x08</td>
			<td>Asserts when the PQI is above the programmed PQT value.<br />De-asserted when the chip reenters RX state (MARCSTATE=0x0D) or the PQI gets below the programmed PQT value.</td>
		</tr>
	</tbody>
</table>

# Configuration   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/19d04daa-1f5c-4144-a876-423988b2764c" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/af5e250c-8e02-489b-8dfc-87c75f357383" />

# Wiring
Add a wire to GDO2.   
|CC1101||ESP32|ESP32-S2/S3|ESP32-Cx/Hx|
|:-:|:-:|:-:|:-:|:-:|
|MISO|--|GPIO19|GPIO37|GPIO4|
|SCK|--|GPIO18|GPIO36|GPIO3|
|MOSI|--|GPIO23|GPIO35|GPIO2|
|CSN|--|GPIO5|GPIO34|GPIO1|
|GDO0|--|GPIO15|GPIO33|GPIO0|
|GDO2|--|GPIO21|GPIO1|GPIO6|
|GND|--|GND|GND|GND|
|VCC|--|3.3V|3.3V|3.3V|


