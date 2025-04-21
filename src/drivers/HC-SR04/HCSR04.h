#ifndef HCSR04_H
#define HCSR04_H
// #define TCNT1 0x85  DO WE REALLY NEED THIS?!!
#include <util/delay.h>
#include "../utils/bit_utils.h"

#define F_CPU 16000000
#define TRIG_PIN PB0
#define ECHO_PIN PB1

class HCSR04
{
private:
    PRESCALER prescaler_;
    
public:
    HCSR04(PRESCALER prescaler) : prescaler_(prescaler)
    {
        switch (prescaler_)
        {
        case PRESCALER_1:
            SET_BITS(TCCR1B, (1 << CS10));
            break;
        case PRESCALER_8:
            SET_BITS(TCCR1B, (1 << CS11));
            break;
        case PRESCALER_64:
            SET_BITS(TCCR1B, (1 << CS11) | (1 << CS10));
            break;
        case PRESCALER_256:
            SET_BITS(TCCR1B, (1 << CS12));
            break;
        case PRESCALER_1024:
            SET_BITS(TCCR1B, (1 << CS12) | (1 << CS11) | (1 << CS10));
            break;
        default:
            break;
        }
        SET_BIT(DDRB, TRIG_PIN);
        CLEAR_BIT(DDRB, ECHO_PIN);
        TCNT1 = 0;
    }

    void init() { //JUST IN CASE
        SET_BIT(DDRB, TRIG_PIN);
        CLEAR_BIT(DDRB, ECHO_PIN);
    }

    float measureDistance() {
        CLEAR_BIT(PORTB, TRIG_PIN);
        _delay_us(2);
        SET_BIT(PORTB, TRIG_PIN);
        _delay_us(10);
        CLEAR_BIT(PORTB, TRIG_PIN);
        while(!(PINB & (1 << ECHO_PIN)));
        
        TCNT1 = 0;
        
        // Wait for echo end
        while(PINB & (1 << ECHO_PIN)) {
            if(TCNT1 > 60000) {  // Timeout after ~30ms
                TCCR1B = 0;
                return -1.0;
            }
        }
        
        uint16_t count = TCNT1;
        TCCR1B = 0;

        // Distance = (Time * Sound Speed) / 2 = (count * prescaler * Sound Speed) / 2 * F_CPU 
        float dist = (float)count * prescaler_ * 34300.0f / (F_CPU * 2.0f);
        return dist;
    }
};

#endif
