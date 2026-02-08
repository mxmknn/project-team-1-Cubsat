#include <Servo.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

// Пины
#define RF_PIN_CE    4
#define RF_PIN_CSN   5
#define SERVO_PIN_Y  7     // горизонт
#define SERVO_PIN_X  8     // вертик
#define LASER_PIN    6

// Тайминги
#define LASER_PULSE_MS 3000

RF24 nrf(RF_PIN_CE, RF_PIN_CSN);

// Адреса
const byte PIPE_RX[6] = "CMD01";
const byte PIPE_TX[6] = "TEL01";

// Телеметрия
struct Packet {
  uint8_t mode;
  int8_t  yaw;
  int8_t  pitch;
} pkt;

Servo sYaw, sPitch;
bool armed = false;

void txPacket(uint8_t mode, int8_t yaw, int8_t pitch) {
  pkt.mode  = mode;
  pkt.yaw   = yaw;
  pkt.pitch = pitch;

  digitalWrite(LASER_PIN, HIGH);
  nrf.write(&pkt, sizeof(pkt));
  delay(LASER_PULSE_MS);
  digitalWrite(LASER_PIN, LOW);
}

void dumpPacket(const Packet &p) {
  Serial.print("Telemetry { state=");
  Serial.print(p.mode);
  Serial.print(", h=");
  Serial.print(p.yaw);
  Serial.print(" deg, v=");
  Serial.print(p.pitch);
  Serial.println(" deg }");
}

void setup() {
  Serial.begin(9600);

  nrf.begin();
  nrf.setPALevel(RF24_PA_LOW);
  nrf.setDataRate(RF24_1MBPS);
  nrf.openReadingPipe(1, PIPE_RX);
  nrf.openWritingPipe(PIPE_TX);
  nrf.startListening();

  pinMode(LASER_PIN, OUTPUT);

  sYaw.attach(SERVO_PIN_Y);
  sPitch.attach(SERVO_PIN_X);

  sYaw.write(90);
  sPitch.write(90);
}

void loop() {
  if (!armed) {
    if (nrf.available()) {
      char buf[6] = {0};
      nrf.read(buf, sizeof(buf));
      if (!strcmp(buf, "start")) {
        armed = true;
        nrf.stopListening();
      }
    }
    return;
  }

  // 1) Горизонт
  for (int8_t yaw = -40; yaw <= 40; yaw += 10) {
    sYaw.write(90 + yaw);
    sPitch.write(90);
    txPacket(1, yaw, 0);
    dumpPacket(pkt);
  }
  sYaw.write(90);

  // 2) Вертик
  for (int8_t pitch = -40; pitch <= 40; pitch += 10) {
    sPitch.write(90 + pitch);
    sYaw.write(90);
    txPacket(2, 0, pitch);
  }
  sPitch.write(90);

  // 3) Диаг +
  for (int8_t a = -40; a <= 40; a += 10) {
    sYaw.write(90 + a);
    sPitch.write(90 + a);
    txPacket(3, a, a);
  }

  // 4) Диаг -
  for (int8_t a = -40; a <= 40; a += 10) {
    sYaw.write(90 + a);
    sPitch.write(90 - a);
    txPacket(4, a, -a);
  }

  sYaw.write(90);
  sPitch.write(90);

  armed = false;
  nrf.startListening();
  armed = false;
}
