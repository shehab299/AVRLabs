#ifndef BIT_UTILS_H
#define BIT_UTILS_H

#include <avr/io.h>

enum PRESCALER {
    PRESCALER_1 = 1,
    PRESCALER_8 = 8,
    PRESCALER_64 = 64,
    PRESCALER_256 = 256,
    PRESCALER_1024 = 1024
};
// Set a bit/bits
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))

#define SET_BITS(REG, BITS) ((REG) |= (BITS))

// Clear a bit/bits
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))

#define CLEAR_BITS(REG, BITS) ((REG) &= ~(BITS))

// Toggle a bit/bits
#define TOGGLE_BIT(REG, BIT) ((REG) ^= (1 << (BIT)))

#define TOGGLE_BITS(REG, BITS) ((REG) ^= (BITS))

// Read a bit (returns 0 or non-zero)
#define READ_BIT(REG, BIT) ((REG) & (1 << (BIT)))

// Set bit using pointer to volatile register
static inline void set_bit_ptr(volatile uint8_t *reg, uint8_t bit)
{
    *reg |= (1 << bit);
}

static inline void clear_bit_ptr(volatile uint8_t *reg, uint8_t bit)
{
    *reg &= ~(1 << bit);
}

static inline void clear_bits_ptr(volatile uint8_t *reg, uint8_t bits)
{
    *reg &= ~(bits);
}

static inline void toggle_bit_ptr(volatile uint8_t *reg, uint8_t bit)
{
    *reg ^= (1 << bit);
}

#endif
