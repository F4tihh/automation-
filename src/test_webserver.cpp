#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6050.h>

// ======= Wi-Fi Ayarları =======
const char* ssid     = "YYYYYYYYYY";
const char* password = "XXXXXX";

// ======= Web Sunucusu =======
WebServer server(80);

// ======= Motor Bağlantıları =======
const int pullPin = 27;
const int dirPin  = 14;
const int enablePin = 13;
const int stepsPerRevolution = 1600; // 1 tur = 1600 adım

MPU6050 accelgyro;

// Web üzerinden tetiklenecek bayraklar
volatile bool openCommand  = false;
volatile bool closeCommand = false;

String lastAction = "Hazır";    // Arayüzde gösterilecek durum metni
bool   isBusy     = false;      // İşlem sırasında butonları kilitle

// ======= İleri/Geri Fonksiyonları =======
void ileriGit(int adim) {
  digitalWrite(dirPin, HIGH);   // İleri yön
  digitalWrite(enablePin, LOW); // Sürücüyü etkinleştir
  delay(100);

  for (int i = 0; i < adim; i++) {
    digitalWrite(pullPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pullPin, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(enablePin, HIGH); // Sürücüyü devre dışı
}

void geriGit(int adim) {
  digitalWrite(dirPin, LOW);    // Geri yön
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

// ======= HTTP Handler'lar =======
void handleRoot() {
  // UTF-8 başlık
  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=utf-8", "");

  // HTML: raw literal ile daha temiz
  const char page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Kapı Kontrol Paneli</title>
  <style>
    :root { --bg:#0f172a; --card:#111827; --txt:#e5e7eb; --muted:#94a3b8; --accent:#22c55e; --danger:#ef4444; --btn:#1f2937; }
    * { box-sizing:border-box; }
    body { margin:0; font-family:system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif; background:var(--bg); color:var(--txt); }
    .wrap { max-width:680px; margin:40px auto; padding:16px; }
    .card { background:linear-gradient(180deg,#111827, #0b1220); border:1px solid #243244; border-radius:16px; padding:24px; box-shadow:0 10px 30px rgba(0,0,0,.35); }
    h1 { margin:0 0 8px; font-size:1.5rem; }
    p.subtitle { margin:0 0 20px; color:var(--muted); }
    .grid { display:grid; gap:12px; grid-template-columns:1fr 1fr; }
    .btn {
      appearance:none; border:1px solid #334155; background:var(--btn);
      color:var(--txt); padding:14px 16px; border-radius:12px; font-weight:600; cursor:pointer;
      transition:.15s transform ease, .2s background ease, .2s border ease, .2s opacity ease;
      user-select:none;
    }
    .btn:hover { transform:translateY(-1px); }
    .btn:active { transform:translateY(0); }
    .btn-accent { border-color:#134e4a; }
    .btn-danger { border-color:#7f1d1d; }
    .badge { display:inline-block; padding:6px 10px; border-radius:999px; background:#0b1324; border:1px solid #23324a; color:var(--muted); }
    .status { margin-top:16px; display:flex; align-items:center; gap:10px; }
    .dot { width:10px; height:10px; border-radius:50%; background:var(--accent); box-shadow:0 0 10px var(--accent); }
    .dot.busy { background:#f59e0b; box-shadow:0 0 10px #f59e0b; }
    .dot.err { background:var(--danger); box-shadow:0 0 10px var(--danger); }
    footer { margin-top:18px; color:var(--muted); font-size:.9rem; }
    .row { display:flex; gap:12px; align-items:center; flex-wrap:wrap; }
    @media (max-width:520px) { .grid { grid-template-columns:1fr; } }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>Kapı Kontrol Paneli</h1>
      <p class="subtitle">Aşağıdaki butonlarla kapıyı açıp kapatabilirsiniz. Durum anlık güncellenir.</p>

      <div class="grid">
        <button id="btnOpen"  class="btn btn-accent">🔓 Kapıyı Aç</button>
        <button id="btnClose" class="btn btn-danger">🔒 Kapıyı Kapat</button>
      </div>

      <div class="status">
        <span id="dot" class="dot"></span>
        <span id="state" class="badge">Durum: Bağlanıyor…</span>
      </div>

      <footer>
        IP: <span id="ip">%IP%</span> • Son işlem: <span id="last">%LAST%</span>
      </footer>
    </div>
  </div>

  <script>
    const btnOpen  = document.getElementById('btnOpen');
    const btnClose = document.getElementById('btnClose');
    const dot   = document.getElementById('dot');
    const state = document.getElementById('state');
    const last  = document.getElementById('last');

    let busy = false;

    function setBusy(v) {
      busy = v;
      btnOpen.disabled  = v;
      btnClose.disabled = v;
      dot.className = 'dot ' + (v ? 'busy' : '');
      state.textContent = v ? 'Durum: Çalışıyor…' : 'Durum: Hazır';
    }

    async function call(path) {
      try {
        setBusy(true);
        const r = await fetch(path, { method: 'POST' });
        const j = await r.json();
        if (!j.ok) throw new Error('İşlem başarısız');
        last.textContent = j.lastAction || '—';
      } catch (e) {
        dot.className = 'dot err';
        state.textContent = 'Durum: Hata';
        console.error(e);
      } finally {
        // statüyü tazele
        setTimeout(refreshStatus, 300);
        setTimeout(()=>setBusy(false), 400);
      }
    }

    async function refreshStatus() {
      try {
        const r = await fetch('/api/status');
        const j = await r.json();
        last.textContent = j.lastAction || '—';
        if (j.isBusy) setBusy(true); else setBusy(false);
      } catch (e) {
        dot.className = 'dot err';
        state.textContent = 'Durum: Bağlantı Hatası';
      }
    }

    btnOpen.addEventListener('click',  () => call('/api/open'));
    btnClose.addEventListener('click', () => call('/api/close'));

    refreshStatus();
    setInterval(refreshStatus, 2000);
  </script>
</body>
</html>
)rawliteral";

  String html(page);
  html.replace("%IP%", WiFi.localIP().toString());
  html.replace("%LAST%", lastAction);
  server.sendContent(html);
  server.client().stop(); // aktarımı kapat
}

void handleOpen() {
  if (isBusy) { server.send(200, "application/json; charset=utf-8", "{\"ok\":false,\"msg\":\"Meşgul\"}"); return; }
  openCommand = true;
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}

void handleClose() {
  if (isBusy) { server.send(200, "application/json; charset=utf-8", "{\"ok\":false,\"msg\":\"Meşgul\"}"); return; }
  closeCommand = true;
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}

void handleStatus() {
  String json = String("{\"ok\":true,\"isBusy\":") + (isBusy ? "true" : "false")
              + ",\"lastAction\":\"" + lastAction + "\"}";
  server.send(200, "application/json; charset=utf-8", json);
}

// ======= Kurulum =======
void setup() {
  Serial.begin(115200);

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("WiFi bağlanıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Bağlandı: "); Serial.println(WiFi.localIP());

  // I2C – MPU istersen kullan
  Wire.begin(21, 22);
  accelgyro.initialize();

  // Motor pinleri
  pinMode(pullPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH); // başlangıçta sürücüyü pasif tut

  // Rotalar
  server.on("/",        HTTP_GET,  handleRoot);
  server.on("/api/open",  HTTP_POST, handleOpen);
  server.on("/api/close", HTTP_POST, handleClose);
  server.on("/api/status",HTTP_GET,  handleStatus);

  server.begin();
  Serial.println("Web sunucusu başlatıldı");
}

// ======= Döngü =======
void loop() {
  server.handleClient();

  if (openCommand) {
    openCommand = false;
    isBusy = true;  lastAction = "Açılıyor…";
    Serial.println("WEB: Kapı açılıyor...");
    ileriGit(3 * stepsPerRevolution);
    delay(500);
    // İstersen güvenli pozisyona geri dön:
    // geriGit(stepsPerRevolution);
    lastAction = "Açma tamamlandı";
    isBusy = false;
  }

  if (closeCommand) {
    closeCommand = false;
    isBusy = true;  lastAction = "Kapanıyor…";
    Serial.println("WEB: Kapı kapanıyor...");
    geriGit(3 * stepsPerRevolution);
    delay(500);
    lastAction = "Kapama tamamlandı";
    isBusy = false;
  }
}
