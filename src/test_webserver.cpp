#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6050.h>

const char* ssid = "YYYYYYYYYY";
const char* password = "XXXXXX";

WebServer server(80);

// Motor bağlantıları
const int pullPin = 27;
const int dirPin = 14;
const int enablePin = 13;
const int stepsPerRevolution = 1600; // 1 tur = 1600 adım

MPU6050 accelgyro;

// Web üzerinden tetiklenecek bayrak
volatile bool openCommand = false;
volatile bool closeCommand = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("WiFi bağlanıyor");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nBağlandı: " + WiFi.localIP().toString());

  Wire.begin(21, 22);
  accelgyro.initialize();

  pinMode(pullPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  // Web sunucusu yönlendirmeleri
  server.on("/", handleRoot);
  server.on("/open", []() {
    openCommand = true;
    server.send(200, "text/html", "<h1>Kapı açılıyor</h1><a href='/'>Geri dön</a>");
  });
  server.on("/close", []() {
    closeCommand = true;
    server.send(200, "text/html", "<h1>Kapı kapanıyor</h1><a href='/'>Geri dön</a>");
  });

  server.begin();
  Serial.println("Web sunucusu başlatıldı");
}

void loop() {
  server.handleClient();

  if (openCommand) {
    openCommand = false;
    Serial.println("WEB: Kapı açılıyor...");
    ileriGit(3 * stepsPerRevolution);
    delay(500);
    geriGit(3 * stepsPerRevolution);
    delay(500);
  }

  if (closeCommand) {
    closeCommand = false;
    Serial.println("WEB: Kapı kapanıyor...");
    geriGit(3 * stepsPerRevolution);
    delay(500);
  }
}

void ileriGit(int adim) {
  digitalWrite(dirPin, HIGH); // İleri yön
  digitalWrite(enablePin, LOW);
  delay(100);

  for (int i = 0; i < adim; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(enablePin, HIGH);
}

void geriGit(int adim) {
  digitalWrite(dirPin, LOW); // Geri yön
  digitalWrite(enablePin, LOW);
  delay(100);

  for (int i = 0; i < adim; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(enablePin, HIGH);
}

void handleRoot() {
  String html = "<h1>Kapı Kontrol Paneli</h1>";
  html += "<button onclick=\"location.href='/open'\">Kapıyı Aç</button><br><br>";
  html += "<button onclick=\"location.href='/close'\">Kapıyı Kapat</button>";
  server.send(200, "text/html", html);
}
