#include <ESP8266WiFi.h>

// פרטי WiFi (התאם לפי הצורך)
const char* ssid = "AndroidAP32C2";
const char* password = "12345678";

// פונקציה לאתחול חיבור ה-WiFi
void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("מתחבר ל-WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  delay(5000);
  Serial.println();
  Serial.print("מחובר. כתובת IP: ");
  Serial.println(WiFi.localIP());
  delay(500);
}
