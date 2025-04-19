// ------------------------------------------------------------------------------------------
// ------------------------------------------ EVAN ------------------------------------------

//============================================================================
// TFTLCD stepup + display
//============================================================================
#define SHELL


void nano_wait(int);
void internal_clock();
int get_new_len(int x, int x_prev, int x_len, int y, int num_layers);

#ifdef SHELL
#include "stm32f0xx.h"
#include <stdint.h>
#include "lcd.h"
#include "fifo.h"
#include "tty.h"
#include <stdio.h>
#include <math.h>

#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

//============================================================================
// START OF LCD SETUP
//============================================================================
void init_spi1_slow(){
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER |= GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1;
    GPIOB -> AFR[0] |= ~(GPIO_AFRL_AFRL3_Msk | GPIO_AFRL_AFRL4_Msk | GPIO_AFRL_AFRL5_Msk); //AF0 is spi mode

    RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN; //enable spi
    SPI1 -> CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2; //set baud rate to be lowest
    SPI1 -> CR1 |= SPI_CR1_MSTR; //set to master config
    SPI1 -> CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2; //set to 8-bit word size
    SPI1 -> CR1 |= SPI_CR1_SSM; //software slave managment enabled
    SPI1 -> CR1 |= SPI_CR1_SSI; //internal slave select enabled
    SPI1 -> CR2 |= SPI_CR2_FRXTH; //set fifo threshold
    SPI1 -> CR1 |= SPI_CR1_SPE; //enable SPI channel
}

void enable_sdcard(){
    GPIOB -> BRR |= GPIO_BRR_BR_2;
}

void disable_sdcard(){
    GPIOB -> BSRR |= GPIO_BSRR_BS_2;
}

void init_sdcard_io(){
    init_spi1_slow();
    GPIOB -> MODER |= GPIO_MODER_MODER2_0;
    disable_sdcard();
}

void sdcard_io_high_speed(){
    SPI1 -> CR1 &= ~SPI_CR1_SPE; //disable SPI channel
    SPI1 -> CR1 &= ~SPI_CR1_BR_Msk;
    SPI1 -> CR1 |= SPI_CR1_BR_0;
    SPI1 -> CR1 |= SPI_CR1_SPE;
}

void init_lcd_spi(){
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER14_0;
    init_spi1_slow();
    sdcard_io_high_speed();
}

//============================================================================
// END OF LCD SETUP
//============================================================================




//============================================================================
// START OF LCD DISPLAY
//============================================================================
int moving_rect(int x, int x_prev, int y, int x_len, int delay, int num_layers, int game_over, int count){
    if(game_over != 0){
        return game_over; //even when hardcoded to -1, doesnt do the correct thing in main
    }
    if (count == 15){
        return game_over;
    }

    int X_MAX = 240; //the lcd is 240 pixels wide
    int moving_left = 0;
    int moving_right = 1;

    while(!(GPIOB -> ODR & (1 << 7))){ //while the button isnt pressed, the block will shift
        nano_wait(delay);
        if(moving_right){
            LCD_DrawFillRectangle(x, y, x+x_len, y+20, 0x0f0f);
            LCD_DrawFillRectangle(0, y, x, y+20, 0xFFFF);
            x++;
            if(x+x_len >= X_MAX-2){
                moving_left = 1;
                moving_right = 0;
            }            
        }
        else if(moving_left){
            LCD_DrawFillRectangle(x, y, x+x_len, y+20, 0x0f0f);
            LCD_DrawFillRectangle(x+x_len, y, X_MAX, y+20, 0xFFFF);
            x--;
            if(x <= 2){
                moving_left = 0;
                moving_right = 1;
            }            
        }
    }
    GPIOB -> BSRR = GPIO_BSRR_BR_7;
    count += 1;
    // nano_wait(100000000);
    int next_x_len = get_new_len(x, x_prev, x_len, y, num_layers);
    if(next_x_len <= 0){game_over = -1;}
    game_over = moving_rect(x, x, y-20, next_x_len, delay-1000000, num_layers+1, game_over, count);
    return game_over;
}

int get_new_len(int x, int x_prev, int x_len, int y, int num_layers){ //this function will return an int storing the horizontal length of the next stack
    if(num_layers == 1){return x_len;} //base case, if we're on the first stack, the next stack length will always equal the 1st
    if(x == x_prev){return x_len;} //next base case, when the 2 stacks perfectly align

    int offset = abs(x-x_prev); //get offset
    if(x > x_prev){ //if top stack is right of bottom stack, clear out the right part that overflows
        LCD_DrawFillRectangle(x_prev + x_len, y, x_prev+x_len+offset, y+20, 0xFFFF);
    }
    else{ //otherwise, we will clear out the left part that overflows
        LCD_DrawFillRectangle(x_prev - offset, y, x_prev, y+20, 0xFFFF);
    }
    return x_len-offset;

}
    //============================================================================
// END OF LCD DISPLAY
//============================================================================

// INTERRUPT HANDLER 



void initb (){
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN; //enable b
    
    
    GPIOB -> MODER &= ~ 0xFFFFFF; //resets 11-0 
    GPIOB -> PUPDR &= ~ 0xFFFFFF; //resets pupdr
    GPIOB -> PUPDR |= GPIO_PUPDR_PUPDR0_1; //pb0 as pulldown
    GPIOB -> MODER |= GPIO_MODER_MODER7_0; //set pb7 to ouptut
    GPIOB -> BSRR |= GPIO_BSRR_BR_7; //reset pb7 to 0
    
}

void togglexn() {
    GPIOB -> BSRR = GPIO_BSRR_BS_7; //switched toggle to just turn on, will be turned off when new level is added
}

void init_exti() {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //enable syscfg subsystem
    //RCC_APB2ENR_SYSCFGCOMPEN

    SYSCFG -> EXTICR[0] &= ~ SYSCFG_EXTICR1_EXTI0;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; //selects port b to syscfg subsystem

    EXTI -> RTSR |= EXTI_RTSR_TR0; //configures extir_rtsr register
    EXTI -> IMR |= EXTI_IMR_MR0; //configures IMR register
    
    NVIC -> ISER[0] |= (1 << EXTI0_1_IRQn); //enable interrupts

}

void EXTI0_1_IRQHandler(){

    EXTI->PR = (EXTI_PR_PR0);
    togglexn(); //pressed variable
}

#endif