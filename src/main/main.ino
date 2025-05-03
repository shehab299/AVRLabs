#include "drivers/HC-05/HC05.h"
#include "drivers/HC-SR04/HC-SR04.h"
#include "utils/serial.h"
#include "utils/bit_utils.h"

#define CAR_LENGTH 23
#define CAR_WIDTH 18

// Motor A pins (PORTD)
#define IN1 PD2
#define IN2 PD3
#define ENA PD6  // OC0A

// Motor B pins (PORTD)
#define IN3 PD4
#define IN4 PD7
#define ENB PD5  // OC0B

#define LEFT_TRIG PB0  
#define LEFT_ECHO PB1
#define RIGHT_TRIG PB2
#define RIGHT_ECHO PB3
#define FORWARD_TRIG PB4
#define FORWARD_ECHO PB5

#define MOTOR_SPEED 180
#define DEGREE_TIME_RATIO 4.5
#define MOVE_DURATION 200
#define DISTANCE_TOLERANCE 0.5
#define MOVE_THRESHOLD 10

unsigned long moveStartTime = 0;
int moveDuration = 0;
char moveMode = '\0';
bool isMoving = false;

float forward_distance = 0;
float left_distance = 0;
float right_distance = 0;

Serial serialBluetooth;
HC05 hc05(serialBluetooth);

HCSR04 forwardSensor(FORWARD_TRIG, FORWARD_ECHO, PRESCALER_8);
HCSR04 leftSensor(LEFT_TRIG, LEFT_ECHO,PRESCALER_8);
HCSR04 rightSensor(RIGHT_TRIG, RIGHT_ECHO,PRESCALER_8);

void setMotorSpeed(uint8_t ena_speed, uint8_t enb_speed) {
  OCR0A = ena_speed;
  OCR0B = enb_speed;
}

void setup()
{
  hc05 = HC05(serialBluetooth);

  SET_BIT(DDRB, LEFT_TRIG);
  CLEAR_BIT(DDRB, LEFT_ECHO);
  CLEAR_BIT(PORTB, LEFT_TRIG);

  SET_BIT(DDRB, RIGHT_TRIG);
  CLEAR_BIT(DDRB, RIGHT_ECHO);
  CLEAR_BIT(PORTB, RIGHT_TRIG);

  SET_BIT(DDRB, FORWARD_TRIG);
  CLEAR_BIT(DDRB, FORWARD_ECHO);
  CLEAR_BIT(PORTB, FORWARD_TRIG);

  SET_BIT(DDRD, IN1);
  SET_BIT(DDRD, IN2);
  SET_BIT(DDRD, IN3);
  SET_BIT(DDRD, IN4);
  SET_BIT(DDRD, ENA);
  SET_BIT(DDRD, ENB);

  // Setup PWM - Fast PWM mode, non-inverting, prescaler 8
  TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1) | (1 << COM0B1);
  TCCR0B = (1 << CS01); // prescaler 8

  setMotorSpeed(0, 0);
}

void loop()
{
  if (hc05.available())
  {
    char value = serialBluetooth.receive();
    Serial.print("Bluetooth Received: ");
    Serial.println(value);

    if (value == 'P')
    {
      Park_main();

      if (isMoving && (millis() - moveStartTime >= moveDuration))
      {
        Stop();
        isMoving = false;
      }
    }
  }
}

bool park_trial()
{
  forward_distance = forwardSensor.measureDistance();
  left_distance = leftSensor.measureDistance();
  right_distance = rightSensor.measureDistance();

  if (left_distance > CAR_LENGTH + DISTANCE_TOLERANCE)
    return Park('L');
  else if (right_distance > CAR_LENGTH + DISTANCE_TOLERANCE)
    return Park('R');
  else if (forward_distance > CAR_WIDTH + DISTANCE_TOLERANCE)
    start_motion('F');
  else
  {
    if (left_distance > right_distance + DISTANCE_TOLERANCE)
      start_turn('L', 180);
    else
      start_turn('R', 180);
  }

  return false;
}

void start_motion(char dir)
{
  float dist = 0;
  if (dir == 'F' || dir == 'L' || dir == 'R')
  {
    switch (dir)
    {
      case 'F': dist = forwardSensor.measureDistance(); break;
      case 'L': dist = leftSensor.measureDistance(); break;
      case 'R': dist = rightSensor.measureDistance(); break;
    }

    if (dist > 0 && dist < MOVE_THRESHOLD + DISTANCE_TOLERANCE)
    {
      Serial.println("Obstacle detected! Skipping forward.");
      return;
    }
  }

  moveDuration = MOVE_DURATION;
  moveStartTime = millis();
  moveMode = dir;
  isMoving = true;

  setMotorSpeed(MOTOR_SPEED, MOTOR_SPEED);

  switch (dir)
  {
    case 'F':
      SET_BIT(PORTD, IN1); CLEAR_BIT(PORTD, IN2);
      SET_BIT(PORTD, IN3); CLEAR_BIT(PORTD, IN4);
      break;
    case 'B':
      CLEAR_BIT(PORTD, IN1); SET_BIT(PORTD, IN2);
      CLEAR_BIT(PORTD, IN3); SET_BIT(PORTD, IN4);
      break;
    case 'L':
      SET_BIT(PORTD, IN1); CLEAR_BIT(PORTD, IN2);
      CLEAR_BIT(PORTD, IN3); SET_BIT(PORTD, IN4);
      break;
    case 'R':
      CLEAR_BIT(PORTD, IN1); SET_BIT(PORTD, IN2);
      SET_BIT(PORTD, IN3); CLEAR_BIT(PORTD, IN4);
      break;
  }
}

void start_turn(char direction, int degrees)
{
  moveDuration = degrees * DEGREE_TIME_RATIO;
  moveStartTime = millis();
  moveMode = 'T';
  isMoving = true;

  setMotorSpeed(MOTOR_SPEED, MOTOR_SPEED);

  if (direction == 'L')
  {
    SET_BIT(PORTD, IN1); CLEAR_BIT(PORTD, IN2);
    CLEAR_BIT(PORTD, IN3); SET_BIT(PORTD, IN4);
  }
  else if (direction == 'R')
  {
    CLEAR_BIT(PORTD, IN1); SET_BIT(PORTD, IN2);
    SET_BIT(PORTD, IN3); CLEAR_BIT(PORTD, IN4);
  }
  else
  {
    Serial.println("Invalid direction for rotation!");
    Stop();
    isMoving = false;
  }
}

void Stop()
{
  CLEAR_BIT(PORTD, IN1);
  CLEAR_BIT(PORTD, IN2);
  CLEAR_BIT(PORTD, IN3);
  CLEAR_BIT(PORTD, IN4);
  setMotorSpeed(0, 0);
  isMoving = false;
}

void Park_main()
{
  while (true)
  {
    bool success = park_trial();
    if (success)
    {
      Serial.println("Parking completed!");
      break;
    }
    else
    {
      Serial.println("Searching for empty places in the forward direction...");
    }
  }
}

bool Park(char direction)
{
  Serial.println("Parking in the " + String(direction) + " direction.");
  start_turn(direction, 90);

  forward_distance = forwardSensor.measureDistance();
  float start_position = forward_distance;
  float current_position = forward_distance;

  while (forward_distance > DISTANCE_TOLERANCE)
  {
    start_motion('F');
    forward_distance = forwardSensor.measureDistance();
    Serial.println("Moving forward: " + String(forward_distance) + " cm");

    current_position -= (start_position - forward_distance);
    if (current_position < start_position - CAR_LENGTH + DISTANCE_TOLERANCE)
    {
      Serial.println("Parking completed!");
      Stop();
      return true;
    }
  }

  Serial.println("Parking failed!");
  Stop();
  return false;
}
