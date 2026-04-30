# Electronic Lock System for Pool Room
## מערכת נעילה אלקטרונית לחדר בריכה

**Students:** Osail Hamed (208913798) · Walid Areida (204412654)

---

## Overview

An Arduino/ESP8266-based escape-room lock system. The user must solve four sequential hardware puzzles on the **Puzzle Controller** to progressively reveal a 4-digit code on a 7-segment display. Once all four digits are visible, the **Lock Controller** automatically unlocks the door. The code can also be entered manually through a local web interface.

---

## System Architecture

```
┌─────────────────────┐          HTTP GET/SET          ┌──────────────────────┐
│   Puzzle Controller  │ ──────── api.kits4.me ──────── │   Lock Controller    │
│   (ESP8266 #1)       │    VAL=1..4 on puzzle solved   │   (ESP8266 #2)       │
│                      │                                │                      │
│  4 Hardware Puzzles  │                                │  7-Segment Display   │
│  1. Light Sensor     │                                │  Door Relay          │
│  2. Temperature/Fan  │                                │  Web Server (port 80)│
│  3. LED Sequence     │                                │                      │
│  4. Ultrasonic Range │                                │                      │
└─────────────────────┘                                └──────────────────────┘
```

---

## Hardware Components

### Lock Controller (`LOCK/`)
| Component | Pin |
|-----------|-----|
| 7-Segment Latch | D6 |
| 7-Segment Clock | D5 |
| 7-Segment Data  | D7 |
| Door Relay      | D4 |

### Puzzle Controller (`Puzzle/`)
| Component | Pin(s) |
|-----------|--------|
| Multiplexer A/B/C | D7, D6, D5 |
| Multiplexer Input | A0 |
| DHT11 Temperature | D4 |
| Fan (Puzzle 2)    | D7 |
| LEDs (Puzzle 3)   | D1, D2, D3, D4 |
| Buttons (Puzzle 3)| D5, D6, D7, D8* |
| Ultrasonic TRIG   | D2 |
| Ultrasonic ECHO   | D1 |

> **Note:** Several pins are shared between puzzles (D1, D2, D4, D7). This works because puzzles run sequentially, never simultaneously. Pin D8 (GPIO15) has a hardware pull-down; if button 4 does not respond, remap it to D0.

---

## Puzzle Flow

| # | Puzzle | Goal |
|---|--------|------|
| 1 | Light Sensor | Lower light reading below 818 (20% of max) for 2 seconds |
| 2 | Temperature | Cool the room by 2 °C (measured by DHT11) for 2 seconds |
| 3 | LED Sequence | Repeat a random 8-step LED sequence using the buttons |
| 4 | Ultrasonic | Hold hand 18–22 cm from sensor for 2 seconds |

Each solved puzzle calls `api.kits4.me` with `VAL=1..4`. The Lock Controller polls this API every 2 seconds and reveals the matching digit on the display. When all 4 digits appear, the door opens automatically for 5 seconds.

---

## Web Interface

Connect to the same WiFi as the Lock Controller, then open a browser to the device's IP address. A form lets you enter the 4-digit code manually to unlock the door.

---

## Setup

### 1. Libraries Required (Arduino IDE)
- `ESP8266WiFi` (ESP8266 Arduino core)
- `ESP8266HTTPClient`
- `ESP8266WebServer`
- `DHT sensor library` (Adafruit)
- `NX7Seg` (included in `NX7Seg-master/`)
- `ASCIIDic` (included in `ASCIIDic-master/`)

> The `NX7Seg` and `ASCIIDic` libraries have been modified from their originals to correctly display all digits on this specific 7-segment screen.

### 2. WiFi Configuration
Edit the credentials in both WiFi setup files before flashing:

- `LOCK/Lock_WiFiSetup.ino` — for the Lock Controller
- `Puzzle/PuzzleWiFi_setup.ino` — for the Puzzle Controller

```cpp
const char* ssid     = "YOUR_NETWORK_NAME";
const char* password = "YOUR_PASSWORD";
```

### 3. Secret Code
The 4-digit unlock code is defined at the top of `LOCK/LOCK.ino`:
```cpp
String secretCode = "1234";
```
Change this before flashing.

### 4. Flash Order
1. Flash `LOCK/` sketch to Lock Controller ESP8266
2. Flash `Puzzle/` sketch to Puzzle Controller ESP8266
3. Power both devices — they connect to WiFi automatically
4. Lock Controller IP prints to Serial Monitor on boot

---

## Code Changes (April 2026)

The following bugs were fixed after initial upload:

| File | Fix |
|------|-----|
| `Puzzle/Puzzle.ino` | Added NaN guard on DHT11 reads in `puzzle2()` — failed sensor reads no longer cause a silent infinite loop |
| `Puzzle/Puzzle.ino` | Replaced recursive `puzzle3()` retry with a `while` loop — repeated wrong answers could cause a stack overflow crash |
| `Puzzle/Puzzle.ino` | Added warning comments on all shared-pin conflicts (D7, D4, D1, D2) and the D8/GPIO15 pull-down issue |
| `LOCK/Lock_WiFiSetup.ino` | Added 30-second timeout to WiFi connect loop — device no longer hangs forever on wrong credentials |
| `Puzzle/PuzzleWiFi_setup.ino` | Same 30-second WiFi timeout fix |

---

## Security Notes

- WiFi credentials and the secret code are stored in plain source code. Do not commit real passwords to a public repository.
- The web server has no authentication — anyone on the same WiFi can submit codes.
- The unlock endpoint has no rate limiting — brute-force is theoretically possible over the local network.

These are acceptable trade-offs for a classroom/escape-room project on an isolated hotspot.
