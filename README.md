# ⚡ IoT-Based Short Circuit Protection System

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
| ACS712 | 5A / 20A / 30A module | Hall-effect current sensing |
| Relay Module | 5V, 1-channel | Load disconnection on fault |
| Buzzer | Active, 5V | Audible fault alert |
| LED (Red) | 5mm | Fault indicator |
| LED (Green) | 5mm | Normal operation indicator |
| Resistor | 220Ω x2 | LED current limiting |
| Power Supply | 5V DC | System power |
| Load | Lamp / Motor / Resistor | Protected device |

> ✏️ *Update ACS712 variant (5A / 20A / 30A) based on your actual module.*

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

Fault Cleared:
  Manual reset → Relay ON → System restored ✅
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
| GPIO 14 | Red LED (via 220Ω) |
| GPIO 12 | Green LED (via 220Ω) |
| 3.3V / GND | ACS712 VCC / GND |
| 5V / GND | Relay VCC / GND |

> ✏️ *Modify GPIO pins to match your actual wiring.*

---

## 📐 Circuit Overview

> 📌 *Add your schematic / Fritzing diagram here.*

**Key Parameters (update with your values):**

| Parameter | Value |
|-----------|-------|
| Supply Voltage | 5V DC |
| ACS712 Sensitivity | `___ mV/A` |
| Overcurrent Threshold | `___ A` |
| Relay Trigger Logic | Active LOW / HIGH |
| Telegram Alert Delay | `< ___ ms` |

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
- UniversalTelegramBot   (by Brian Lough)
- ArduinoJson            (by Benoit Blanchon)  ← Use v6.x
- WiFi.h                 (built-in with ESP32 package)
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
├── src/
│   ├── main.ino        ← Main Arduino sketch
│   └── config.h        ← WiFi, Telegram credentials & pin config
├── schematic/
│   └── circuit_diagram.png
├── docs/
│   └── project_report.pdf
└── README.md
```

---

## 🗂️ config.h

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// ─── Wi-Fi ────────────────────────────────────────────────
#define WIFI_SSID         "your_wifi_name"
#define WIFI_PASSWORD     "your_wifi_password"

// ─── Telegram ─────────────────────────────────────────────
#define BOT_TOKEN         "your_telegram_bot_token"
#define CHAT_ID           "your_chat_id"

// ─── Pin Definitions ──────────────────────────────────────
#define ACS712_PIN        34    // ADC pin (input only, no pullup)
#define RELAY_PIN         26
#define BUZZER_PIN        27
#define LED_RED_PIN       14
#define LED_GREEN_PIN     12

// ─── ACS712 Calibration ───────────────────────────────────
// Sensitivity: 0.185 V/A for 5A module
//              0.100 V/A for 20A module
//              0.066 V/A for 30A module
#define ACS712_SENSITIVITY  0.185f
#define ACS712_OFFSET       2.5f    // Voltage at 0A (Vcc/2)

// ─── Protection Settings ──────────────────────────────────
#define CURRENT_THRESHOLD   5.0f    // Amps — adjust to your load
#define ALERT_COOLDOWN_MS   10000   // Minimum ms between Telegram alerts

// ─── Device Label (shown in Telegram message) ─────────────
#define DEVICE_LABEL      "Lab Bench - ESP32 Node 1"

#endif
```

---

## 🗂️ main.ino

```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "config.h"

// ─── Globals ──────────────────────────────────────────────
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

bool faultActive       = false;
unsigned long lastAlert = 0;

// ─── Function Prototypes ──────────────────────────────────
float   readCurrent();
void    triggerFault(float current);
void    clearFault();
void    sendTelegramAlert(float current);
void    connectWiFi();
String  getTimestamp();

// ──────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN,     OUTPUT);
  pinMode(BUZZER_PIN,    OUTPUT);
  pinMode(LED_RED_PIN,   OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  // Safe default state — relay ON (load powered), no alerts
  digitalWrite(RELAY_PIN,     LOW);   // LOW = relay energized (adjust if active-HIGH module)
  digitalWrite(BUZZER_PIN,    LOW);
  digitalWrite(LED_RED_PIN,   LOW);
  digitalWrite(LED_GREEN_PIN, HIGH);

  connectWiFi();

  client.setInsecure();   // Accept Telegram's SSL cert without CA verification
  Serial.println("System ready. Monitoring current...");
}

// ──────────────────────────────────────────────────────────
void loop() {
  float current = readCurrent();

  Serial.print("Current: ");
  Serial.print(current, 3);
  Serial.println(" A");

  if (current >= CURRENT_THRESHOLD && !faultActive) {
    triggerFault(current);
  }

  if (faultActive) {
    // Blink red LED while fault is active
    digitalWrite(LED_RED_PIN, (millis() / 300) % 2);
  }

  // Re-check Wi-Fi and attempt reconnect if dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    connectWiFi();
  }

  delay(200);
}

// ──────────────────────────────────────────────────────────
float readCurrent() {
  // Average 20 samples to reduce ADC noise
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(ACS712_PIN);
    delayMicroseconds(500);
  }
  float raw     = sum / 20.0f;
  float voltage = (raw / 4095.0f) * 3.3f;          // ESP32: 12-bit ADC, 3.3V ref
  float current = (voltage - ACS712_OFFSET) / ACS712_SENSITIVITY;
  return abs(current);                               // Return absolute value
}

// ──────────────────────────────────────────────────────────
void triggerFault(float current) {
  faultActive = true;

  // Disconnect load
  digitalWrite(RELAY_PIN,     HIGH);  // HIGH = relay open (load disconnected)
  digitalWrite(BUZZER_PIN,    HIGH);  // Buzzer ON
  digitalWrite(LED_GREEN_PIN, LOW);   // Green OFF

  Serial.println("⚠️  FAULT DETECTED — Load disconnected.");
  Serial.print("   Measured current: ");
  Serial.println(current);

  // Send Telegram alert (with cooldown to avoid spam)
  unsigned long now = millis();
  if (now - lastAlert > ALERT_COOLDOWN_MS) {
    sendTelegramAlert(current);
    lastAlert = now;
  }
}

// ──────────────────────────────────────────────────────────
void clearFault() {
  faultActive = false;

  digitalWrite(RELAY_PIN,     LOW);   // Relay ON — load restored
  digitalWrite(BUZZER_PIN,    LOW);   // Buzzer OFF
  digitalWrite(LED_RED_PIN,   LOW);   // Red OFF
  digitalWrite(LED_GREEN_PIN, HIGH);  // Green ON

  Serial.println("✅ Fault cleared. System restored.");

  bot.sendMessage(CHAT_ID,
    "✅ *Fault Cleared*\n"
    "📍 Device: " DEVICE_LABEL "\n"
    "🟢 Status: Load RECONNECTED",
    "Markdown");
}

// ──────────────────────────────────────────────────────────
void sendTelegramAlert(float current) {
  String message =
    "⚠️ *SHORT CIRCUIT DETECTED!*\n\n"
    "📍 *Device:* " + String(DEVICE_LABEL) + "\n"
    "⚡ *Current:* " + String(current, 2) + " A"
    "  _(Threshold: " + String(CURRENT_THRESHOLD, 1) + " A)_\n"
    "🔴 *Status:* Load DISCONNECTED\n\n"
    "_Please inspect the circuit before resetting._";

  Serial.println("Sending Telegram alert...");
  bool sent = bot.sendMessage(CHAT_ID, message, "Markdown");

  if (sent) {
    Serial.println("Telegram alert sent successfully.");
  } else {
    Serial.println("Failed to send Telegram alert.");
  }
}

// ──────────────────────────────────────────────────────────
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWi-Fi connection failed. Alerts will not be sent.");
  }
}
```

> 💡 **To manually reset after a fault**, add a push button on any GPIO pin and call `clearFault()` in its interrupt handler, or add a Serial command check in `loop()`.

---

## 📱 Telegram Alert Format

When a fault is detected, you will receive:

```
⚠️ SHORT CIRCUIT DETECTED!

📍 Device   : Lab Bench - ESP32 Node 1
⚡ Current  : 12.43 A  (Threshold: 5.0 A)
🔴 Status   : Load DISCONNECTED

Please inspect the circuit before resetting.
```

And when cleared:

```
✅ Fault Cleared
📍 Device   : Lab Bench - ESP32 Node 1
🟢 Status   : Load RECONNECTED
```

---

## 🧪 How to Test

1. Flash `main.ino` to the ESP32 via Arduino IDE (**Tools > Board > ESP32 Dev Module**)
2. Open Serial Monitor at **115200 baud**
3. Verify Wi-Fi connects and current reads ~0 A at idle
4. Power on the load — green LED should glow
5. Simulate a short by briefly connecting a low-resistance wire across the load terminals
6. Confirm:
   - [ ] Relay clicks off (load disconnected)
   - [ ] Buzzer sounds
   - [ ] Red LED blinks
   - [ ] Telegram message received on your phone

> ⚠️ **Safety Note:** Use a current-limited bench power supply during testing. Keep the short duration brief (< 1 second).

---

## 📊 Observations

| Test Case | Current Measured | Relay Tripped | Telegram Alert |
|-----------|-----------------|---------------|----------------|
| No Load | `___ A` | No | — |
| Normal Load | `___ A` | No | — |
| Simulated Short | `___ A` | Yes ✅ | Received ✅ |

> 📌 *Fill in with your actual measured values.*

---

## ⚠️ Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| Current reads garbage / always high | ADC noise or wrong sensitivity | Average more samples; verify `ACS712_SENSITIVITY` for your module |
| Relay doesn't trigger | Wrong relay logic | Swap `HIGH`/`LOW` in `triggerFault()` and `clearFault()` |
| Telegram alert not sent | Wrong token or Chat ID | Re-check `config.h`; verify bot is not blocked |
| Wi-Fi keeps dropping | Weak signal or power issue | Move closer to router; use stable 5V supply |
| False triggers at startup | ACS712 offset drift | Add 2-second warm-up delay in `setup()` before monitoring |

---

## 📚 References

- ACS712 Datasheet — Allegro MicroSystems
- ESP32 Technical Reference Manual — Espressif Systems
- [UniversalTelegramBot Library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) — Brian Lough
- [ArduinoJson](https://arduinojson.org/) — Benoit Blanchon

---

## 👨‍💻 Author

> ✏️ *Add your name, institution (Amrita Vishwa Vidyapeetham), branch (B.Tech ECE), and year here.*

---

## 📝 License

This project is intended for educational use. Feel free to adapt it for your own coursework or experiments.
