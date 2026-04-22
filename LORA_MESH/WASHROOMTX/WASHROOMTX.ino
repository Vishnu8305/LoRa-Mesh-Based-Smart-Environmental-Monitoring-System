/*************************************************
 * WASHROOM TX NODE (ESP32 + LR71)
 *************************************************/

#include <Wire.h>
#include "Adafruit_CCS811.h"

// -------- NODE CONFIG --------
#define NODE_ADDR "0004"   // 🔥 Washroom node
#define DEST_ADDR "0001"

// -------- LORA UART --------
#define LORA_RX 16
#define LORA_TX 17
HardwareSerial loraSerial(2);

Adafruit_CCS811 ccs;

// Pins
#define MQ136_PIN A0
#define MQ137_PIN A1

#define ADC_MAX 4095.0
#define ADC_VREF 3.3
#define DIVIDER_RATIO 0.5
#define RL 10000.0

float R0_MQ136 = 20000.0;
float R0_MQ137 = 20000.0;

bool mq136Connected = false;
bool mq137Connected = false;
bool ccsConnected   = false;

// ---------- HEX ENCODER ----------
String toHex(String input) {
  const char* hex = "0123456789ABCDEF";
  String out = "";

  for (int i = 0; i < input.length(); i++) {
    byte b = input[i];
    out += hex[b >> 4];
    out += hex[b & 0x0F];
  }
  return out;
}

// ---------- Utility ----------
float getVoltage(int pin) {
  int adc = analogRead(pin);
  float v_adc = (adc / ADC_MAX) * ADC_VREF;
  return v_adc / DIVIDER_RATIO;
}

float calculateRs(float v) {
  if (v <= 0.01) return 0;
  return (5.0 - v) * RL / v;
}

float estimatePPM(float rs, float R0, float m, float b) {
  if (rs <= 0) return 0;
  float ratio = rs / R0;
  float logppm = (log10(ratio) - b) / m;
  return pow(10, logppm);
}

float normalize(float value, float cleanMax, float criticalMax) {
  if (value <= cleanMax) return 0;
  if (value >= criticalMax) return 100;
  return ((value - cleanMax) / (criticalMax - cleanMax)) * 100.0;
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(2000);

  analogReadResolution(12);
  Wire.begin(D4, D5);

  // LoRa UART
  loraSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("🚽 WASHROOM TX READY");

  // Sensor checks
  if (analogRead(MQ136_PIN) > 0) mq136Connected = true;
  if (analogRead(MQ137_PIN) > 0) mq137Connected = true;
  if (ccs.begin()) ccsConnected = true;
}

// ---------- LOOP ----------
void loop() {

  float h2s = 0, nh3 = 0, idx = 0;
  int eco2 = 400, tvoc = 0;

  // MQ-136
  if (mq136Connected) {
    float rs136 = calculateRs(getVoltage(MQ136_PIN));
    h2s = estimatePPM(rs136, R0_MQ136, -1.43, 0.43);
  }

  // MQ-137
  if (mq137Connected) {
    float rs137 = calculateRs(getVoltage(MQ137_PIN));
    nh3 = estimatePPM(rs137, R0_MQ137, -0.45, 1.20);
  }

  // CCS811
  if (ccsConnected && ccs.available()) {
    if (!ccs.readData()) {
      eco2 = ccs.geteCO2();
      tvoc = ccs.getTVOC();
    }
  }

  // Hygiene index
  float index = 0, weightSum = 0;

  if (mq136Connected) {
    index += 0.30 * normalize(h2s, 3, 20);
    weightSum += 0.30;
  }

  if (mq137Connected) {
    index += 0.30 * normalize(nh3, 5, 50);
    weightSum += 0.30;
  }

  if (ccsConnected) {
    index += 0.20 * normalize(eco2, 600, 2000);
    index += 0.20 * normalize(tvoc, 65, 600);
    weightSum += 0.40;
  }

  if (weightSum > 0) index /= weightSum;

  idx = index;

  // =====================================
  // 📡 CREATE JSON PAYLOAD
  // =====================================
  String payload = "{src:" + String(NODE_ADDR) +
                   ",h2s:" + String(h2s,2) +
                   ",nh3:" + String(nh3,2) +
                   ",co2:" + String(eco2) +
                   ",tvoc:" + String(tvoc) +
                   ",idx:" + String(idx,1) +
                   "}\n";

  String hexPayload = toHex(payload);

  String cmd = "at+ab SendData " + String(DEST_ADDR) + " " + hexPayload + "\r\n";

  // Debug
  Serial.println("📡 TX JSON:");
  Serial.println(payload);

  Serial.println("📡 HEX:");
  Serial.println(hexPayload);

  // Send via LoRa
  loraSerial.print(cmd);

  delay(5000);
}