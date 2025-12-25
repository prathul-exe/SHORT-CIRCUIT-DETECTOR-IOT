#include <WiFi.h>
#include <HTTPClient.h>

#define ACS_PIN     34
#define RELAY_PIN   26
#define BUZZER_PIN  27
#define LED_PIN     25

// ===== WiFi =====
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== Telegram =====
String botToken = "TELEGRAM_BOT_TOKEN";
String chatID   = "TELEGRAM_CHAT_ID";

// ===== ACS712 CONFIG =====
float sensitivity = 0.185;      // ACS712 5A
float thresholdCurrent = 3.00;   // Threshold Value

float zeroCurrentVoltage = 0;
unsigned long lastAlertTime = 0;

void sendTelegram(String msg) {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + botToken +
               "/sendMessage?chat_id=" + chatID +
               "&text=" + msg;
  http.begin(url);
  http.GET();
  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Relay OFF initially (active LOW)
  digitalWrite(RELAY_PIN, HIGH);

  // ADC setup
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // WiFi connect
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ===== ACS712 AUTO CALIBRATION =====
  long sum = 0;
  for (int i = 0; i < 1000; i++){
    sum += analogRead(ACS_PIN);
    delay(2);
  }
  float avgADC = sum / 1000.0;
  zeroCurrentVoltage = (avgADC / 4095.0) * 3.3;

  Serial.print("Zero current voltage = ");
  Serial.println(zeroCurrentVoltage, 3);
}

void loop() {
  // ===== AVERAGING (KEY FIX) =====
  const int samples = 50;
  float voltageSum = 0;

  for (int i = 0; i < samples; i++) {
    int adc = analogRead(ACS_PIN);
    voltageSum += (adc / 4095.0) * 3.3;
    delay(2);
  }

  float voltage = voltageSum / samples;
  float current = abs((voltage - zeroCurrentVoltage) / sensitivity);

  Serial.print("Current = ");
  Serial.print(current, 3);
  Serial.println(" A");

  // ===== SHORT CIRCUIT DETECTION =====
  if (current > thresholdCurrent) {

    digitalWrite(RELAY_PIN, HIGH); // power cut (active LOW relay)
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 5000);
    delay(1000);
    noTone(BUZZER_PIN);

    if (millis() - lastAlertTime > 15000) {
      sendTelegram("⚠ SHORT CIRCUIT DETECTED!\nPower Cut OFF.");
      lastAlertTime = millis();
    }

  } else {
    digitalWrite(RELAY_PIN, LOW);  // power ON
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  delay(300);
}
