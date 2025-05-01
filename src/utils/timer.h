#ifndef TIMER_H
#define TIMER_H 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "bit_utils.h"

#define MAX_DELAY (1 << 16) - 1

class Timer
{
private:
int F_CPU;
int prescaler;

void computeTicks(int delay, int &overflow, int &ticks)
{
    overflow = delay / (MAX_DELAY * (prescaler / F_CPU));
    ticks = delay % (MAX_DELAY * (prescaler / F_CPU));
}
public:
    static volatile int overflowCount;

    Timer(int F_CPU = 16000000, int prescaler = PRESCALER_1)
        : F_CPU(F_CPU), prescaler(prescaler)
    {
        switch (prescaler)
        {
        case PRESCALER_1:
            SET_BIT(TCCR1B, CS00);
            break;
        case PRESCALER_8:
            SET_BIT(TCCR1B, CS01);
            break;
        case PRESCALER_64:
            SET_BITS(TCCR1B, (1 << CS00) | (1 << CS01));
            break;
        case PRESCALER_256:
            SET_BIT(TCCR1B, CS02);
            break;
        case PRESCALER_1024:
            SET_BITS(TCCR1B, (1 << CS00) | (1 << CS01) | (1 << CS02));
            break;
        default:
            break;
        }

        TCNT1 = 0;
        overflowCount = 0;
        SET_BIT(TIMSK1, TOIE1);
        sei();
    }

    void setDelay(int delay)
    {
        TCNT1 = 0;
        overflowCount = 0;
        int overflow, ticks;
        computeTicks(delay, overflow, ticks);

        while(true) {
            if (overflowCount >= overflow && TCNT1 >= ticks) break;
        }
    }

    int getCurrentTime()
    {
        return overflowCount * (MAX_DELAY * (prescaler / F_CPU)) + TCNT1;
    }
};

volatile int Timer::overflowCount = 0;

ISR(TIMER1_OVF_vect)
{
    Timer::overflowCount++;
}

#endif