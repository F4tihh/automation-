const int relayPins[] = {14, 27, 26, 25, 33, 32, 18, 19};  // Röle giriş pinleri

void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // Röle başlangıçta kapalı (aktif düşük)
  }

  Serial.begin(115200);
}

void loop() {
  // Tüm röleleri sırayla aç/kapat
  for (int i = 0; i < 8; i++) {
    digitalWrite(relayPins[i], LOW);  // Röleyi aktif et (aktif düşük)
    Serial.printf("Relay %d ON\n", i + 1);
    delay(1000);
    digitalWrite(relayPins[i], HIGH);  // Röleyi pasif yap
    Serial.printf("Relay %d OFF\n", i + 1);
    delay(500);
  }
}