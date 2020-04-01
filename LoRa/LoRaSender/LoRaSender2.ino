#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_BMP085.h>


#ifndef ESP8266
#include <LowPower.h>

#endif

const long freq = 868E6;
const int SF = 9;
const long bw = 125E3;
long zeroAltitude;

int counter = 1, messageLostCounter = 0;
//#define BMP
// #define INTER


void setup() {
  Serial.begin(9600);
  while (!Serial);

#ifdef ESP8266
  LoRa.setPins(16, 17, 15); // set CS, reset, IRQ pin
#else
  LoRa.setPins(10, A0, 2); // set CS, reset, IRQ pin
#endif

  Serial.println("LoRa Sender");

  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(SF);
  //  LoRa.setSignalBandwidth(bw);

  Serial.print("Frequency ");
  Serial.print(freq);
  Serial.print(" Bandwidth ");
  Serial.print(bw);
  Serial.print(" SF ");
  Serial.println(SF);
}

void loop() {
  char message[90];
  sprintf(message, "{\"Vcc\":\"\",\"Count\":\"%03d\",\"Lost\":\"%03d\",xxx}", counter, messageLostCounter);

  int nackCounter = 0;
  while (!receiveAck(message) && nackCounter <= 5) {

    Serial.println(" refused ");
    Serial.print(nackCounter);
    LoRa.sleep();
    delay(1000);
    sendMessage(message);
    nackCounter++;
  }

  if (nackCounter >= 5) {
    Serial.println("");
    Serial.println("--------------- MESSAGE LOST ---------------------");
    messageLostCounter++;
    delay(100);
  } else {
    Serial.println("Acknowledged ");
  }
  counter++;
  LoRa.sleep();
  //LoRa.idle();

#ifdef ESP8266
  delay(8000);
#else
  Serial.println("Falling asleep");
  delay(100);

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  delay(8000);
#endif
}

bool receiveAck(String message) {
  String ack;
  Serial.print(" Waiting for Ack ");
  bool stat = false;
  unsigned long entry = millis();
  while (stat == false && millis() - entry < 2000) {
    if (LoRa.parsePacket()) {
      ack = "";
      while (LoRa.available()) {
        ack = ack + ((char)LoRa.read());
      }
      int check = 0;
      // Serial.print("///");
      for (int i = 0; i < message.length(); i++) {
        check += message[i];
        //   Serial.print(message[i]);
      }
      /*    Serial.print("/// ");
          Serial.println(check);
          Serial.print(message);*/
      Serial.print(" Ack ");
      Serial.print(ack);
      Serial.print(" Check ");
      Serial.print(check);
      if (ack.toInt() == check) {
        Serial.print(" Checksum OK ");
        stat = true;
      }
    }
  }
  return stat;
}

void sendMessage(String message) {
  Serial.print(message);
  // send packet
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

void wakeUp()
{
  delay(100);
  Serial.println("wakeup");
  delay(100);
}
