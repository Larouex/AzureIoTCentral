#include <Arduino.h>
#define onboard 13
byte comdata;

void setup() {
  pinMode(onboard, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BLE Test");
  Serial.println("BLE Starting");
}

void loop() {
  while (Serial.available() > 0)  
  {
    Serial.println("BLE Test 1");
    digitalWrite(onboard, LOW);
    delay(5000);
    digitalWrite(onboard, HIGH);
    delay(5000);
    comdata = Serial.read();
    delay(2);
    Serial.write(comdata);
  }
}