# ⚡Real-Time IoT-Based Short Circuit and Overcurrent Protection System Using ESP32

A real-time short circuit detection and alert system built on the **ESP32** microcontroller. When a fault is detected via the **ACS712 current sensor**, the system immediately disconnects the load through a **relay module**, triggers a **buzzer** and **LED** alert locally, and sends an instant notification to your phone via **Telegram**.

---

## 🌐 System Architecture

```
ACS712 Current Sensor
        │
        ▼
      ESP32  ──────────────────────────► Telegram Bot 📱
        │              Wi-Fi
        ├──► Relay Module → Load Disconnect ⛔
        ├──► Buzzer 🔔 (Audible Alert)
        └──► LED 💡 (Visual Alert)
```

---

## 🎯 Objectives

- Continuously monitor load current using the ACS712 sensor
- Detect short circuit / overcurrent conditions in real time
- Instantly disconnect the load via relay on fault detection
- Alert the user remotely through a Telegram bot message
- Provide local visual (LED) and audible (buzzer) alerts

---

## 🔧 Components

| Component | Specification | Purpose |
|-----------|--------------|---------|
| ESP32 | 38-pin Dev Board | Microcontroller + Wi-Fi for Telegram |
| ACS712 | 5A module | Hall-effect current sensing |
| Relay Module | 5V, 1-channel | Load disconnection on fault |
| Buzzer | Passive, 5V | Audible fault alert |
| LED (Red) | 5mm | Fault indicator |
| Resistor | 220Ω | LED current limiting |
| Power Supply | 5V DC | System power |
| Load | Lamp / Motor / Resistor | Protected device |

---

## ⚙️ Working Principle

```
Normal Operation:
  ACS712 reads current → Within threshold → Relay ON → Load powered ✅

Fault Detected:
  ACS712 reads spike → Exceeds threshold → Relay OFF → Load disconnected ⛔
                                         → Buzzer ON 🔔
                                         → Red LED ON 💡
                                         → Telegram Alert Sent 📱

```

The **ACS712** is a Hall-effect-based current sensor that outputs an analog voltage proportional to the current flowing through it. The ESP32 reads this via its ADC pin. When the current exceeds the defined threshold, the ESP32:

1. Opens the relay to cut power to the load
2. Activates the buzzer and red LED
3. Connects to Wi-Fi and sends a Telegram bot message to notify the user

---

## 📌 Pin Connections

| ESP32 Pin | Connected To |
|-----------|-------------|
| GPIO 34 (ADC) | ACS712 OUT |
| GPIO 26 | Relay Module IN |
| GPIO 27 | Buzzer (+) |
| GPIO 25 | Red LED (via 220Ω) |
| 3.3V / GND | ACS712 VCC / GND |
| 5V / GND | Relay VCC / GND |


---

## 📐 Circuit Overview


| Parameter | Value |
|-----------|-------|
| Supply Voltage | 5V DC |
| ACS712 Sensitivity | `185 mV/A` |
| Overcurrent Threshold | `3 A` |
| Relay Trigger Logic | Active LOW / HIGH |
| Telegram Alert Delay | `<1 s` |

---

## 💻 Software Setup

### 1. Arduino IDE Board Setup

1. Open Arduino IDE → **File > Preferences**
2. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools > Board > Board Manager** → Search `esp32` → Install

### 2. Required Libraries

Install via **Tools > Manage Libraries**:

```
-HTTPClient.h
- WiFi.h                 
```

### 3. Telegram Bot Setup

1. Open Telegram → search **@BotFather**
2. Send `/newbot` → follow the prompts → copy your **Bot Token**
3. Send any message to your new bot, then open this URL in a browser:
   ```
   https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates
   ```
4. Find `"id"` inside `"chat"` — that is your **Chat ID**

---

## 📂 Project Files

```
iot-short-circuit-protection/
├── short_circuit_detector_esp32_code  
│   └── main.ino        ← Main Arduino sketch
├── schematic/
│   └── spice_simulation
|
├── LICENSE
└── README.md

```

## 🧪 How to Test

1. Flash `main.ino` to the ESP32 via Arduino IDE (**Tools > Board > ESP32 Dev Module**)
2. Open Serial Monitor at **115200 baud**
3. Verify Wi-Fi connects and current reads ~0 A at idle
4. Simulate a short by briefly connecting a low-resistance wire across the load terminals
5. Confirm:
   - [ ] Relay clicks off (load disconnected)
   - [ ] Buzzer sounds
   - [ ] Red LED blinks
   - [ ] Telegram message received on your phone

> ⚠️ **Safety Note:** Use a current-limited bench power supply during testing. Keep the short duration brief (< 1 second).
---
## 👨‍💻 Author

  Prathul P Nair.
 - B.Tech Electronics and Communication

---
## 🤝 Contributions
  This project is intended for educational use. Feel free to adapt it for your own coursework or experiments. Also do feel free to open issues or submit pull requests if you'd like to contribute to make the code better and also to add extra features.

## 📝 License

This project is licensed under the MIT license
- checkout the LICENSE.md file for details

## 💡 Suggest Features / Contact Me

Have an idea to improve this project? I'd love to hear it! 🚀

Feel free to reach out or share your suggestions:

- 📧 **Email:** prathul.nair@gmail.com 
- 💼 **LinkedIn:**  www.linkedin.com/in/prathul-p-nair

