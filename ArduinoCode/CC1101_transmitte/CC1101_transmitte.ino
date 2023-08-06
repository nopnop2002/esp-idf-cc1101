#include <Arduino.h>
#include <cc1101.h> // https://github.com/veonik/arduino-cc1101
#include <ccpacket.h>

// Attach CC1101 pins to their corresponding SPI pins
// Uno pins:
// CSN (SS) => 10
// MOSI => 11
// MISO => 12
// SCK => 13
// GD0 => Not used in Transmitter

CC1101 radio;

byte syncWord[2] = {199, 10};

unsigned long lastSend = 0;
unsigned int sendDelay = 1000;

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
}

void loop() {
    unsigned long now = millis();
    if (now > lastSend + sendDelay) {
        lastSend = now;
        CCPACKET packet;
        packet.length = sprintf((char *) packet.data, "Hello World %ld", now);
        radio.sendData(packet);
        Serial.print(F("Sent packet. length="));
        Serial.println(packet.length);
    }
}
