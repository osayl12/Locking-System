#include <ESP8266WiFi.h>

// פרטי WiFi
const char* ssid = "AndroidAP32C2";
const char* password = "12345678";

// פונקציה לאתחול חיבור ה-WiFi.
void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("מתחבר ל-WiFi");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 30000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("חיבור WiFi נכשל! בדוק SSID וסיסמה.");
    return;
  }
  delay(500);
  Serial.print("מחובר. כתובת IP: ");
  Serial.println(WiFi.localIP());
}
