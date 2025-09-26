#include <Wire.h>
#include <VL53L1X.h>
#include <MPU6050.h>

#define BUTTON_PIN 4     // 8. pin buna bağlı
#define RELAY_PIN  2     // role kontrol pini

unsigned long prevmillis = 0;
const unsigned long measurementInterval = 1000; // 1 saniye ölçüm aralığı

VL53L1X sensor;
MPU6050 accelgyro;
int16_t ax, ay, az; // ivme tanımlama
int16_t gx, gy, gz; // gyro tanımlama

// CWD556 sürücü pin bağlantıları
const int pullPin = 34;    // PULL pini
const int dirPin = 35;     // Yön pini
const int enablePin = 32;   // Enable pini
const int stepsPerRevolution = 1600; // Step motor adım sayısı (pulse/rev)

// Giriş pinleri
const int inputPins[] = {16, 17, 18, 19  };


void setup() {
  Wire.begin(21, 22);
  Wire.setClock(400000); // 400 kHz I2C hızı
  sensor.setTimeout(50);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Dahili pull-up kullan
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);       // Başlangıçta role kapalı

  pinMode(pullPin, OUTPUT);   // PULL pini çıkış olarak ayarla
  pinMode(dirPin, OUTPUT);    // Yön pini çıkış olarak ayarla
  pinMode(enablePin, OUTPUT); // Enable pini çıkış olarak ayarla


  digitalWrite(enablePin, HIGH); // Step motoru etkinleştir

  // Giriş pinlerini giriş olarak ayarla
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


    long vl53l1x = sensor.read();
    Serial.print("Entfernung VL53L1X: ");
    Serial.print(vl53l1x);
    Serial.println(" mm");

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // ivme ve gyro değerlerini okuma
    delay(700); // 0.7 saniye dur

    if (digitalRead(inputPins[3]) == HIGH || vl53l1x < 1000 ||  digitalRead(BUTTON_PIN) == LOW ) {  //vl53l1x < 1000 || 

    digitalWrite(RELAY_PIN, LOW);   // Roleyi kapat
    delay(1000);
    digitalWrite(RELAY_PIN, HIGH);   // Roleyi kapat
    
    geriGit(3 * stepsPerRevolution); // İleri gitme fonksiyonu
    delay(21000); // 7 saniye dur
    ileriGit(3.2 * stepsPerRevolution); // Geri gitme fonksiyonunu kontrolle
    delay(3000); // 3 saniye dur

  

    digitalWrite(enablePin, HIGH); // Step motoru devre dışı bırak
      
    } else if (digitalRead(inputPins[2]) == HIGH) {
         //digitalWrite(motorRolePin2, HIGH); // Motor role pini pasif

        // İkinci durumda işlemler
        geriGit(3 * stepsPerRevolution); // İleri gitme fonksiyonu
        delay(14000); // 14 saniye dur
        ileriGit(3.2 * stepsPerRevolution); // Geri gitme fonksiyonunu kontrolle
        delay(3000); // 3 saniye dur


        digitalWrite(enablePin, HIGH); // Step motoru devre dışı bırak
        // Diğer giriş pinleri için benzer şekilde işlemleri devam ettirebilirsiniz
    } else if (digitalRead(inputPins[1]) == HIGH) {
        // Üçüncü durumda işlemler


        geriGit(3 * stepsPerRevolution); // İleri gitme fonksiyonu
        delay(7000); // 21 saniye dur
        ileriGit(3.2 * stepsPerRevolution); // Geri gitme fonksiyonunu kontrolle
        delay(3000); // 3 saniye dur


        digitalWrite(enablePin, HIGH); // Step motoru devre dışı bırak
    } else if (digitalRead(inputPins[0]) == HIGH) {
        // Dördüncü durumda işlemler


        geriGit(3* stepsPerRevolution); // İleri gitme fonksiyonu
        delay(1000);
         while (true) {
            if (digitalRead(inputPins[0]) == HIGH) {
                ileriGit(3.2 * stepsPerRevolution);
                break; // İkinci butona basıldı, iç içe döngüden çık
            }
          }

  
    }
    //------------gyro---------------
    if ( gy > 300) { 
      
      geriGit(3 * stepsPerRevolution); // İleri gitme fonksiyonu
      delay(1000); // .. saniye dur
      digitalWrite(enablePin, HIGH); // Step motoru devre dışı bırak

    }

  }
}

void geriGit(int stepSayisi) {
  digitalWrite(dirPin, HIGH);   // Geri yön
  digitalWrite(enablePin, LOW); // Step motoru etkinleştir
  delay(500);


  for (int i = 0; i < stepSayisi; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500); // Uygun adım aralığı
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
  delay(100);
  
}

void ileriGit(int stepSayisi) {
  digitalWrite(dirPin, LOW);   // İleri yön
  digitalWrite(enablePin, LOW); // Step motoru etkinleştir
  delay(500);
  //digitalWrite(motorRolePin, LOW);
  for (int i = 0; i < stepSayisi; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500); // Uygun adım aralığı
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
    
}


bool isAnyInputHigh() {
  for (int i = 0; i < sizeof(inputPins) / sizeof(inputPins[0]); i++) {
    if (digitalRead(inputPins[i]) == HIGH) {
      return true;
    }
  }
  return false;
}