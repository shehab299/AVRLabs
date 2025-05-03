#include <NewPing.h>

#define TRIG_PIN 2       // Arduino Nano D2
#define ECHO_PIN 3       // Arduino Nano D4
#define MAX_DISTANCE 200 // cm

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// // HCSR04 sonar(PRESCALER_8);

// // Motor A pins
const int IN1 = 8;
const int IN2 = 9;
const int ENA = 10; // PWM

// Motor B pins
const int IN3 = 11;
const int IN4 = 12;
const int ENB = 5; // PWM

void setup()
{
    // Set all motor pins as outputs
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);

    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENB, OUTPUT);
    Serial.begin(9600);
}

void move_forward()
{
    // Move both motors forward
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 200);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 200);
}
void move_backword()
{
    // Move both motors backward
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 200);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, 200);
}

void stop_both_motors()
{
    // Stop both motors
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

void loop()
{

    Serial.println("NEW LOOOOP");
    Serial.println("--------------------------------------------------");

    float startDist = sonar.ping() / 100.0;

    if (startDist > 0)
    {
        Serial.print("Start Dist: ");
        Serial.print(startDist);
        Serial.println(" cm");
    }
    else
    {
        Serial.println("Out of range");
    }

    Serial.println("Moving forward");
    while (sonar.ping() / 100.0 - startDist < 10.0)
    {
        move_forward();
    }

    float new_dist = sonar.ping() / 100.0;

    if (new_dist > 0)
    {
        Serial.print("New Dist: ");
        Serial.print(new_dist);
        Serial.println(" cm");
    }
    else
    {
        Serial.println("Out of range");
    }

    stop_both_motors();

    delay(5000);
    Serial.println("Moving backword");
    // Move backward until near starting distance
    while (sonar.ping() / 100.0 > startDist + 1.0)
    {
        move_backword();
    }

    float _dist = sonar.ping() / 100.0;
    Serial.println(_dist);

    // Final stop
    stop_both_motors();
    delay(5000);
}
