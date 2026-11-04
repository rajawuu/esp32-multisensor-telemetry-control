#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ================= KONFIGURASI WIFI & MQTT =================
const char* ssid        = "bakayarou"; //Put your wifi ssd here
const char* password    = "raja12345"; //The wifi pasword here
const char* mqtt_server = "192.168.137.1"; // The mqtt server here or ur ip if using local broker
const int   mqtt_port   = 1883; //mqtt port here gila english gua :v

// Kredensial Autentikasi Broker MQTT
const char* mqtt_user   = "";     // Ganti sesuai username broker MQTT Anda ato kosongin kalo ngga pakai
const char* mqtt_pass   = "";     // Ganti sesuai password broker MQTT Anda ato kosongin kalo ngga pakai

// ---------------- TOPIK TELEMETRI DATA SENSOR (Publish) ----------------
const char* topic_suhu         = "esp32gacor/monitoring/suhu";
const char* topic_kelembaban   = "esp32gacor/monitoring/kelembaban";
const char* topic_jarak        = "esp32gacor/monitoring/jarak";
const char* topic_asap         = "esp32gacor/monitoring/asap";
const char* topic_accel_x      = "esp32gacor/monitoring/accel_x";
const char* topic_accel_y      = "esp32gacor/monitoring/accel_y";
const char* topic_accel_z      = "esp32gacor/monitoring/accel_z";

// ---------------- TOPIK STATUS SENSOR & AKTUATOR (Publish ke GUI) ----------------
// Nilai Status Sensor: 1 = Online/Normal, 0 = Dimatikan (OFF), -1 = Hardware Error/Lepas
const char* topic_stat_mma     = "esp32gacor/monitoring/status_mma";
const char* topic_stat_mq2     = "esp32gacor/monitoring/status_mq2";
const char* topic_stat_sonic   = "esp32gacor/monitoring/status_sonic";
const char* topic_stat_aht     = "esp32gacor/monitoring/status_aht";

// Feedback Status Aktuator ke GUI
const char* topic_state_relay1 = "esp32gacor/monitoring/state_relay1"; // 0 atau 1
const char* topic_state_relay2 = "esp32gacor/monitoring/state_relay2"; // 0 atau 1
const char* topic_state_pwm    = "esp32gacor/monitoring/state_pwm";    // 0 - 255

// ---------------- TOPIK KONTROL DARI GUI (Subscribe) ----------------
const char* topic_cmd_mma      = "esp32gacor/control/sensor/mma";      // Kirim "1"/"ON" atau "0"/"OFF"
const char* topic_cmd_mq2      = "esp32gacor/control/sensor/mq2";      // Kirim "1"/"ON" atau "0"/"OFF"
const char* topic_cmd_sonic    = "esp32gacor/control/sensor/sonic";    // Kirim "1"/"ON" atau "0"/"OFF"
const char* topic_cmd_aht      = "esp32gacor/control/sensor/aht";      // Kirim "1"/"ON" atau "0"/"OFF"

const char* topic_cmd_relay1   = "esp32gacor/control/aktuator/relay1"; // Kirim "1"/"ON" atau "0"/"OFF"
const char* topic_cmd_relay2   = "esp32gacor/control/aktuator/relay2"; // Kirim "1"/"ON" atau "0"/"OFF"
const char* topic_cmd_pwm      = "esp32gacor/control/aktuator/pwm";    // Kirim nilai "0" s/d "255"

// Wildcard Subscribe
const char* topic_sub_metrics  = "esp32gacor/monitoring/#";
const char* topic_sub_control  = "esp32gacor/control/#";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= KONFIGURASI PIN (ESP32 DevKitC V4) =================
// Pin Sensor
#define I2C_SDA        19
#define I2C_SCL        22
#define MQ2_PIN        34
#define RX2_PIN        16  
#define TX2_PIN        17  
#define MMA_ADDR       0x1C
#define AHT_ADDR       0x38

// Pin Aktuator (Aman & Bebas Konflik di ESP32 DevKitC V4)
#define PIN_RELAY_1    25  // Output Digital 1 (Misal: Pompa / Kipas / Relay 1)
#define PIN_RELAY_2    26  // Output Digital 2 (Misal: Buzzer / Alarm / Relay 2)
#define PIN_PWM_OUT    27  // Output PWM 0-255 (Misal: Speed Kipas / Dimmer LED)

// ================= OBJEK & VARIABEL STATUS =================
Adafruit_AHTX0 aht;

// Flag Enable/Disable dari GUI (Default: Semua Menyala)
bool en_mma   = true;
bool en_mq2   = true;
bool en_sonic = true;
bool en_aht   = true;

// Flag Kesehatan Hardware (Apakah sensor merespons secara fisik)
bool hw_mma_ok   = false;
bool hw_mq2_ok   = false;
bool hw_sonic_ok = false;
bool hw_aht_ok   = false;

// State Aktuator Saat Ini
bool state_relay1 = false;
bool state_relay2 = false;
int  state_pwm    = 0;

// ---------------- TIMER (INTERVAL NON-BLOCKING) ----------------
unsigned long prev_mma       = 0; const unsigned long interval_mma       = 50;   // Sampling MMA tiap 50ms
unsigned long prev_250ms     = 0; const unsigned long interval_250ms     = 250;  // Kirim MQ2 & MMA tiap 250ms
unsigned long prev_sonic     = 0; const unsigned long interval_sonic     = 1000; // Trigger Ultrasonic tiap 1000ms
unsigned long prev_aht       = 0; const unsigned long interval_aht       = 2000; // Baca AHT10 & Status tiap 2000ms
unsigned long prev_reconnect = 0; const unsigned long interval_reconnect = 5000; // Reconnect MQTT non-blocking

// ---------------- BUFFER SENSOR ----------------
float sum_ax = 0, sum_ay = 0, sum_az = 0;
int count_mma = 0;
bool sonic_triggered = false;
unsigned long sonic_trigger_time = 0;

// ---------------- VARIABEL METRIK JARINGAN ----------------
unsigned long paket_dikirim = 0;
unsigned long paket_diterima = 0;
unsigned long waktu_kirim_terakhir = 0;
unsigned long latency_terakhir = 0;
unsigned long bytes_terkirim_window = 0;

// ================= FUNGSI KONTROL HARDWARE SENSOR =================
// Mengatur MMA845x ke Active Mode (true) atau Hardware Standby Mode (false) untuk hemat daya
bool setMMA845xActive(bool active) {
  Wire.beginTransmission(MMA_ADDR);
  Wire.write(0x2A); // CTRL_REG1
  Wire.write(active ? 0x01 : 0x00);
  return (Wire.endTransmission() == 0);
}

// Mengecek apakah alamat I2C merespons di jalur bus
bool checkI2CDevice(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

// Menghasilkan kode status sensor untuk GUI (1 = Online, 0 = Dimatikan, -1 = Hardware Error)
int getSensorStatusCode(bool is_enabled, bool is_hw_ok) {
  if (!is_enabled) return 0;
  return is_hw_ok ? 1 : -1;
}

// ================= HELPER PUBLISH & METRIK =================
void publishData(const char* topic, const String& payload, bool retained = false) {
  if (!client.connected()) return;
  
  waktu_kirim_terakhir = millis();
  if (client.publish(topic, payload.c_str(), retained)) {
    paket_dikirim++;
    bytes_terkirim_window += payload.length();
  }
}

// Kirim seluruh status sensor & aktuator terbaru ke GUI
void publishAllStatus() {
  publishData(topic_stat_mma,   String(getSensorStatusCode(en_mma, hw_mma_ok)), true);
  publishData(topic_stat_mq2,   String(getSensorStatusCode(en_mq2, hw_mq2_ok)), true);
  publishData(topic_stat_sonic, String(getSensorStatusCode(en_sonic, hw_sonic_ok)), true);
  publishData(topic_stat_aht,   String(getSensorStatusCode(en_aht, hw_aht_ok)), true);

  publishData(topic_state_relay1, String(state_relay1 ? 1 : 0), true);
  publishData(topic_state_relay2, String(state_relay2 ? 1 : 0), true);
  publishData(topic_state_pwm,    String(state_pwm), true);
}

// Helper parsing payload perintah ON/OFF dari GUI (mendukung "1", "ON", "true")
bool parseBoolCommand(String msg) {
  msg.trim();
  msg.toUpperCase();
  return (msg == "1" || msg == "ON" || msg == "TRUE");
}

// ================= CALLBACK MQTT (TERIMA KONTROL GUI & HITUNG LATENCY) =================
void callback(char* topic, byte* payload, unsigned int length) {
  unsigned long waktu_terima = millis();

  // 1. Jika pesan berasal dari topik monitoring, hitung sebagai metrik loopback
  if (strncmp(topic, "esp32gacor/monitoring/", 22) == 0) {
    paket_diterima++;
    latency_terakhir = waktu_terima - waktu_kirim_terakhir;
    return;
  }

  // 2. Jika pesan berasal dari topik control GUI, salin payload menjadi String
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  // --- Kontrol Enable/Disable Sensor ---
  if (strcmp(topic, topic_cmd_mma) == 0) {
    en_mma = parseBoolCommand(msg);
    hw_mma_ok = setMMA845xActive(en_mma); // Masuk mode standby jika dimatikan
    sum_ax = 0; sum_ay = 0; sum_az = 0; count_mma = 0;
    publishData(topic_stat_mma, String(getSensorStatusCode(en_mma, hw_mma_ok)), true);
  }
  else if (strcmp(topic, topic_cmd_mq2) == 0) {
    en_mq2 = parseBoolCommand(msg);
    publishData(topic_stat_mq2, String(getSensorStatusCode(en_mq2, hw_mq2_ok)), true);
  }
  else if (strcmp(topic, topic_cmd_sonic) == 0) {
    en_sonic = parseBoolCommand(msg);
    sonic_triggered = false;
    publishData(topic_stat_sonic, String(getSensorStatusCode(en_sonic, hw_sonic_ok)), true);
  }
  else if (strcmp(topic, topic_cmd_aht) == 0) {
    en_aht = parseBoolCommand(msg);
    publishData(topic_stat_aht, String(getSensorStatusCode(en_aht, hw_aht_ok)), true);
  }
  // --- Kontrol Output Aktuator ---
  else if (strcmp(topic, topic_cmd_relay1) == 0) {
    state_relay1 = parseBoolCommand(msg);
    digitalWrite(PIN_RELAY_1, state_relay1 ? HIGH : LOW);
    publishData(topic_state_relay1, String(state_relay1 ? 1 : 0), true);
  }
  else if (strcmp(topic, topic_cmd_relay2) == 0) {
    state_relay2 = parseBoolCommand(msg);
    digitalWrite(PIN_RELAY_2, state_relay2 ? HIGH : LOW);
    publishData(topic_state_relay2, String(state_relay2 ? 1 : 0), true);
  }
  else if (strcmp(topic, topic_cmd_pwm) == 0) {
    state_pwm = constrain(msg.toInt(), 0, 255);
    analogWrite(PIN_PWM_OUT, state_pwm);
    publishData(topic_state_pwm, String(state_pwm), true);
  }
}

// ================= KONEKSI WIFI & MQTT =================
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

void checkMqttConnection(unsigned long currentMillis) {
  if (!client.connected()) {
    if (currentMillis - prev_reconnect >= interval_reconnect) {
      prev_reconnect = currentMillis;
      String clientId = "ESP32Client-" + String(random(0xffff), HEX);
      
      // Connect menggunakan Username dan Password
      if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
        Serial.println("MQTT Terhubung (Authenticated)!");
        client.subscribe(topic_sub_metrics);
        client.subscribe(topic_sub_control);
        publishAllStatus(); // Sinkronisasi state awal ke GUI
      } else {
        Serial.printf("Gagal konek MQTT, rc=%d. Coba lagi 5 detik...\n", client.state());
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  // Inisialisasi Pin Aktuator
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_PWM_OUT, OUTPUT);
  digitalWrite(PIN_RELAY_1, LOW);
  digitalWrite(PIN_RELAY_2, LOW);
  analogWrite(PIN_PWM_OUT, 0);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  // Inisialisasi awal sensor
  hw_aht_ok = aht.begin(&Wire);
  hw_mma_ok = setMMA845xActive(true);
}

void loop() {
  unsigned long currentMillis = millis();

  checkMqttConnection(currentMillis);
  client.loop();

  // ================= 1. SAMPLING MMA845x (Tiap 50 ms jika aktif) =================
  if (en_mma && (currentMillis - prev_mma >= interval_mma)) {
    prev_mma = currentMillis;
    Wire.beginTransmission(MMA_ADDR);
    Wire.write(0x01); 
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom(MMA_ADDR, 6) == 6) {
      hw_mma_ok = true;
      int16_t x = ((int16_t)(Wire.read() << 8 | Wire.read())) >> 2;
      int16_t y = ((int16_t)(Wire.read() << 8 | Wire.read())) >> 2;
      int16_t z = ((int16_t)(Wire.read() << 8 | Wire.read())) >> 2;
      sum_ax += (float)x / 4096.0;
      sum_ay += (float)y / 4096.0;
      sum_az += (float)z / 4096.0;
      count_mma++;
    } else {
      hw_mma_ok = false; // Sensor gagal merespons di bus I2C
    }
  }

  // ================= 2. KIRIM MQ-2 & RATA-RATA MMA845x (Tiap 250 ms) =================
  if (currentMillis - prev_250ms >= interval_250ms) {
    prev_250ms = currentMillis;

    // --- Eksekusi MQ-2 hanya jika diaktifkan ---
    if (en_mq2) {
      int raw_mq2 = analogRead(MQ2_PIN);
      // Deteksi apakah kabel analog terputus/short ke rail (0 atau 4095 ekstrem)
      hw_mq2_ok = (raw_mq2 > 5 && raw_mq2 < 4090);
      if (hw_mq2_ok) {
        int mq2_fixed = 4095 - raw_mq2;
        publishData(topic_asap, String(mq2_fixed));
      }
    }

    // --- Eksekusi pengiriman MMA hanya jika diaktifkan & sampel tersedia ---
    if (en_mma && hw_mma_ok && count_mma > 0) {
      publishData(topic_accel_x, String(sum_ax / count_mma, 2));
      publishData(topic_accel_y, String(sum_ay / count_mma, 2));
      publishData(topic_accel_z, String(sum_az / count_mma, 2));
      sum_ax = 0; sum_ay = 0; sum_az = 0; count_mma = 0;
    }
  }

  // ================= 3. TRIGGER & KIRIM ULTRASONIC (Tiap 1000 ms jika aktif) =================
  if (en_sonic) {
    if (!sonic_triggered && (currentMillis - prev_sonic >= interval_sonic)) {
      prev_sonic = currentMillis;
      while (Serial2.available()) Serial2.read();
      Serial2.write(0xA0); 
      sonic_triggered = true;
      sonic_trigger_time = currentMillis;
    }

    if (sonic_triggered && (currentMillis - sonic_trigger_time >= 150)) {
      if (Serial2.available() >= 3) {
        hw_sonic_ok = true;
        uint32_t b_h = Serial2.read();
        uint32_t b_m = Serial2.read();
        uint32_t b_l = Serial2.read();
        float dist_val = (float)((b_h << 16) | (b_m << 8) | b_l) / 10000.0;
        if (dist_val >= 700.0) {
          dist_val = 0.0;
        }
        publishData(topic_jarak, String(dist_val, 2));
        // Filter: Jika nilai >= 700 (tidak ada objek terdeteksi), paksa kirim 0

      } else {
        hw_sonic_ok = false; // Timeout: Sensor tidak membalas lewat UART2
      }
      sonic_triggered = false;
    }
  }

  // ================= 4. AHT10, SINKRONISASI STATUS & METRIK (Tiap 2000 ms) =================
  if (currentMillis - prev_aht >= interval_aht) {
    prev_aht = currentMillis;

    // --- Eksekusi AHT10 hanya jika diaktifkan ---
    if (en_aht) {
      if (checkI2CDevice(AHT_ADDR)) {
        if (!hw_aht_ok) hw_aht_ok = aht.begin(&Wire); // Re-init otomatis jika sempat lepas
        sensors_event_t humidity, temp;
        if (hw_aht_ok && aht.getEvent(&humidity, &temp)) {
          publishData(topic_suhu, String(temp.temperature, 2));
          publishData(topic_kelembaban, String(humidity.relative_humidity, 2));
        } else {
          hw_aht_ok = false;
        }
      } else {
        hw_aht_ok = false;
      }
    }

    // Kirim pembaruan status kesehatan sensor & aktuator ke GUI
    publishAllStatus();

    // Kalkulasi Metrik Jaringan
    float data_loss = 0.0;
    if (paket_dikirim > 0) {
      data_loss = ((float)(paket_dikirim - paket_diterima) / paket_dikirim) * 100.0;
    }
    float throughput_kbits = ((float)bytes_terkirim_window * 8.0) / (float)interval_aht;

    Serial.println("================= PARAMETER PENGUJIAN =================");
    Serial.printf("🔌 Status [MMA|MQ2|SONIC|AHT] : [%d | %d | %d | %d]\n",
                  getSensorStatusCode(en_mma, hw_mma_ok),
                  getSensorStatusCode(en_mq2, hw_mq2_ok),
                  getSensorStatusCode(en_sonic, hw_sonic_ok),
                  getSensorStatusCode(en_aht, hw_aht_ok));
    Serial.printf("⚙️  Aktuator [R1 | R2 | PWM]   : [%d | %d | %d]\n", state_relay1, state_relay2, state_pwm);
    Serial.printf("📦 Total Payload (2s)         : %lu byte\n", bytes_terkirim_window);
    Serial.printf("⚡ Latency Terakhir           : %lu ms\n", latency_terakhir);
    Serial.printf("🚀 Throughput                 : %.4f kbit/s\n", throughput_kbits);
    Serial.printf("📉 Data Loss                  : %.2f %%\n", data_loss);
    Serial.println("=======================================================\n");

    bytes_terkirim_window = 0;
  }
}
