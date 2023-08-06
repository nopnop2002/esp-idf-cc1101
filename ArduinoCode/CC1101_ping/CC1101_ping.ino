#include <Arduino.h>
#include <cc1101.h> // https://github.com/veonik/arduino-cc1101
#include <ccpacket.h>

// Attach CC1101 pins to their corresponding SPI pins
// Uno pins:
// CSN (SS) => 10
// MOSI => 11
// MISO => 12
// SCK => 13
// GD0 => A valid interrupt pin for your platform (defined below this)

// ATmega2560/1280
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define CC1101Interrupt 4 // Pin 19

// Teensy 3.5
#elif defined(__MK64FX512__)
#define CC1101Interrupt 9 // Pin 9

// ATmega328
#else
#define CC1101Interrupt 0 // Pin 2
#endif

CC1101 radio;

byte syncWord[2] = {199, 10};
bool packet_arrival;
unsigned long lastSend = 0;
unsigned int sendDelay = 1000;

void messageReceived() {
    packet_arrival = true;
}

void setup() {
    Serial.begin(115200);
    radio.init();
    radio.setSyncWord(syncWord);
    radio.setCarrierFreq(CFREQ_433);
    radio.disableAddressCheck();
    radio.setChannel(0);
    radio.setTxPowerAmp(PA_LongDistance);

    bool install = true;
    Serial.print(F("CC1101_PARTNUM "));
    //Serial.println(radio.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
    byte partnum = radio.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER);
    Serial.println(partnum);
    if (partnum != 0) install = false;
    
    Serial.print(F("CC1101_VERSION "));
    //Serial.println(radio.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
    byte version = radio.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER);
    Serial.println(version);
    if (version != 20) install = false;

    Serial.print(F("CC1101_MARCSTATE "));
    Serial.println(radio.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);

    if (install) {
      Serial.println(F("CC1101 radio initialized."));
    } else {
      Serial.println(F("CC1101 radio not initialized."));
      while(1);
    }

    packet_arrival = false;
    attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
}

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

void loop() {
    static bool waiting_response = false;
    unsigned long startMillis;
    unsigned long now = millis();
    CCPACKET packet_sent;
    CCPACKET packet_recv;

    if (now > lastSend + sendDelay && waiting_response == false) {
        lastSend = now;
        packet_sent.length = sprintf((char *) packet_sent.data, "Hello World %ld", now);
        radio.sendData(packet_sent);
        Serial.println(F("Sent packet..."));
        packet_sent.data[packet_sent.length] = 0;
        waiting_response = true;
        startMillis = millis();
    }

    // Wait for a response from the other party 
    if (packet_arrival) {
        unsigned long diffMillis = now - startMillis;
        detachInterrupt(CC1101Interrupt);
        packet_arrival = false;
        if (radio.receiveData(&packet_recv) > 0) {
            Serial.println(F("Received packet..."));
            if (!packet_recv.crc_ok) {
                Serial.println(F("crc not ok"));
            } else {
                Serial.print("Responce time: ");
                Serial.println(diffMillis);
                if (packet_recv.length == packet_sent.length) {
                    packet_recv.data[packet_recv.length] = 0;
                    Serial.print(F("packet.length: "));
                    Serial.println(packet_recv.length);
                    Serial.print((const char *)packet_sent.data);
                    Serial.print(" --> ");
                    Serial.println((const char *)packet_recv.data);
                } else {
                    Serial.println(F("illegal receive packet length"));
                    Serial.print(F("packet_sent.length="));
                    Serial.println(packet_sent.length);
                    Serial.print(F("packet_recv.length="));
                    Serial.println(packet_recv.length);
                }
            }
            waiting_response = false;
        }
        attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
    } // end packet_arrival


    if (waiting_response) {
      unsigned long diffMillis = millis() - startMillis;
      if (diffMillis > 2000) {
        Serial.println("No responce from others");
        waiting_response = false;
      }
    }

}
