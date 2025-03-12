#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <NX7Seg.h>

// הצהרת הפונקציה setupWiFi שמוגדרת בקובץ LockWifi_setup.ino
extern void setupWiFi();

// הגדרות פינים (התאם לפי הצורך)
#define LATCH_DIO D6
#define CLK_DIO   D5
#define DATA_DIO  D7
#define LOCK_PIN  D4  // מחובר למעגל ריליי או מופעל לשחרור הנעילה

// קוד סודי קבוע בת 4 ספרות (מאוחסן מראש)
String secretCode = "1234";
// הקוד המוצג – בתחילה כל הספרות מוסתרות (מוחלפים במקפים)
String displayedCode = "----";

// יצירת מופע לתצוגת 7-סגמנטים ולשרת אינטרנט
nx7seg display(LATCH_DIO, CLK_DIO, DATA_DIO, false);
ESP8266WebServer server(80);

// פונקציה לשליפת נתונים מה-API המרוחק באמצעות GET
int GetData() {
  int ret = -1;
  WiFiClient client;
  HTTPClient http;
  // בניית כתובת URL עם ערוץ 2 (כפי שמוגדר בבקר החידה)
  String dataURL = "ACT=GET&DEV=1121&CH=2";
  String url = String("http://api.kits4.me/GEN/api.php?") + dataURL;
  http.begin(client, url);
  int httpCode = http.GET();
  Serial.print("קוד GET מה-HTTP: ");
  Serial.println(httpCode);
  if (httpCode == HTTP_CODE_OK) {
    String res = http.getString();
    Serial.print("תגובה: ");
    Serial.println(res);
    ret = res.toInt();
  }
  http.end();
  return ret;
}

//
// מטפלי השרת
//

// דף ראשי: מציג טופס לפתיחת נעילה ידנית עם עיצוב משופר
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>פתח דלת</title>";
  html += "<style>";
  html += "body { background-color: #f0f0f0; font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }";
  html += "h1 { color: #333; }";
  html += "input[type='text'] { padding: 10px; font-size: 16px; border: 1px solid #ccc; border-radius: 4px; width: 200px; }";
  html += "input[type='submit'] { padding: 10px 20px; font-size: 16px; border: none; border-radius: 4px; background-color: #4CAF50; color: white; cursor: pointer; margin-top: 10px; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>הזן את הקוד הסודי לפתיחת הדלת</h1>";
  html += "<form action='/unlock' method='get'>";
  html += "<input type='text' name='code' maxlength='4' placeholder='הכנס קוד'>";
  html += "<br>";
  html += "<input type='submit' value='פתח דלת'>";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// פתיחת נעילה ידנית: בודקת את הקוד שהוזן מהטופס
void handleUnlock() {
  if (server.hasArg("code")) {
    String code = server.arg("code");
    Serial.print("הקוד שהוזן: ");
    Serial.println(code);
    if (code == secretCode) {
      // עדכון התצוגה להצגת הקוד הסודי המלא
      displayedCode = secretCode;
      display.write((char*)displayedCode.c_str(), displayedCode.length());
      display.refresh(500);
      
      // שחרור הנעילה
      digitalWrite(LOCK_PIN, HIGH);
      String response = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>נעילה פתוחה</title></head>";
      response += "<body style='background-color:#f0f0f0; text-align:center; margin-top:50px; font-family:Arial, sans-serif;'>";
      response += "<h1>הדלת נפתחה!</h1></body></html>";
      server.send(200, "text/html", response);
      Serial.println("הדלת נפתחה!");
      delay(5000); // הדלת פתוחה למשך 5 שניות
      digitalWrite(LOCK_PIN, LOW);
      Serial.println("הדלת נסגרה מחדש.");
    } else {
      String response = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>קוד שגוי</title></head>";
      response += "<body style='background-color:#f0f0f0; text-align:center; margin-top:50px; font-family:Arial, sans-serif;'>";
      response += "<h1>קוד שגוי!</h1></body></html>";
      server.send(200, "text/html", response);
      Serial.println("קוד שגוי.");
    }
  } else {
    server.send(400, "text/html", "בקשה שגויה");
  }
}

//
// פונקציות אתחול
//

// מאתחל את פיני תצוגת 7-סגמנטים
void setup_7seg() {
  pinMode(LATCH_DIO, OUTPUT);
  pinMode(CLK_DIO, OUTPUT);
  pinMode(DATA_DIO, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW); // וודא שהדלת נעולה בהתחלה

  // אתחול תצוגת 7-סגמנטים
  setup_7seg();
  display.clear();
  display.write((char*)displayedCode.c_str(), displayedCode.length());
  display.refresh(500);

  // אתחול חיבור ה-WiFi באמצעות הפונקציה מהקובץ LockWifi_setup.ino
  setupWiFi();

  // הגדרת נקודות קצה לשרת האינטרנט
  server.on("/", handleRoot);
  server.on("/unlock", handleUnlock);
  server.begin();
  Serial.println("שרת ה-HTTP הופעל");
}

void loop() {
  server.handleClient();
  
  // שליפת נתונים מה-API המרוחק כל 2 שניות לעדכון מצב החידה
  static unsigned long lastPoll = 0;
  if (millis() - lastPoll > 2000) {
    int puzzleVal = GetData();
    // אם הערך שהוחזר תקף ובין 1 ל-4...
    if (puzzleVal >= 1 && puzzleVal <= 4) {
      // עדכון הספרה המתאימה בתצוגה אם עדיין לא נחשפה
      if (displayedCode.charAt(puzzleVal - 1) != secretCode.charAt(puzzleVal - 1)) {
        displayedCode.setCharAt(puzzleVal - 1, secretCode.charAt(puzzleVal - 1));
        Serial.print("קוד מוצג מעודכן: ");
        Serial.println(displayedCode);
      }
    }
    lastPoll = millis();
  }
  
  // רענון רציף של תצוגת 7-סגמנטים
  display.write((char*)displayedCode.c_str(), displayedCode.length());
  display.refresh(5);
  delay(5);
  
  // אם כל הספרות נחשפות, שחרור הנעילה אוטומטי ואיפוס התצוגה
  if (displayedCode == secretCode) {
    Serial.println("כל הספרות נחשפו. פותח דלת באופן אוטומטי.");
    digitalWrite(LOCK_PIN, HIGH);
    delay(5000);  // הדלת פתוחה למשך 5 שניות
    digitalWrite(LOCK_PIN, LOW);
    Serial.println("הדלת נסגרה מחדש. מאפס את התצוגה.");
    displayedCode = "----";  // איפוס התצוגה
  }
}
