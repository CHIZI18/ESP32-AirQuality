#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "GOODLUCK";
const char* password = "12341234";

// AWS API Gateway endpoint
const char* server = "https://qtnbdiozq1.execute-api.eu-north-1.amazonaws.com/dev/sendData";

// Sensor pins
int mq135Pin = 34;  // MQ135 for CO2
int mq7Pin = 32;    // MQ7 for CO

// Device and location info
const char* device_id = "ESP32-001";
const char* locationName = "CURRENT LOCATION";  // ✅ Always sends "CURRENT LOCATION"

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // ⚠️ Bypass certificate validation (not secure, for testing only)
  client.setInsecure();
}

void loop() {
  // Read sensors
  int mq135Value = analogRead(mq135Pin);
  int mq7Value = analogRead(mq7Pin);

  // Convert raw values to ppm (simple scaling for demo)
  float co2_ppm = mq135Value * (1000.0 / 4095.0);
  float co_ppm = mq7Value * (50.0 / 4095.0);

  // Generate timestamp (milliseconds since boot)
  unsigned long timestamp = millis();

  // JSON payload in backend-friendly format
  String payload = "{";
  payload += "\"device_id\":\"" + String(device_id) + "\",";
  payload += "\"location\":\"" + String(locationName) + "\",";
  payload += "\"CO2\":" + String(co2_ppm) + ",";
  payload += "\"CO\":" + String(co_ppm) + ",";
  payload += "\"timestamp\":" + String(timestamp);
  payload += "}";

  Serial.println("📊 Sending payload: " + payload);

  // Send to AWS
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    https.begin(client, server);
    https.addHeader("Content-Type", "application/json");

    int code = https.POST(payload);

    if (code > 0) {
      Serial.println("✅ Cloud response code: " + String(code));
      Serial.println("🔁 Response body: " + https.getString());
    } else {
      Serial.println("❌ Cloud error: " + String(code));
    }
    https.end();
  }

  delay(15000); // Wait 15 seconds before next reading
}