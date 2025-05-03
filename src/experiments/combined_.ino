#include <Arduino.h>

// Ultrasonic sensor pins
const int TRIG = 2;
const int ECHO = 3;

// Motor A pins
const int IN1 = 8;
const int IN2 = 9;
const int ENA = 10; // PWM

// Motor B pins
const int IN3 = 11;
const int IN4 = 12;
const int ENB = 5; // PWM

void setup()
{
    // Motor pin setup
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);

    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENB, OUTPUT);

    // Sensor setup
    pinMode(ECHO, INPUT);
    pinMode(TRIG, OUTPUT);
    digitalWrite(TRIG, LOW);

    Serial.begin(9600);
}

void move_forward()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 200);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 200);
}

void move_backward()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 200);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, 200);
}

void stop_both_motors()
{
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

float get_distance()
{
    float timing, distance;
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    timing = pulseIn(ECHO, HIGH, 30000); // 30ms timeout
    if (timing == 0)
        return -1; // Out of range or failed

    distance = (timing * 0.034) / 2;
    delay(50); // stabilize readings
    return distance;
}

void loop()
{
    Serial.println("\n--- NEW LOOP ---");

    float startDist = get_distance();
    if (startDist < 0)
    {
        Serial.println("Start distance out of range.");
        delay(2000);
        return;
    }

    Serial.print("Start Dist: ");
    Serial.print(startDist);
    Serial.println(" cm");

    Serial.println("Moving forward...");
    unsigned long tStart = millis();
    while ((get_distance() - startDist < 10.0) && (millis() - tStart < 5000))
    {
        move_forward();
    }
    stop_both_motors();

    float newDist = get_distance();
    Serial.print("New Dist: ");
    Serial.println(newDist >= 0 ? newDist : -1);

    delay(2000);

    Serial.println("Moving backward...");
    tStart = millis();
    while ((get_distance() > startDist + 1.0) && (millis() - tStart < 5000))
    {
        move_backward();
    }
    stop_both_motors();

    float backDist = get_distance();
    Serial.print("Back to Dist: ");
    Serial.println(backDist >= 0 ? backDist : -1);

    delay(5000); // Wait before next loop
}
