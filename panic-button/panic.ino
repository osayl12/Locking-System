#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// WiFi credentials - edit before upload
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// WebSocket server
const char* websocket_host = "192.168.4.1"; // change to server IP
const uint16_t websocket_port = 8080;
const char* websocket_path = "/";

// Device ID
String device_id = "panic_001";

// Pins
const uint8_t BUTTON_PIN = D3; // interrupt pin
const uint8_t LED_PIN = D4;    // red LED
const uint8_t BUZZER_PIN = D5; // buzzer (PWM)

// State machine
enum State {Idle, Panic, Neutral};
volatile State state = Idle;
volatile bool panicTriggered = false;

WebSocketsClient webSocket;

// timing
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 10UL * 60UL * 1000UL; // 10 minutes

// led control
unsigned long lastLedToggle = 0;
bool ledOn = false;

// buzzer control
unsigned long buzzerStart = 0;

// Forward
void sendStatus(State s);

void ICACHE_RAM_ATTR handleButtonInterrupt() {
  // only trigger if currently Idle
  if (state == Idle) {
    panicTriggered = true;
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket connected");
      sendStatus(state);
      break;
    case WStype_TEXT: {
      Serial.printf("Got WS text: %s\n", payload);
      StaticJsonDocument<200> doc;
      DeserializationError err = deserializeJson(doc, payload);
      if (!err) {
        const char* cmd = doc["cmd"];
        if (cmd && strcmp(cmd, "cancel") == 0) {
          // only cancel if in Panic
          if (state == Panic) {
            state = Neutral;
            Serial.println("Cancelled via server");
            sendStatus(state);
          }
        }
      }
      break;
    }
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  analogWrite(BUZZER_PIN, 0);

  // attach interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi failed");
  }

  // WebSocket
  webSocket.begin(websocket_host, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  lastHeartbeat = millis();
}

void loop() {
  webSocket.loop();

  // check interrupt flag
  if (panicTriggered) {
    noInterrupts();
    panicTriggered = false;
    interrupts();
    // move to Panic
    state = Panic;
    sendStatus(state);
  }

  // State behaviors
  if (state == Panic) {
    // fast LED blink (100ms)
    unsigned long now = millis();
    if (now - lastLedToggle >= 100) {
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
      lastLedToggle = now;
    }
    // buzzer rising/falling tone
    unsigned long t = millis() - buzzerStart;
    // simple triangle up/down using PWM freq
    int toneVal = 200 + (int)( (sin(t/1000.0 * 3.1415 * 2) + 1.0) * 200 );
    analogWrite(BUZZER_PIN, toneVal);
  } else if (state == Neutral) {
    // steady on? let's blink slow like 1s
    unsigned long now = millis();
    if (now - lastLedToggle >= 1000) {
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
      lastLedToggle = now;
    }
    analogWrite(BUZZER_PIN, 0);
    // do not allow re-trigger via button
  } else { // Idle
    // slow heartbeat blink every 10 minutes: one short blink
    unsigned long now = millis();
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
      // short blink
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      lastHeartbeat = now;
      sendStatus(state);
    }
    analogWrite(BUZZER_PIN, 0);
  }

  // keep ws alive
  delay(10);
}

void sendStatus(State s) {
  StaticJsonDocument<200> doc;
  doc["device_id"] = device_id;
  const char* st = "idle";
  if (s == Panic) st = "panic";
  else if (s == Neutral) st = "neutral";
  doc["status"] = st;
  String out;
  serializeJson(doc, out);
  webSocket.sendTXT(out);
}
