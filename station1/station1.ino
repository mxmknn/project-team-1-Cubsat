#include <SPI.h>//подключение библиотек
#include <nRF24L01.h>
#include <RF24.h>

#define CE 9
#define CSN 10

RF24 rfLink(CE, CSN);//радио

const byte PIPE_TX[6] = "CMD01";//отправка
const byte PIPE_RX[6] = "TEL01"; //прием

struct Packet {
  uint8_t mode;
  int8_t  yaw;
  int8_t  pitch;
} pkt;

void setup() {
  Serial.begin(9600);
  rfLink.begin();
  rfLink.setPALevel(RF24_PA_LOW);
  rfLink.setDataRate(RF24_1MBPS);
  rfLink.openWritingPipe(PIPE_TX);
  rfLink.openReadingPipe(1, PIPE_RX);
  rfLink.startListening();

  Serial.println("Type: + for start");
}
// start
void loop() {
  if (Serial.available()) {
    char inputChar = Serial.read();
    if (inputChar == '+') {
      char startMsg[6] = "start";
      rfLink.stopListening();
      rfLink.write(startMsg, sizeof(startMsg));
      rfLink.startListening();
      Serial.println("sent start");
    }
  }

  rfLink.startListening();
  if (rfLink.available()) { // телеметрия прием
    rfLink.read(&pkt, sizeof(pkt));
    Serial.print("state="); Serial.print(pkt.mode);
    Serial.print(" h=");    Serial.print(pkt.yaw);
    Serial.print(" v=");    Serial.println(pkt.pitch);
  }
}