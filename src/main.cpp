#include <Wire.h>
#include <VL53L1X.h>
#include <MPU6050.h>

unsigned long prevmillis = 0;
const unsigned long measurementInterval = 1000; // 1 saniye ölçüm aralığı

VL53L1X sensor;
MPU6050 accelgyro;
int16_t ax, ay, az; // ivme tanımlama
int16_t gx, gy, gz; // gyro tanımlama

// Fonksiyon bildirimleri (önceden tanıtılması gerekiyor)
void geriGit(int stepSayisi);
void ileriGit(int stepSayisi);
void geriGitKontrollu(int stepSayisi);
bool isAnyInputHigh();

// CWD556 sürücü pin bağlantıları
const int pullPin = 17;
const int dirPin = 18;
const int enablePin = 5;
const int stepsPerRevolution = 1600;

// Giriş pinleri
const int inputPins[] = {2, 15, 4, 16};

const int pirPin = 35;
const int audioPin = 19;
int audioValue = 0;

// Röle pinleri
const int motorRolePin = 32;
const int motorRolePin1 = 33;
const int motorRolePin3 = 26;
const int motorRolePin7 = 13;
const int motorRolePin2 = 27;

void setup() {
  Wire.begin(21, 22);
  Wire.setClock(400000);
  sensor.setTimeout(50);

  pinMode(pirPin, INPUT);
  pinMode(audioPin, INPUT);
  pinMode(pullPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  pinMode(motorRolePin, OUTPUT);
  pinMode(motorRolePin1, OUTPUT);
  pinMode(motorRolePin2, OUTPUT);
  pinMode(motorRolePin3, OUTPUT);
  pinMode(motorRolePin7, OUTPUT);

  digitalWrite(enablePin, HIGH);

  for (int i = 0; i < sizeof(inputPins) / sizeof(inputPins[0]); i++) {
    pinMode(inputPins[i], INPUT);
  }

  accelgyro.initialize();

  if (!sensor.init(false)) {
    Serial.println("Sensör tespit edilemedi veya başlatılamadı!");
  } else {
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(50);
  }

  Serial.begin(115200);
  while (!Serial) {}
}

void loop() {
  if ((millis() - prevmillis) >= measurementInterval) {
    prevmillis = millis();

    int pirValue = digitalRead(pirPin);
    int audioValue = digitalRead(audioPin);
    long vl53l1x = sensor.read();

    Serial.print("Entfernung VL53L1X: ");
    Serial.print(vl53l1x);
    Serial.println(" mm");

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    delay(700);

    if (digitalRead(inputPins[3]) == HIGH || audioValue == HIGH || pirValue == HIGH) {
      digitalWrite(motorRolePin2, HIGH);
      geriGit(3 * stepsPerRevolution);
      delay(21000);
      ileriGit(3.2 * stepsPerRevolution);
      delay(3000);
      digitalWrite(motorRolePin2, LOW);
      digitalWrite(enablePin, HIGH);

    } else if (digitalRead(inputPins[2]) == HIGH) {
      digitalWrite(motorRolePin2, HIGH);
      geriGit(3 * stepsPerRevolution);
      delay(14000);
      ileriGit(3.2 * stepsPerRevolution);
      delay(3000);
      digitalWrite(motorRolePin2, LOW);
      digitalWrite(enablePin, HIGH);

    } else if (digitalRead(inputPins[1]) == HIGH) {
      digitalWrite(motorRolePin2, HIGH);
      geriGit(3 * stepsPerRevolution);
      delay(7000);
      ileriGit(3.2 * stepsPerRevolution);
      delay(3000);
      digitalWrite(motorRolePin2, LOW);
      digitalWrite(enablePin, HIGH);

    } else if (digitalRead(inputPins[0]) == HIGH) {
      digitalWrite(motorRolePin2, HIGH);
      geriGit(3 * stepsPerRevolution);
      delay(1000);

      while (true) {
        if (digitalRead(inputPins[0]) == HIGH) {
          ileriGit(3.2 * stepsPerRevolution);
          break;
        }
      }

      digitalWrite(motorRolePin2, LOW);
    }

    // Gyro kontrolü
    if (gy > 300) {
      digitalWrite(motorRolePin, HIGH);
      geriGit(3 * stepsPerRevolution);
      delay(1000);
      digitalWrite(enablePin, HIGH);
      digitalWrite(motorRolePin, LOW);
    }
  }
}

void geriGit(int stepSayisi) {
  digitalWrite(dirPin, HIGH);
  digitalWrite(enablePin, LOW);
  digitalWrite(motorRolePin, HIGH);
  delay(500);
  digitalWrite(motorRolePin, LOW);

  for (int i = 0; i < stepSayisi; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
  delay(100);
}

void ileriGit(int stepSayisi) {
  digitalWrite(dirPin, LOW);
  digitalWrite(enablePin, LOW);
  digitalWrite(motorRolePin, HIGH);
  delay(500);
  digitalWrite(motorRolePin, LOW);

  for (int i = 0; i < stepSayisi; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
}

void geriGitKontrollu(int stepSayisi) {
  digitalWrite(dirPin, HIGH);
  digitalWrite(enablePin, LOW);
  digitalWrite(motorRolePin, HIGH);
  delay(500);

  int totalAdim = stepSayisi;
  bool devamEt = true;

  while (devamEt) {
    for (int i = 0; i < totalAdim; i++) {
      digitalWrite(pullPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(pullPin, LOW);
      delayMicroseconds(500);

      if (i % (int)(1.1 * stepsPerRevolution) == 0) {
        delay(1000);
        long vl53l1x = sensor.read();
        delay(500);

        if (vl53l1x < 1500) {
          digitalWrite(motorRolePin1, HIGH);
          digitalWrite(motorRolePin2, HIGH);
          digitalWrite(motorRolePin3, HIGH);
          digitalWrite(motorRolePin7, HIGH);

          ileriGit(1.1 * stepsPerRevolution);

          digitalWrite(motorRolePin1, LOW);
          digitalWrite(motorRolePin2, LOW);
          digitalWrite(motorRolePin3, LOW);
          digitalWrite(motorRolePin7, LOW);

          i = -1;
          break;
        }
      }

      if (i == totalAdim - 1) {
        devamEt = false;
      }
    }
  }

  delay(100);
  digitalWrite(motorRolePin, LOW);
}

bool isAnyInputHigh() {
  for (int i = 0; i < sizeof(inputPins) / sizeof(inputPins[0]); i++) {
    if (digitalRead(inputPins[i]) == HIGH) {
      return true;
    }
  }
  return false; 
}
