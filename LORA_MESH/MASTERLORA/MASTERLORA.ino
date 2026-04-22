/*************************************************
 * MASTER NODE (ESP32 + LR71)
 * ✔ ONLY RECEIVES DATA
 * ✔ Decodes sensor payload
 * ✔ Displays clean output
 *************************************************/

#define LORA_RX 16
#define LORA_TX 17

HardwareSerial loraSerial(2);

String incomingLine = "";

void setup() {
  Serial.begin(115200);
  loraSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);

  delay(1000);
  Serial.println("===== MASTER RECEIVER READY =====");
}

void loop() {

  while (loraSerial.available()) {
    char c = loraSerial.read();

    // Show raw incoming data
    Serial.write(c);

    if (c == '\n') {

      incomingLine.trim();

      // Process only LoRa received data
      if (incomingLine.startsWith("+RCV=")) {

        int c1 = incomingLine.indexOf(',');
        int c2 = incomingLine.indexOf(',', c1 + 1);
        int c3 = incomingLine.indexOf(',', c2 + 1);

        // Extract sender and payload
        String sender = incomingLine.substring(5, c1);
        String payload = incomingLine.substring(c2 + 1, c3);
        payload.trim();

        Serial.println("\n======= RECEIVED DATA =======");
        Serial.println("From Node : " + sender);
        Serial.println("Payload   : " + payload);

        // -------- PARSE SENSOR DATA --------
        float h2s=0, nh3=0, idx=0;
        int co2=0, tvoc=0;

        int parsed = sscanf(payload.c_str(),
                            "H2S:%f,NH3:%f,CO2:%d,TVOC:%d,IDX:%f",
                            &h2s, &nh3, &co2, &tvoc, &idx);

        if (parsed >= 5) {

          Serial.println("------ DECODED VALUES ------");
          Serial.printf("H2S   : %.2f ppm\n", h2s);
          Serial.printf("NH3   : %.2f ppm\n", nh3);
          Serial.printf("CO2   : %d ppm\n", co2);
          Serial.printf("TVOC  : %d ppb\n", tvoc);
          Serial.printf("INDEX : %.1f\n", idx);

        } else {
          Serial.println("⚠ Unable to parse payload properly");
        }

        Serial.println("============================\n");
      }

      // Clear buffer
      incomingLine = "";
    }
    else {
      incomingLine += c;
    }
  }

  // Optional: manual AT commands passthrough
  while (Serial.available()) {
    loraSerial.write(Serial.read());
  }
}