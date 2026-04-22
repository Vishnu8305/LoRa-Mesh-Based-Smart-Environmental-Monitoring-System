#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------------- CONFIG ----------------
#define WIFI_SSID "iPhone"
#define WIFI_PASS "2200100028"
#define MQTT_SERVER "34.93.121.38"
#define MQTT_PORT 1883

#define LORA_RX 16
#define LORA_TX 17
#define LORA_RST 4

// ---------------- OBJECTS ----------------
WiFiClient espClient;
PubSubClient client(espClient);
HardwareSerial loraSerial(2);

// ---------------- BUFFER ----------------
char incomingLine[256];
int bufferIndex = 0;

// ---------------- WATCHDOG ----------------
unsigned long lastDataTime = 0;

// ---------------- WIFI ----------------
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected");
  } else {
    Serial.println("\n❌ WiFi Failed");
  }
}

// ---------------- MQTT ----------------
void connectMQTT() {
  if (client.connected()) return;

  Serial.println("Connecting MQTT...");
  if (client.connect("ESP32_MASTER_NODE")) {
    Serial.println("✅ MQTT Connected");
  } else {
    Serial.println("❌ MQTT Failed");
  }
}

// ---------------- LORA RESET ----------------
void resetLoRa() {
  Serial.println("🔄 Resetting LoRa...");

  digitalWrite(LORA_RST, LOW);
  delay(200);
  digitalWrite(LORA_RST, HIGH);
  delay(1000);

  bufferIndex = 0;
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  // LoRa Serial
  loraSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);

  // Reset pin
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, HIGH);

  resetLoRa();

  connectWiFi();

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setBufferSize(512);

  lastDataTime = millis();

  Serial.println("🌐 MASTER READY");
}

// ---------------- LOOP ----------------
void loop() {

  // WiFi reconnect
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }

  // MQTT reconnect
  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  // ---------------- SERIAL READ ----------------
  while (loraSerial.available()) {
    char c = loraSerial.read();

    if (c == '\n' || c == '\r') {

      if (bufferIndex == 0) continue;

      incomingLine[bufferIndex] = '\0';

      Serial.println("\n📥 DATA: " + String(incomingLine));

      lastDataTime = millis();

      // ===============================
      // 🔍 DETECT SOURCE
      // ===============================
      int src = 0;
      sscanf(incomingLine, "{src:%d", &src);

      // ===============================
      // 🚽 WASHROOM (src:0004)
      // ===============================
      if (src == 4) {

        float h2s=0, nh3=0, idx=0;
        int co2=0, tvoc=0;

        int parsed = sscanf(incomingLine,
          "{src:%*d,h2s:%f,nh3:%f,co2:%d,tvoc:%d,idx:%f",
          &h2s,&nh3,&co2,&tvoc,&idx);

        if (parsed >= 5) {
          Serial.println("🚽 Washroom OK");

          client.publish("washroom/h2s", String(h2s,2).c_str());
          client.publish("washroom/nh3", String(nh3,2).c_str());
          client.publish("washroom/co2", String(co2).c_str());
          client.publish("washroom/tvoc", String(tvoc).c_str());
          client.publish("washroom/idx", String(idx,1).c_str());
        } else {
          Serial.println("❌ Washroom Parse Error");
        }
      }

      // ===============================
      // 🌤 WEATHER (src:0006)
      // ===============================
      else if (src == 6) {

        float t=0,h=0,p=0,ws=0,wd=0,r=0;

        int parsed = sscanf(incomingLine,
          "{src:%*d,t:%f,h:%f,p:%f,ws:%f,wd:%f,r:%f}",
          &t,&h,&p,&ws,&wd,&r);

        if (parsed == 6) {
          Serial.println("🌤 Weather OK");

          client.publish("weather/temp", String(t,1).c_str());
          client.publish("weather/humidity", String(h,0).c_str());
          client.publish("weather/pressure", String(p,0).c_str());
          client.publish("weather/wind_speed", String(ws,1).c_str());
          client.publish("weather/wind_dir", String(wd,0).c_str());
          client.publish("weather/rain", String(r,1).c_str());
        } else {
          Serial.println("❌ Weather Parse Error");
        }
      }

      else {
        Serial.println("⚠ Unknown Source");
      }

      bufferIndex = 0;
    }
    else {
      if (bufferIndex < sizeof(incomingLine) - 1) {
        incomingLine[bufferIndex++] = c;
      } else {
        Serial.println("⚠ Buffer Overflow Reset");
        bufferIndex = 0;
      }
    }
  }

  // ---------------- AUTO RESET ----------------
  if (millis() - lastDataTime > 20000) {
    Serial.println("⚠ No LoRa Data → Auto Reset");
    resetLoRa();
    lastDataTime = millis();
  }
}