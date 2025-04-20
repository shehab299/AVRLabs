#ifndef BIT_UTILS_H
#define BIT_UTILS_H

#include <avr/io.h>

// Set a bit
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))

// Clear a bit
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))

// Toggle a bit
#define TOGGLE_BIT(REG, BIT) ((REG) ^= (1 << (BIT)))

// Read a bit (returns 0 or non-zero)
#define READ_BIT(REG, BIT) ((REG) & (1 << (BIT)))

// Set bit using pointer to volatile register
static inline void set_bit_ptr(volatile uint8_t* reg, uint8_t bit) {
    *reg |= (1 << bit);
}

static inline void clear_bit_ptr(volatile uint8_t* reg, uint8_t bit) {
    *reg &= ~(1 << bit);
}

static inline void toggle_bit_ptr(volatile uint8_t* reg, uint8_t bit) {
    *reg ^= (1 << bit);
}

#endif 
