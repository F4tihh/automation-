#include <Wire.h>
#include <MPU6050.h>

#define BUTTON_PIN 4       // Buton girişi
#define RELAY_PIN  2       // Röle kontrol pini
#define SWITCH_PIN 26      // Kapı kapanma switch'i
#define PIR_PIN 34         // PIR sensör pini

// CWD556 step sürücü pinleri
const int pullPin = 27;
const int dirPin = 14;
const int enablePin = 13;
const int stepsPerRevolution = 1600;

// Diğer giriş pinleri (manuel buton vs.)
const int inputPins[] = {16, 17, 18, 19};

MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

void setup() {
  Wire.begin(21, 22);
  Wire.setClock(400000);

  Serial.begin(115200);
  while (!Serial) {}

  accelgyro.initialize();

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Röle başlangıçta kapalı

  pinMode(pullPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);  // Motor başlangıçta devre dışı

  // input pinlerini INPUT olarak ayarla
  for (int i = 0; i < sizeof(inputPins) / sizeof(inputPins[0]); i++) {
    pinMode(inputPins[i], INPUT);
  }
}

void loop() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  delay(700);

  // 1. Durum: Buton, PIR ya da inputPins[3] tetiklerse
  if (digitalRead(inputPins[3]) == HIGH || digitalRead(PIR_PIN) == HIGH || digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("PIR/BUTON/input3 tetiklendi");

    digitalWrite(RELAY_PIN, LOW);
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);

    geriGit(stepsPerRevolution);
    delay(3000);
    ileriGit(stepsPerRevolution);
    delay(3000);

    digitalWrite(enablePin, HIGH); // Motoru devre dışı bırak
  }

  // 2. inputPins[2]
  else if (digitalRead(inputPins[2]) == HIGH) {
    Serial.println("input2 tetiklendi");

    digitalWrite(RELAY_PIN, LOW);
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);

    geriGit(stepsPerRevolution);
    delay(3000);
    ileriGit(stepsPerRevolution);
    delay(3000);

    digitalWrite(enablePin, HIGH);
  }

  // 3. inputPins[1]
  else if (digitalRead(inputPins[1]) == HIGH) {
    Serial.println("input1 tetiklendi");

    digitalWrite(RELAY_PIN, LOW);
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);

    geriGit(stepsPerRevolution);
    delay(3000);
    ileriGit(stepsPerRevolution);
    delay(3000);

    digitalWrite(enablePin, HIGH);
  }

  // 4. inputPins[0]
  else if (digitalRead(inputPins[0]) == HIGH) {
    Serial.println("input0 tetiklendi");

    digitalWrite(RELAY_PIN, LOW);
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);

    geriGit(stepsPerRevolution);
    delay(1000);

    // input0 hala HIGH ise bekle
    while (true) {
      if (digitalRead(inputPins[0]) == HIGH) {
        ileriGit(2 * stepsPerRevolution);
        break;
      }
    }
  }

  // 5. Gyro hareket kontrolü
  if (abs(gy) > 500) {
    Serial.println("Gyro hareketi algılandı!");

    digitalWrite(RELAY_PIN, LOW);
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);

    geriGit(stepsPerRevolution);
    delay(1000);
    digitalWrite(enablePin, HIGH);
  }
}

void geriGit(int stepSayisi) {
  digitalWrite(dirPin, LOW);
  digitalWrite(enablePin, LOW);
  delay(500);

  for (int i = 0; i < stepSayisi; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }

  digitalWrite(enablePin, HIGH);
}

void ileriGit(int stepSayisi) {
  digitalWrite(dirPin, HIGH);
  digitalWrite(enablePin, LOW);
  delay(500);

  for (int i = 0; i < stepSayisi; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }

  digitalWrite(enablePin, HIGH);
  delay(100);
}
