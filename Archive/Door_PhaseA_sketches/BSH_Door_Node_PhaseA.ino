// BSH-Door node -- Phase A standalone sketch (no WiFi, no Modbus)
// Hardware: ESP32, RC522 (SPI), SG90 servo on external 5V, common-cathode RGB LED,
//           IR sensor module, reed switch module (S/V/G).
//
// SET THESE TWO BEFORE FLASHING (read the raw values the sketch prints):
//   IR_ACTIVE_LOW:        true  if the IR sensor reads LOW  with your hand in front of it
//                         false if it reads HIGH with your hand in front of it
//   REED_LOW_MEANS_CLOSED: true  if the reed module reads LOW  with the magnet touching it
//                         false if it reads HIGH with the magnet touching it
//   (magnet near the module = door closed)

#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ---- Pins ----
#define PIN_SERVO      13
#define PIN_RGB_RED    25
#define PIN_RGB_GREEN  26
#define PIN_RGB_BLUE   27
#define PIN_IR_SENSOR  14   // if GPIO14 is not on your board, use 4 (NOT 27, blue LED is there)
#define PIN_REED       34   // reed module S pin; GPIO34 is input-only
#define PIN_RFID_SS    32
#define PIN_RFID_RST   33
#define PIN_SPI_SCK    18
#define PIN_SPI_MISO   19
#define PIN_SPI_MOSI   23

const bool IR_ACTIVE_LOW = true;
const bool REED_LOW_MEANS_CLOSED = true;

// ---- Authorized cards ----
byte AUTHORIZED_CARDS[][4] = {
  {0xB3, 0x12, 0x87, 0x2D}   // BSH card
};
const int NUM_AUTHORIZED_CARDS = 1;

MFRC522 mfrc522(PIN_RFID_SS, PIN_RFID_RST);
Servo servo;

unsigned long lastPrint = 0;

bool isAuthorized(byte *uid, byte size) {
  if (size != 4) return false;
  for (int i = 0; i < NUM_AUTHORIZED_CARDS; i++) {
    bool match = true;
    for (byte j = 0; j < 4; j++) {
      if (uid[j] != AUTHORIZED_CARDS[i][j]) { match = false; break; }
    }
    if (match) return true;
  }
  return false;
}

bool readDoorPositionOpen() {
  bool raw = digitalRead(PIN_REED);
  return REED_LOW_MEANS_CLOSED ? (raw == HIGH) : (raw == LOW);   // open = magnet away
}

bool readIrDetected() {
  bool raw = digitalRead(PIN_IR_SENSOR);
  return IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);
}

// Green = gate closed and idle, red = gate open or moving or card denied. Blue stays off.
void showClosed() {
  digitalWrite(PIN_RGB_RED, LOW);
  digitalWrite(PIN_RGB_GREEN, HIGH);
  digitalWrite(PIN_RGB_BLUE, LOW);
}

void showOpen() {
  digitalWrite(PIN_RGB_RED, HIGH);
  digitalWrite(PIN_RGB_GREEN, LOW);
  digitalWrite(PIN_RGB_BLUE, LOW);
}

void door_open() {
  showOpen();
  servo.attach(PIN_SERVO);
  for (int angle = 0; angle <= 90; angle += 5) {
    servo.write(angle);
    delay(25);
  }
  delay(2000);
  for (int angle = 90; angle >= 0; angle -= 5) {
    servo.write(angle);
    delay(25);
  }
  servo.detach();
  showClosed();
}

void setup() {
  Serial.begin(115200);

  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_RFID_SS);
  mfrc522.PCD_Init();

  servo.attach(PIN_SERVO);
  servo.write(0);
  servo.detach();

  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  pinMode(PIN_REED, INPUT);        // INPUT, not INPUT_PULLUP, on GPIO34
  pinMode(PIN_IR_SENSOR, INPUT);

  showClosed();
  Serial.println("BSH-Door ready. Badge a card...");
}

void loop() {
  // Sensor report twice a second, with raw pin values so polarity is easy to check
  if (millis() - lastPrint >= 500) {
    lastPrint = millis();
    Serial.print(F("Reed raw="));
    Serial.print(digitalRead(PIN_REED));
    Serial.print(F(" -> DoorPositionOpen="));
    Serial.print(readDoorPositionOpen());
    Serial.print(F("   IR raw="));
    Serial.print(digitalRead(PIN_IR_SENSOR));
    Serial.print(F(" -> IRDetected="));
    Serial.println(readIrDetected());
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" ");
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  if (isAuthorized(mfrc522.uid.uidByte, mfrc522.uid.size)) {
    Serial.println(F("Access granted"));
    door_open();
  } else {
    Serial.println(F("Access denied"));
    showOpen();       // red for 1 s, then back to green
    delay(1000);
    showClosed();
  }

  mfrc522.PICC_HaltA();   // stop talking to this card so it is not re-read
}
