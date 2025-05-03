#ifndef HC05_H
#define HC05_H

#include "../utils/serial.h"

class HC05
{
private:
    Serial& serial_;

public:
    HC05(Serial& serial) : serial_(serial) 
    {
        serial_.begin();
    }

    void send(const char* str)
    {
        while (*str)
        {
            serial_.transmit(*str++);
        }
    }

    void receiveLine(char* buffer, uint8_t maxLen)
    {
        uint8_t i = 0;
        
        while (i < maxLen - 1)
        {
            if (serial_.available())
            {
                char c = serial_.receive();
                if (c == '\n') break;
                buffer[i++] = c;
            }
        }

        buffer[i] = '\0'; 
    }

    bool available()
    {
        return serial_.available();
    }
};

#endif
