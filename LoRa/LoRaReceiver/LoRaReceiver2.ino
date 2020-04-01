#include <SPI.h>
#include <LoRa.h>


#define ESP8266  // comment if you want to use the sketch on a Arduino board
#define OLED        // comment if you do nto have a OLED display

const long freq = 868E6;
const int SF = 9;
const long bw = 125E3;


#ifdef OLED
#include "SSD1306.h"
SSD1306  display(0x3d, 4, 5);
#endif

int counter, lastCounter;

void setup() {
  Serial.begin(9600);
  Serial.println("LoRa Receiver");
#ifdef ESP8266
  LoRa.setPins(16, 17, 15); // set CS, reset, IRQ pin
#else
  LoRa.setPins(10, A0, 2); // set CS, reset, IRQ pin
#endif
#ifdef OLED
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 10, "Mailbox");
  display.display();
#endif

  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");

    while (1);
  }

  LoRa.setSpreadingFactor(SF);
  // LoRa.setSignalBandwidth(bw);
  Serial.println("LoRa Started");

  Serial.print("Frequency ");
  Serial.print(freq);
  Serial.print(" Bandwidth ");
  Serial.print(bw);
  Serial.print(" SF ");
  Serial.println(SF);
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    String message = "";
    while (LoRa.available()) {
      message = message + ((char)LoRa.read());
    }
    String rssi = "\"RSSI\":\"" + String(LoRa.packetRssi()) + "\"";
    String jsonString = message;
    jsonString.replace("xxx", rssi);

    int ii = jsonString.indexOf("Count", 1);
    String count = jsonString.substring(ii + 8, ii + 11);
    counter = count.toInt();
    if (counter - lastCounter == 0) Serial.println("Repetition");
    lastCounter = counter;


    sendAck(message);
    String value1 = jsonString.substring(8, 11);  // Vcc or heighth
    String value2 = jsonString.substring(23, 26); //counter
#ifdef OLED
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    displayText(40, 0, value1, 3);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    displayText(120, 0, String(LoRa.packetRssi()), 3);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    displayText(60, 35, count, 3);
    display.display();
#endif

  }
}

void sendAck(String message) {
  int check = 0;
  for (int i = 0; i < message.length(); i++) {
    check += message[i];
  }
  // Serial.print("/// ");
  LoRa.beginPacket();
  LoRa.print(String(check));
  LoRa.endPacket();
  Serial.print(message);
  Serial.print(" ");
  Serial.print("Ack Sent: ");
  Serial.println(check);
}

#ifdef OLED
void displayText(int x, int y, String tex, byte font ) {
  switch (font) {
    case 1:
      display.setFont(ArialMT_Plain_10);
      break;
    case 2:
      display.setFont(ArialMT_Plain_16);
      break;
    case 3:
      display.setFont(ArialMT_Plain_24);
      break;
    default:
      display.setFont(ArialMT_Plain_16);
      break;
  }
  display.drawString(x, y, tex);
}
#endif
