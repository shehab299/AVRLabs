#ifndef HCSR04_H
#define HCSR04_H

#include "../utils/bit_utils.h"
#include "../utils/timer.h"

#define F_CPU 16000000
#define SOUND_SPEED 34300.0f
#define TRIG_PIN PB0
#define ECHO_PIN PB1

class HCSR04
{
private:
    PRESCALER prescaler;
    Timer *timer;
    int triggerPin, echoPin;

public:
    HCSR04(int triggerPin = TRIG_PIN, int echoPin = ECHO_PIN, PRESCALER prescaler = PRESCALER_1)
        : triggerPin(triggerPin), echoPin(echoPin), prescaler(prescaler)
    {
        SET_BIT(DDRB, triggerPin);
        CLEAR_BIT(DDRB, echoPin);
        SET_BIT(PORTB, triggerPin);
        timer = new Timer(F_CPU, prescaler);
    }

    float computeEcho() {
        float start = timer->getCurrentTime();
        while (READ_BIT(PINB, echoPin));
        float end = timer->getCurrentTime();
        return (end - start) * (prescaler / F_CPU);
    }

    float measureDistance(int timeout = 30000) {
        timer->setDelay(1);
        SET_BIT(PORTB, triggerPin);
        timer->setDelay(1);
        CLEAR_BIT(PORTB, triggerPin);
    
        int startTime = timer->getCurrentTime();
    
        while (!READ_BIT(PINB, echoPin) && timer->getCurrentTime() - startTime < timeout);
        if (timer->getCurrentTime() - startTime >= timeout) {
            return -1;
        }
        
        float travelTime = computeEcho();
        return (travelTime * SOUND_SPEED) / 2;
    }

    float measureSpeed(int timeout = 30000) {
        float start = measureDistance(timeout);
        timer->setDelay(1000);
        float end = measureDistance(timeout);
        return (end - start) / 1.0;
    }
};

#endif