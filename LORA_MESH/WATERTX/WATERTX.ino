/*************************************************
 * WATER QUALITY TX NODE (ESP32-S3 + LR71)
 *************************************************/

#include <OneWire.h>
#include <DallasTemperature.h>

// -------- NODE CONFIG --------
#define NODE_ADDR "0002"   // 🔥 Water node ID
#define DEST_ADDR "0001"

// -------- LORA UART --------
#define LORA_RX 16
#define LORA_TX 17
HardwareSerial loraSerial(2);

// -------- SENSOR PINS --------
#define PH_PIN         1
#define TDS_PIN        2
#define TURBIDITY_PIN  3
#define ONE_WIRE_BUS   4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// -------- VARIABLES --------
int phADC, tdsADC;
float phVoltage, phValue, tdsVoltage, tdsValue, temperatureC;
float turbidityVolt, ntu;

// -------- HEX ENCODER --------
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

// -------- SETUP --------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("🌊 WATER TX READY");

  sensors.begin();

  analogReadResolution(12); // ESP32-S3 ADC

  // LoRa UART
  loraSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);
}

// -------- LOOP --------
void loop() {

  // --- Temperature ---
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);

  // --- pH ---
  phADC = analogRead(PH_PIN);
  phVoltage = phADC * (3.3 / 4095.0);
  phValue = 7 + ((2.5 - phVoltage) / 0.18);

  // --- TDS ---
  tdsADC = analogRead(TDS_PIN);
  tdsVoltage = (tdsADC / 4095.0) * 3.3;

  float compCoeff = 1.0 + 0.02 * (temperatureC - 25.0);
  float compVoltage = tdsVoltage / compCoeff;

  tdsValue = (133.42 * pow(compVoltage, 3)
             - 255.86 * pow(compVoltage, 2)
             + 857.39 * compVoltage) * 0.5;

  // --- Turbidity ---
  turbidityVolt = 0;
  for (int i = 0; i < 500; i++) {
    turbidityVolt += ((float)analogRead(TURBIDITY_PIN) / 4095.0) * 3.3;
  }

  turbidityVolt /= 500.0;
  turbidityVolt = round_to_dp(turbidityVolt, 2);

  if (turbidityVolt < 1.6) {
    ntu = 3000;
  } else {
    ntu = -1120.4 * sq(turbidityVolt) + 5742.3 * turbidityVolt - 4353.8;
  }

  // =====================================
  // 📡 CREATE JSON PAYLOAD (MATCH MASTER)
  // =====================================
  String payload = "{src:" + String(NODE_ADDR) +
                   ",temp:" + String(temperatureC,1) +
                   ",ph:"   + String(phValue,2) +
                   ",tds:"  + String(tdsValue,0) +
                   ",turb:" + String(ntu,0) +
                   "}\n";

  String hexPayload = toHex(payload);

  String cmd = "at+ab SendData " + String(DEST_ADDR) + " " + hexPayload + "\r\n";

  // -------- DEBUG --------
  Serial.println("📡 TX JSON:");
  Serial.println(payload);

  Serial.println("📡 HEX:");
  Serial.println(hexPayload);

  // -------- SEND --------
  loraSerial.print(cmd);

  delay(5000);
}

// -------- UTILITY --------
float round_to_dp(float val, int dp) {
  float mul = pow(10.0, dp);
  return round(val * mul) / mul;
}