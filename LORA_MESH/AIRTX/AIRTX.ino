#include <Arduino.h>
#include <Wire.h>

#include <SensirionI2cSht4x.h>
#include <SensirionI2CSgp41.h>
#include <VOCGasIndexAlgorithm.h>
#include <NOxGasIndexAlgorithm.h>

#define NO_ERROR 0

/* -------- LORA CONFIG -------- */
#define NODE_ADDR "0007"
#define DEST_ADDR "0001"

#define LORA_RX 4
#define LORA_TX 5

HardwareSerial loraSerial(2);

// HEX ENCODER
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

/* Sensors */

SensirionI2cSht4x sht4x;
SensirionI2CSgp41 sgp41;

VOCGasIndexAlgorithm voc_algorithm;
NOxGasIndexAlgorithm nox_algorithm;

/* CO2 UART */
HardwareSerial co2Serial(1);

static char errorMessage[128];
static int16_t error;

uint16_t conditioning_s = 10;

byte request[] = {0xFE,0x44,0x00,0x08,0x02,0x9F,0x25};

/* FAKE PM VALUES */

uint16_t mc1p0 = 50;
uint16_t mc2p5 = 50;
uint16_t mc4p0 = 50;
uint16_t mc10p0 = 50;

void setup() {

  Serial.begin(115200);
  delay(2000);

  randomSeed(micros());

  Serial.println("Air Quality System Start");

  /* I2C FIX (NO CONFLICT NOW) */
  Wire.begin(8, 9);

  /* SHT4x + SGP41 */
  sht4x.begin(Wire, SHT40_I2C_ADDR_44);
  sgp41.begin(Wire);
  sht4x.softReset();

  /* CO2 UART */
  co2Serial.begin(9600, SERIAL_8N1, 18, 19);

  /* LORA UART */
  loraSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);

  Serial.println("Sensor warming up...");
  delay(10000);
}

void loop() {

  float temperature = 0;
  float humidity = 0;

  uint16_t srawVoc = 0;
  uint16_t srawNox = 0;

  uint16_t compensationRh;
  uint16_t compensationT;

  int co2ppm = 0;

  /* FAKE PM DATA */

  mc1p0  = constrain(mc1p0 + random(-2,3),47,55);
  mc2p5  = constrain(mc2p5 + random(-2,3),47,55);
  mc4p0  = constrain(mc4p0 + random(-2,3),47,55);
  mc10p0 = constrain(mc10p0 + random(-2,3),47,55);

  /* SHT4x */

  error = sht4x.measureHighPrecision(temperature, humidity);

  if (error) {
    errorToString(error, errorMessage, sizeof(errorMessage));
    Serial.println(errorMessage);
  }

  compensationT = (uint16_t)((temperature + 45) * 65535 / 175);
  compensationRh = (uint16_t)(humidity * 65535 / 100);

  /* SGP41 */

  if (conditioning_s > 0) {
    error = sgp41.executeConditioning(compensationRh, compensationT, srawVoc);
    conditioning_s--;
  } else {
    error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc, srawNox);
  }

  int32_t voc_index = voc_algorithm.process(srawVoc);
  int32_t nox_index = nox_algorithm.process(srawNox);

  /* CO2 */

  co2Serial.write(request,7);
  delay(50);

  if(co2Serial.available() >= 7)
  {
    byte response[7];
    co2Serial.readBytes(response,7);
    co2ppm = (response[3] << 8) | response[4];
  }

  /* SERIAL OUTPUT */

  Serial.println("------ AIR QUALITY ------");

  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" C | RH: "); Serial.println(humidity);

  Serial.print("VOC Index: "); Serial.println(voc_index);
  Serial.print("NOx Index: "); Serial.println(nox_index);

  Serial.print("CO2: "); Serial.print(co2ppm);
  Serial.println(" ppm");

  Serial.print("PM1.0 : "); Serial.println(mc1p0);
  Serial.print("PM2.5 : "); Serial.println(mc2p5);
  Serial.print("PM4.0 : "); Serial.println(mc4p0);
  Serial.print("PM10  : "); Serial.println(mc10p0);

  Serial.println("-------------------------");

  /* 📡 LORA SEND */

  String payload = "{src:" + String(NODE_ADDR) +
                   ",temp:" + String(temperature,1) +
                   ",hum:"  + String(humidity,1) +
                   ",co2:"  + String(co2ppm) +
                   ",voc:"  + String(voc_index) +
                   ",nox:"  + String(nox_index) +
                   ",pm25:" + String(mc2p5) +
                   ",pm10:" + String(mc10p0) +
                   "}\n";

  String hexPayload = toHex(payload);

  String cmd = "at+ab senddata " + String(DEST_ADDR) + " " + hexPayload + "\r\n";

  Serial.println("📡 TX JSON:");
  Serial.println(payload);

  loraSerial.print(cmd);

  delay(2000);
}
