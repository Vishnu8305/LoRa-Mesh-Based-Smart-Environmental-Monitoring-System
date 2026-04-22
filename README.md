# 🌐 LoRa Mesh-Based Smart Environmental Monitoring System

[![GitHub Repo](https://img.shields.io/badge/GitHub-Repository-black?logo=github)](https://github.com/MOHAMMADIKRAM03/LoRa-Mesh-Based-Smart-Environmental-Monitoring-System)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)
![Communication](https://img.shields.io/badge/Communication-LoRa%20Mesh-green)
![Protocol](https://img.shields.io/badge/Protocol-MQTT-orange)

---

## 🚀 Overview

A **distributed IoT monitoring system** using **LoRa Mesh networking** with multiple sensor nodes and a central master node that aggregates data and publishes it to an **MQTT broker**.

### 🌍 Monitored Domains:

* 💧 Water Quality
* 🚻 Washroom Hygiene
* 🌤 Weather Conditions
* 🌫 Outdoor Air Quality

Designed for **smart cities, environmental monitoring, and industrial IoT applications**.

---

## 📡 System Architecture

<img src="https://github.com/user-attachments/assets/c3da27f2-8272-44b5-bcf7-51646f64d50b" width="700"/>
<img src="https://github.com/user-attachments/assets/41a36eb1-d5d2-483a-93fd-e93a7725fdb5" width="750"/>

---
**🧠 System Components (Node Architecture)**
---
<p align="center">
  <img src="https://github.com/user-attachments/assets/aed22b92-71e6-42ca-9869-b1603c91d3f5" width="650"/>
</p>

A distributed set of ESP32-S3 + LoRa sensor nodes work together in a mesh network. Each node is dedicated to a specific monitoring domain, while the Master Node aggregates and publishes data via MQTT.

<img width="733" height="309" alt="image" src="https://github.com/user-attachments/assets/097c1bef-b264-4538-a4ff-a63de272357b" />


**🔍 Node Responsibilities**
---

**🌫 Node 1 – Air Quality Monitoring (0007)**

Measures PM levels (PM1, PM2.5, PM10)

Detects VOC, NOx, CO₂

Tracks temperature & humidity

👉 Used for **outdoor pollution analysis**

---

**💧 Node 2 – Water Quality Monitoring (0002)**

pH, TDS, turbidity sensing

Water temperature measurement

👉 Ensures **safe and clean water conditions**

---

**🌤 Node 3 – Weather Monitoring (0006)**

Wind speed & direction

Rainfall detection

Atmospheric pressure, temperature, humidity

👉 Provides **real-time environmental conditions**

---

**🚻 Node 4 – Washroom Hygiene Monitoring (0004)**

Detects NH₃, H₂S, CO₂, TVOC

Computes hygiene index

👉 Helps maintain **public sanitation standards**

---

**🧠 Master Node Role**
---
Collects data from all LoRa nodes

Decodes **HEX → JSON**

Identifies source node

Publishes structured data to MQTT topics

**💡 Concept Analogy**
---
Think of the system like a postal network:

📦 Nodes → Senders

📡 LoRa Mesh → Delivery network

🧠 Master Node → Sorting hub

☁️ MQTT → Final delivery to applications

**🔗 Communication Flow**
---
Sensor Node → LoRa Mesh → Master Node → MQTT Broker → Dashboard / Cloud


---

## 🧠 How It Works

### 🔹 Node Side

* Collects sensor data
* Converts JSON → HEX
* Sends via LoRa (LR71)

### 🔹 Master Node (DOIT ESP32 DevKit V1)

* Receives LoRa packets
* Decodes HEX → JSON
* Identifies source node
* Publishes to MQTT topics
<p align="center">
  <img src="https://github.com/user-attachments/assets/1fe1d0b9-0212-4eec-8a7d-954ef3df1014" width="350"/>
  <img src="https://github.com/user-attachments/assets/53b20ded-b3c3-4ec7-a85c-80d6f1c6e0fe"  width="350"/>
   <img src="https://github.com/user-attachments/assets/4f213663-e42b-487c-bcdb-e6f5c7a5f2a8"  width="550"/>
</p>


---

## 🧩 Nodes Description

### 💧 Water Quality Node (`0002`)

**Sensors:**

* pH Sensor
* TDS Sensor
* Turbidity Sensor
* DS18B20
<p align="center">
  <img src="https://github.com/user-attachments/assets/35026469-a1e8-4002-97b0-27b87e14dbcc" width="350"/>
  <img src="https://github.com/user-attachments/assets/9eaaeee2-9aec-4718-bd4f-71510f922d63" width="350"/>
</p>


**Outputs:**

   <img src="https://github.com/user-attachments/assets/8cd1df62-9141-4c1d-836b-ab4e57e3b5b8"  width="400"/>


---

### 🌤 Weather Node (`0006`)

**Sensors:**

* SparkFun Weather Meter Kit
* BME280
<p align="center">
  <img  src="https://github.com/user-attachments/assets/4e0b80b3-547f-42a9-8e64-ba157069c2f7"  width="300"/>
  <img  src="https://github.com/user-attachments/assets/16d03550-e7d4-44ec-9014-c36483bc8d52"   width="300"/>
  <img  src="https://github.com/user-attachments/assets/d60e96e5-abb2-4258-8bc7-f794fa37c12c"   width="300"/>
  


</p>

**Outputs:**

   <img src="https://github.com/user-attachments/assets/a7a87cd4-9b9e-4a3c-bc82-c3d71b80c8b6"  width="400"/>


---

### 🚻 Washroom Hygiene Node (`0004`)

**Sensors:**

* MQ136 (H₂S)
* MQ137 (NH₃)
* CCS811 (CO₂, TVOC)
<p align="center">
  <img src="https://github.com/user-attachments/assets/df8b5742-6c37-4caf-96d7-2c3048a77f7b"  width="350"/>
  <img src="https://github.com/user-attachments/assets/7ce47c0d-6c6b-4e8a-97bf-4a34c2c0096d"width="350"/>
</p>

**Outputs:**

<img  src="https://github.com/user-attachments/assets/32b99cdd-5d00-40ea-a44e-376cacba09b7"  width="400"/>

---

### 🌫 Outdoor Air Quality Node (`0007`)

**Sensors:**

* SPS30 → PM1, PM2.5, PM10
* SHT4x → Temp & Humidity
* SGP41 → VOC & NOx
* Senseair S8 → CO₂
<p align="center">
  <img src="https://github.com/user-attachments/assets/248eb7fc-3b09-4a74-8034-329ba4566794"  width="350"/>
  <img src="https://github.com/user-attachments/assets/6087254b-c2ca-40f7-9fe6-e3165922632e" width="350"/>
</p>

**Outputs:**

<img  src="https://github.com/user-attachments/assets/6c0b43d9-9b60-4b43-8535-3e7351bf70e3"  width="400"/>

---

## 🔌 Hardware

* ESP32-S3 (Nodes + Master)
* LR71 LoRa Module
* Environmental Sensors
* Solar Power System (Outdoor Node)

---

## 🔄 Data Flow

### 📤 Transmission

```
JSON → HEX → LoRa TX
```

Example:

```
{src:0002,temp:25.4,ph:7.1,tds:320,turb:120}
```

---

### 📥 Master Processing

```
HEX → JSON → MQTT Publish
```

---

## 📡 MQTT Topics

| Topic         | Data Type     |
| ------------- | ------------- |
| water/data    | Water Quality |
| weather/data  | Weather Data  |
| washroom/data | Hygiene Data  |
| air/data      | Air Quality   |

---

## 📊 Example Output

```
📡 RAW:
at+recv=0002 7B7372633A303030322C74656D703A32352E347D

✅ DECODED:
{src:0002,temp:25.4,ph:7.1,tds:320,turb:120}
```

---

## 🛠 Setup Guide

### 1️⃣ Clone Repository

```
git clone https://github.com/MOHAMMADIKRAM03/LoRa-Mesh-Based-Smart-Environmental-Monitoring-System.git
```

---

### 2️⃣ Install Libraries

**Arduino:**

* OneWire
* DallasTemperature
* PubSubClient
* SparkFun Weather Kit
* SparkFun BME280

**ESP-IDF (Washroom Node):**

* FreeRTOS
* ADC Driver
* I2C Driver

---

### 3️⃣ Configure WiFi & MQTT

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "BROKER_IP";
```

---

### 4️⃣ Upload Firmware

* Flash node codes individually
* Upload master node
* Power the system

---

## 📈 Features

✔ LoRa Mesh Communication
✔ Multi-node Distributed System
✔ Real-time MQTT Streaming
✔ Environmental Monitoring
✔ Scalable Architecture
✔ Edge Processing
✔ Low Power Design

---

## 🌍 Applications

* Smart Cities
* Industrial Monitoring
* Environmental Research
* Public Hygiene Systems
* IoT Data Analytics

---

## 🔮 Future Scope

* 📊 Dashboard (Node-RED / Grafana)
* ☁️ Cloud Integration (AWS / Firebase)
* 📱 Mobile App
* 🧠 AI Predictions
* 📡 LoRaWAN Upgrade

---

## 👨‍💻 Author

**Pedaballi Vishnu Vardhan Reddy**
B.Tech IoT — KL University

---

## ⭐ Support

If you like this project:

⭐ Star the repo
🍴 Fork it
📢 Share it

---

## 📬 Contact

* GitHub: https://github.com/Vishnu8305
* Email: pedaballivishnuvardhanreddy@gmail.com
* Linkedin: https://www.linkedin.com/in/pvishnuvardhan/
