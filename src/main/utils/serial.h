#ifndef SERIAL_H
#define SERIAL_H 

#include <avr/io.h>
#include "bit_utils.h"

class Serial
{
private:
    uint32_t ubrr;

public:
    Serial(uint32_t baudrate = 9600, uint32_t clk = 1843200)
    {
        ubrr = clk / 16 / (baud - 1);
    }

    void begin()
    {
        UBRR0H = (unsigned char)(ubrr >> 8);
        UBRR0L = (unsigned char) ubrr;

        SET_BIT(USCR0B, TXEN0);
        SET_BIT(USCR0B, RXEN0);

        UCSR0C = (1<<USBS0)|(3<<UCSZ00);
    }

    void transmit(unsigned char data)
    {
        while(!READ_BIT(UCSRna, UDREn));
        UDRn = data;
    }

    unsigned char receive(void)
    {
        while(!available());
        return UDRn;
    }

    bool available()
    {
        return READ_BIT(UCSRnA, RXCn);
    }
};


#endif