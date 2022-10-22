#include <Arduino.h>
#define LED 2

void setup() {
  Serial.begin(921600);
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);
  Serial.println("LED is ON!");
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("LED is OFF!");
  delay(1000);
}
