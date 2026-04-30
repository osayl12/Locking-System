#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// הצהרת הפונקציה setupWiFi שמוגדרת ב-LockWifi_setup.ino
extern void setupWiFi();

// כתובת API מרחוק ופרמטרי השרת
const char* apiURL = "http://api.kits4.me/GEN/api.php";
const String DEV = "1121";  // מזהה המכשיר
const String CH = "2";      // ערוץ

// ------------------ הגדרות המולטיפלקסר ------------------
// אזהרה: MUX_PIN_A (D7) זהה ל-FAN_PIN — ודא שזה מכוון או שנה פין אחד
#define MUX_PIN_A D7
#define MUX_PIN_B D6
#define MUX_PIN_C D5
#define MUX_PIN_IN A0

// ------------------ DHT ומאוורר ------------------
#define DHT_PIN D4
// אזהרה: FAN_PIN (D7) חולק עם MUX_PIN_A — החידות רצות בסדר, אך ודא שאין התנגשות חומרה
#define FAN_PIN D7
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// ------------------ LED וכפתורים לחידה 3 ------------------
// אזהרה: ledPins[0]=D1 ו-[1]=D2 חולקים עם ECHO_PIN/TRIG_PIN; ledPins[3]=D4 חולק עם DHT_PIN
// החידות רצות בסדר ולכן אין שימוש בו-זמני, אך שנה פינים אם יש בעיה בחומרה
const int ledPins[4] = {D1, D2, D3, D4};
// אזהרה: buttonPins[3]=D8 (GPIO15) — לפין זה יש נגד pull-down מובנה; INPUT_PULLUP לא יעבוד כראוי
// שנה לפין אחר (למשל D0) אם הכפתור לא מגיב
const int buttonPins[4] = {D5, D6, D7, D8};

// ------------------ פיני חיישן אולטרסוניק לחידה 4 ------------------
#define TRIG_PIN D2
#define ECHO_PIN D1

//
// פונקציה: קריאת ערוץ מהמולטיפלקסר
//
int ReadMuxChannel(byte chnl) {
  int a = (bitRead(chnl, 0) > 0) ? HIGH : LOW;
  int b = (bitRead(chnl, 1) > 0) ? HIGH : LOW;
  int c = (bitRead(chnl, 2) > 0) ? HIGH : LOW;
  
  digitalWrite(MUX_PIN_A, a);
  digitalWrite(MUX_PIN_B, b);
  digitalWrite(MUX_PIN_C, c);
  delay(1); // המתן להתייצבות הערוץ
  return analogRead(MUX_PIN_IN);
}

//
// שולח בקשת HTTP GET לעדכון מצב החידה.
// פורמט URL: ?ACT=SET&DEV=1121&CH=2&VAL=puzzleNum
//
void sendPuzzleUpdate(int puzzleNum) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = String(apiURL) + "?ACT=SET&DEV=" + DEV + "&CH=" + CH + "&VAL=" + String(puzzleNum);
    Serial.print("שולח עדכון חידה: ");
    Serial.println(url);
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.print("העדכון נשלח, קוד תגובה: ");
      Serial.println(httpCode);
      String response = http.getString();
      Serial.print("תגובה: ");
      Serial.println(response);
    } else {
      Serial.print("שליחת העדכון נכשלה, שגיאה: ");
      Serial.println(http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi לא מחובר");
  }
}

//
// חידה 1: חיישן אור דרך המולטיפלקסר
//
void puzzle1() {
  Serial.println("חידה 1: כוון את קריאת חיישן האור ל-20% מתחת למקסימום למשך 2 שניות.");
  const int threshold = 818;  // התאם את סף הקריאה לפי הצורך.
  unsigned long stableTime = 0;
  while (true) {
    int lightValue = ReadMuxChannel(4);  // קריאה מערוץ 4 במולטיפלקסר
    Serial.print("ערך חיישן האור: ");
    Serial.println(lightValue);
    if (lightValue < threshold) {
      if (stableTime == 0)
        stableTime = millis();
      else if (millis() - stableTime >= 2000) {
        Serial.println("חידה 1 נפתרה!");
        sendPuzzleUpdate(1);
        delay(1000);
        break;
      }
    } else {
      stableTime = 0;
    }
    delay(200);
  }
}

//
// חידה 2: בקרה על טמפרטורה
//
void puzzle2() {
  Serial.println("חידה 2: הורד את הטמפרטורה ב-2°C למשך 2 שניות.");
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);

  float initialTemp = NAN;
  while (isnan(initialTemp)) {
    initialTemp = dht.readTemperature();
    if (isnan(initialTemp)) {
      Serial.println("שגיאה בקריאת חיישן הטמפרטורה. מנסה שוב...");
      delay(1000);
    }
  }
  Serial.print("טמפרטורה התחלתית: ");
  Serial.println(initialTemp);

  unsigned long stableTime = 0;
  while (true) {
    float currentTemp = dht.readTemperature();
    if (isnan(currentTemp)) {
      Serial.println("קריאת חיישן נכשלה, מדלג על איטרציה זו.");
      delay(500);
      continue;
    }
    Serial.print("טמפרטורה נוכחית: ");
    Serial.println(currentTemp);
    if (currentTemp <= initialTemp - 2.0) {
      if (stableTime == 0)
        stableTime = millis();
      else if (millis() - stableTime >= 2000) {
        Serial.println("חידה 2 נפתרה!");
        sendPuzzleUpdate(2);
        delay(1000);
        break;
      }
    } else {
      stableTime = 0;
    }
    delay(500);
  }
  digitalWrite(FAN_PIN, LOW);
}

//
// חידה 3: רצף נוריות עם כפתורים (מהבהבת 8 פעמים)
//
void puzzle3() {
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  bool solved = false;
  while (!solved) {
    Serial.println("חידה 3: עקוב אחר רצף הנוריות המהבהבות.");

    int sequence[8];
    randomSeed(analogRead(0));
    for (int i = 0; i < 8; i++) {
      sequence[i] = random(0, 4);
    }

    Serial.print("הרצף: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(sequence[i]);
      Serial.print(" ");
      digitalWrite(ledPins[sequence[i]], HIGH);
      delay(500);
      digitalWrite(ledPins[sequence[i]], LOW);
      delay(300);
    }
    Serial.println();

    int userSequence[8];
    for (int i = 0; i < 8; i++) {
      Serial.print("מחכה ללחיצת כפתור עבור מיקום ");
      Serial.println(i + 1);
      bool pressed = false;
      while (!pressed) {
        for (int j = 0; j < 4; j++) {
          if (digitalRead(buttonPins[j]) == LOW) {
            userSequence[i] = j;
            Serial.print("כפתור ");
            Serial.print(j);
            Serial.println(" נלחץ.");
            delay(500);
            pressed = true;
            break;
          }
        }
      }
    }

    bool correct = true;
    for (int i = 0; i < 8; i++) {
      if (userSequence[i] != sequence[i]) {
        correct = false;
        break;
      }
    }

    if (correct) {
      Serial.println("חידה 3 נפתרה!");
      sendPuzzleUpdate(3);
      solved = true;
    } else {
      Serial.println("הרצף שגוי. מנסה שוב.");
    }
  }
  delay(1000);
}

//
// חידה 4: מדידת מרחק באולטרסוניקה
//
long readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void puzzle4() {
  Serial.println("חידה 4: החזק את ידך כ-20 ס\"מ מהחיישן למשך 2 שניות.");
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  unsigned long stableTime = 0;
  while (true) {
    long distance = readUltrasonicDistance();
    Serial.print("מרחק: ");
    Serial.print(distance);
    Serial.println(" ס\"מ");
    if (distance >= 18 && distance <= 22) {
      if (stableTime == 0)
        stableTime = millis();
      else if (millis() - stableTime >= 2000) {
        Serial.println("חידה 4 נפתרה!");
        sendPuzzleUpdate(4);
        delay(1000);
        break;
      }
    } else {
      stableTime = 0;
    }
    delay(300);
  }
}

//
// אתחול ולולאה
//
void setup() {
  Serial.begin(9600);
  
  // אתחל את פיני המולטיפלקסר
  pinMode(MUX_PIN_A, OUTPUT);
  pinMode(MUX_PIN_B, OUTPUT);
  pinMode(MUX_PIN_C, OUTPUT);
  pinMode(MUX_PIN_IN, INPUT);
  
  // אתחל את חיישן ה-DHT
  dht.begin();
  
  // התחבר ל-WiFi באמצעות הפונקציה מהקובץ LockWifi_setup.ino
  setupWiFi();
  
  // הרץ את החידות בסדר רציף.
  puzzle1();
  puzzle2();
  puzzle3();
  puzzle4();
  
  Serial.println("כל החידות נפתרו. כעת ניתן להזין את הקוד בממשק האינטרנט של בקר הנעילה.");
  while (true) {
    delay(1000);
  }
}

void loop() {
  // החידות מורצות רק פעם אחת ב-setup.
  while (true) {
    delay(1000);
  }
}
