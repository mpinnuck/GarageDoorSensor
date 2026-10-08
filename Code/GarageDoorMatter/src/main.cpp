// main.cpp  (GarageDoorMatter)
// Garage door open/closed sensor as a Matter contact sensor (Wi-Fi).
// Board: Seeed XIAO ESP32C3, Arduino ESP32 core 3.x (Matter library).
//
// Hardware (schematic rev 10):
//   D1 (GPIO3): door input, 10k pull-up to 3V3, 100nF to GND,
//               1k series + 1N4148 clamps, from J3 (JST PH) -> panel jack -> MC-38 reed.
//   Reed closed (magnet present, door shut) = LOW.
//   BOOT button (GPIO9): hold 5 s to decommission (factory reset Matter).
//
// Commissioning: over Bluetooth (CHIPoBLE). The Apple/Google app sends the
// Wi-Fi credentials, so none are stored in this sketch.
//
// Arduino IDE settings:
//   Board:              XIAO_ESP32C3
//   Partition Scheme:   Huge APP (3MB No OTA/1MB SPIFFS)
//   Erase All Flash Before Sketch Upload: Enabled for the FIRST upload only,
//                       then Disabled (otherwise every upload wipes the pairing).

#include <Arduino.h>
#include <Matter.h>

MatterContactSensor GarageDoor;

const uint8_t DOOR_PIN = 3;     // XIAO D1
const uint8_t BUTTON_PIN = 9;   // XIAO BOOT button

const uint32_t DEBOUNCE_MS = 500;        // door must be stable this long
const uint32_t DECOMMISSION_HOLD_MS = 5000;

// Reed closed (LOW) = door shut = contact closed (true)
static bool readDoorClosed() {
  return digitalRead(DOOR_PIN) == LOW;
}

static void printPairingInfo() {
  Serial.println();
  Serial.println("Matter device not commissioned yet.");
  Serial.printf("Manual pairing code: %s\r\n", Matter.getManualPairingCode().c_str());
  Serial.printf("QR code (open in a browser): %s\r\n", Matter.getOnboardingQRCodeUrl().c_str());
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(DOOR_PIN, INPUT);            // external 10k pull-up fitted
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Matter.setProductName("Garage Door");   // must be set before Matter.begin()
  Matter.setDeviceName("Garage Door");

  GarageDoor.begin();                  // endpoint first...
  Matter.begin();                      // ...then the Matter stack

  // Boolean-state sensors must be set after Matter.begin()
  GarageDoor.setContact(readDoorClosed());
  Serial.printf("Initial door state: %s\r\n", GarageDoor ? "CLOSED" : "OPEN");

  if (!Matter.isDeviceCommissioned()) {
    printPairingInfo();
    uint32_t lastPrint = millis();
    while (!Matter.isDeviceCommissioned()) {
      delay(100);
      if (millis() - lastPrint > 30000) {   // reprint every 30 s
        printPairingInfo();
        lastPrint = millis();
      }
    }
    Serial.println("Commissioned. Connected to the Matter fabric.");
  }
}

void loop() {
  // --- Log Wi-Fi connect/disconnect ---
  static int lastWifi = -1;
  int wifiNow = Matter.isWiFiConnected() ? 1 : 0;
  if (wifiNow != lastWifi) {
    lastWifi = wifiNow;
    Serial.println(wifiNow ? "Wi-Fi CONNECTED" : "Wi-Fi not connected");
  }

  // --- Door input with debounce ---
  static bool lastRaw = readDoorClosed();
  static uint32_t lastChange = millis();

  bool raw = readDoorClosed();
  if (raw != lastRaw) {
    lastRaw = raw;
    lastChange = millis();
  }
  if ((millis() - lastChange) >= DEBOUNCE_MS && raw != GarageDoor.getContact()) {
    GarageDoor.setContact(raw);
    Serial.printf("Garage door %s\r\n", raw ? "CLOSED" : "OPEN");
  }

  // --- BOOT button long hold: decommission ---
  static uint32_t pressStart = 0;
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (pressStart == 0) pressStart = millis();
    if (millis() - pressStart > DECOMMISSION_HOLD_MS) {
      Serial.println("Decommissioning. The device must be added again.");
      Matter.decommission();          // factory resets and restarts
      pressStart = 0;
    }
  } else {
    pressStart = 0;
  }

  delay(20);
}
