#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "HX711.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(5, DHT11);
HX711 scale;

const int lightPinDigital = 6; // LM393 DO pin connected to D6
const int airPin = A1;
const int soundPin = 2;
const int greenLED = 7;
const int yellowLED = 8;
const int redLED = 9;
const int buzzer = 10;


bool criticalMode = false;
unsigned long lastToggleTime = 0;
bool showAlertScreen = true;

String criticalSensors = "";

void setup() {
  lcd.begin(16, 2);
  lcd.backlight();
  dht.begin();
  scale.begin(3, 4); // HX711 DT, SCK
  pinMode(soundPin, INPUT);
  pinMode(lightPinDigital, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(buzzer, LOW);
  scale.set_scale(197.23); // Optional: calibrate with known weight
  scale.tare();
}

void loop() {
  criticalSensors = "";
  bool warning = false;
  bool critical = false;

  // DHT11
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (temp < 30 || temp > 38) {
    critical = true;
    criticalSensors += "T";
  } else if (temp < 32 || temp > 36) {
    warning = true;
  }

  if (hum < 40 || hum > 80) {
    critical = true;
    criticalSensors += "H";
  } else if (hum < 50 || hum > 70) {
    warning = true;
  }

  // Air Quality
  int airVal = analogRead(airPin);
  if (airVal > 450) {
    critical = true;
    criticalSensors += "A";
  } else if (airVal > 350) {
    warning = true;
  }

  // Weight
  float weight = scale.get_units();
  if (weight > 1500) {
    critical = true;
    criticalSensors += "W";
  } else if (weight > 1000) {
    warning = true;
  }

  // Sound
  if (digitalRead(soundPin) == LOW) {
    critical = true;
    criticalSensors += "S";
  }

  // Light
 
bool lightState = digitalRead(lightPinDigital); // HIGH = bright, LOW = dark

if (lightState == HIGH) { // It's dark
  if (criticalSensors.indexOf("L") == -1) {
    critical = true;
    criticalSensors += "L";
  }
} else {
  // Optional: remove "L" from criticalSensors if light returns
  criticalSensors.replace("L", "");
}
  // Alert Handling
  if (critical) {
    criticalMode = true;
    digitalWrite(redLED, HIGH);
    digitalWrite(buzzer, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
  } else if (warning) {
    criticalMode = false;
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(buzzer, LOW);
  } else {
    criticalMode = false;
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(buzzer, LOW);
  }

  // LCD Display
  if (criticalMode) {
    if (millis() - lastToggleTime >= 15000) {
      showAlertScreen = !showAlertScreen;
      lastToggleTime = millis();
    }

    if (showAlertScreen) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Critical Alert");
      lcd.setCursor(0, 1);
      lcd.print(criticalSensors);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("T");
      lcd.print((int)temp);
      lcd.print(" H");
      lcd.print((int)hum);
      lcd.setCursor(0, 1);
      lcd.print("A");
      lcd.print(airVal);
      lcd.print(" W");
      lcd.print(weight, 1);
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T");
    lcd.print((int)temp);
    lcd.print(" H");
    lcd.print((int)hum);
    lcd.setCursor(0, 1);
    lcd.print("A");
    lcd.print(airVal);
    lcd.print(" W");
    lcd.print(weight, 1);
  }

  delay(500);
}

