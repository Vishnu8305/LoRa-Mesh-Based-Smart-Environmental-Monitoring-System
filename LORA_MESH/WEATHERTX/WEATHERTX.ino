/*************************************************
 * WEATHER TX NODE (ARDUINO - LR71 STYLE)
 *************************************************/

#include <Wire.h>
#include "SparkFun_Weather_Meter_Kit_Arduino_Library.h"
#include "SparkFunBME280.h"

// -------- NODE CONFIG --------
#define NODE_ADDR "0006"
#define DEST_ADDR "0001"

// -------- LORA UART --------
#define LORA_RX 16
#define LORA_TX 17
HardwareSerial loraSerial(2);

// -------- WEATHER SENSORS --------
const int pinWindDirection = 35;
const int pinWindSpeed     = 14;
const int pinRainfall      = 27;

SFEWeatherMeterKit weather(pinWindDirection, pinWindSpeed, pinRainfall);
BME280 bme;

bool bmeOK = false;

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

void setup() {
  Serial.begin(115200);
  Wire.begin();

  weather.begin();
  bmeOK = bme.beginI2C();

  loraSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("🌤 WEATHER TX READY");
}

void loop() {

  float t=0,h=0,p=0;
  float ws=0,wd=0,r=0;

  wd = weather.getWindDirection();
  ws = weather.getWindSpeed();
  r  = weather.getTotalRainfall();

  if (bmeOK) {
    t = bme.readTempC();
    h = bme.readFloatHumidity();
    p = bme.readFloatPressure() / 100.0;
  }

  // SAME FORMAT AS YOUR WASHROOM CODE
  String payload = "{src:" + String(NODE_ADDR) +
                   ",t:"  + String(t,1) +
                   ",h:"  + String(h,0) +
                   ",p:"  + String(p,0) +
                   ",ws:" + String(ws,1) +
                   ",wd:" + String(wd,0) +
                   ",r:"  + String(r,1) +
                   "}\n";

  String hexPayload = toHex(payload);

  String cmd = "at+ab SendData " + String(DEST_ADDR) + " " + hexPayload + "\r\n";

  Serial.println("📡 TX JSON:");
  Serial.println(payload);

  Serial.println("📡 HEX:");
  Serial.println(hexPayload);

  loraSerial.print(cmd);

  delay(5000);
}