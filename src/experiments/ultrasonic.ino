#include <Arduino.h>

const int trig_pin = 2;
const int echo_pin = 3;

void setup()
{
  pinMode(echo_pin, INPUT);
  pinMode(trig_pin, OUTPUT);

  digitalWrite(trig_pin, LOW);
  Serial.begin(9600); // Start Serial Monitor
}

void loop()
{
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);

  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  timing = pulseIn(echo_pin, HIGH);
  distance = (timing * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
}