#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Fingerprint.h>

#define L_RX 4
#define L_TX 5
SoftwareSerial LoRaSerial(L_RX, L_TX); // RX, TX for LoRa

// Note: For AltSoftSerial, specific pins are used by the library:
// On many Arduino boards (Uno, Nano, Mega), it uses pins 8 (RX) and 9 (TX)
AltSoftSerial gpsSerial; // RX, TX for NEO6M GPS Module on pins 8 and 9

#define FINGERPRINT_RX 2
#define FINGERPRINT_TX 3
SoftwareSerial mySerial(FINGERPRINT_RX, FINGERPRINT_TX); // RX, TX for fingerprint sensor

TinyGPSPlus gps;
#define BAUDRATE 9600

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
bool fingerprintMatched = false;

void setup() {
  Serial.begin(BAUDRATE); // USB Serial
  while (!Serial); // Wait for serial port to connect
  delay(100);
  
  LoRaSerial.begin(BAUDRATE); // LoRa Software Serial
  gpsSerial.begin(9600); // NEO6M GPS Module with AltSoftSerial
  mySerial.begin(57600); // Fingerprint Sensor Software Serial

  // Check if reset button is pressed on startup
  if (digitalRead(7) == LOW) {
    resetFingerprintSensor(); // Reset fingerprint sensor
  }

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); } // Halt the program if the sensor is not found
  }
}

void loop() {
  // Check for fingerprint
  if (!fingerprintMatched) {
    getFingerprintID();
    delay(50); // Delay between fingerprint scans
  } else {
    // Read GPS data
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        displayInfo();
      }
    }
    
    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println(F("No GPS detected: check wiring."));
      while (true);
    }
  }
}

void displayInfo() {
  if (gps.location.isValid()) {
    String data = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "|";
    sendLoraData(data, 2);
    Serial.println(data);
    Serial.flush();
    delay(3000);
  }
}

void sendLoraData(String data, int address) {
  String myString = "AT+SEND=" + String(address) + "," + String(data.length()) + "," + data + "\r\n";
  char* buf = (char*) malloc(sizeof(char) * (myString.length() + 1));
  myString.toCharArray(buf, myString.length() + 1);
  LoRaSerial.write(buf);
  free(buf);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // Image taken successfully, proceed with fingerprint matching and identification
  p = finger.image2Tz();
  // Check for errors during image conversion
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Invalid fingerprint image");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // Image conversion successful, search for a matching fingerprint
  p = finger.fingerSearch();
  // Handle different search results
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint match found!");
    fingerprintMatched = true;
    // Here you can add code to send authentication signal or data over LoRa
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Fingerprint not found");
  } else {
    Serial.println("Unknown error");
  }

  return p;
}

void resetFingerprintSensor() {
  finger.begin(57600); // Restart the fingerprint sensor
  fingerprintMatched = false; // Reset fingerprint match status
  Serial.println("Fingerprint sensor reset successfully");
}
