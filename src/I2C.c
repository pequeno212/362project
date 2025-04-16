#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>



// ------------------------------------------------------------------------------------------
  // ------------------------------------------ BLAS ------------------------------------------
  
  //============================================================================
  // i2c setup
  //============================================================================

  // Initialize PC0 (SDA) and PC1 (SCL) for I2C

void i2c_init_gpio(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Set PC0 and PC1 as i/p
    GPIOC->MODER &= ~(0b11 << (0 * 2)); // PC0
    GPIOC->MODER &= ~(0b11 << (1 * 2)); // PC1

    // Enable pull-up resistors
    GPIOC->PUPDR &= ~(0b11 << (0 * 2));
    GPIOC->PUPDR |=  (0b01 << (0 * 2)); // pull-up for PC0
    GPIOC->PUPDR &= ~(0b11 << (1 * 2));
    GPIOC->PUPDR |=  (0b01 << (1 * 2)); // pull-up for PC1
}

// I2C START
// SDA is supposed to go low while SCL is high
void i2c_start(void) {
    GPIOC->MODER &= ~(0b11 << (0 * 2)); // SDA = i/p (high)
    GPIOC->MODER &= ~(0b11 << (1 * 2)); // SCL = i/p (high)

    nano_wait(1000);

    GPIOC->MODER &= ~(0b11 << (0 * 2)); // Clear mode
    GPIOC->MODER |=  (0b01 << (0 * 2)); // SDA = o/p
    GPIOC->ODR   &= ~(1 << 0); // SDA = 0
    nano_wait(1000);

    GPIOC->MODER &= ~(0b11 << (1 * 2)); // SCL = o/p
    GPIOC->MODER |=  (0b01 << (1 * 2));
    GPIOC->ODR   &= ~(1 << 1); // SCL = 0

    nano_wait(1000);
}
    // I2C STOP
// SDA high while SCL is high
void i2c_stop(void) {
    GPIOC->MODER &= ~(0b11 << (0 * 2)); // SDA = o/p
    GPIOC->MODER |=  (0b01 << (0 * 2));
    GPIOC->ODR   &= ~(1 << 0); // SDA = 0
    nano_wait(1000);

    GPIOC->MODER &= ~(0b11 << (1 * 2)); // SCL = i/p (high)
    nano_wait(1000);

    GPIOC->MODER &= ~(0b11 << (0 * 2)); // SDA = i/p (high)
    nano_wait(1000);

}