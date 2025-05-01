// ------------------------------------------------------------------------------------------
// ------------------------------------------ EVAN ------------------------------------------

//============================================================================
// TFTLCD stepup + display
//============================================================================
#define SHELL


void nano_wait(int);
void internal_clock();
int get_new_len(int x, int x_prev, int x_len, int y, int num_layers, int colorBG);

#ifdef SHELL
#include "stm32f0xx.h"
#include <stdint.h>
#include "lcd.h"
#include "fifo.h"
#include "tty.h"
#include <stdio.h>
#include <math.h>
#include "COLOR_INDEX.h"

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
int moving_rect(int x, int x_prev, int y, int x_len, int delay, int num_layers, int game_over, int *count, int colorBlock, int colorBG, int *level){
    
    if(game_over != 0){ //if game over
        return game_over; //even when hardcoded to -1, doesnt do the correct thing in main
    }
    if (num_layers == 15){
        LCD_Clear(colorBG);
        LCD_DrawFillRectangle(x, 300, x+x_len, 320, colorBlock);
        num_layers = 1;
        y = 280;
        *level = *level + 1;
        //win += 1;
    }

    // if(GPIOB -> ODR & (1 << 9)){
    //      //switch colors
    //      LCD_DrawFillRectangle(0, 0, 200, 200, 0x0000);
    //      color = color + 0x5555;
    //      if (color > 0xFFFF){
    //         color = 0;
    //      }
    //      LCD_Clear(color);
    //      nano_wait(1000000000);
    //      GPIOB -> BSRR = GPIO_BSRR_BR_9;
    // }
    
    int X_MAX = 240; //the lcd is 240 pixels wide
    int moving_left = 0;
    int moving_right = 1;

    while(!(GPIOB -> ODR & (1 << 7))){ //while the button isnt pressed, the block will shift
        nano_wait(delay);
        if(moving_right){
            LCD_DrawFillRectangle(x, y, x+x_len, y+20, colorBlock);
            LCD_DrawFillRectangle(0, y, x, y+20, colorBG);
            x++;
            if(x+x_len >= X_MAX-2){
                moving_left = 1;
                moving_right = 0;
            }            
        }
        else if(moving_left){
            LCD_DrawFillRectangle(x, y, x+x_len, y+20, colorBlock);
            LCD_DrawFillRectangle(x+x_len, y, X_MAX, y+20, colorBG);
            x--;
            if(x <= 2){
                moving_left = 0;
                moving_right = 1;
            }            
        }
    }
    GPIOB -> BSRR = GPIO_BSRR_BR_7; //reset bits was bsrr before but idk why
    *count = *count + 1;
    nano_wait(300000000);
    int next_x_len = get_new_len(x, x_prev, x_len, y, num_layers, colorBG);
    if(next_x_len <= 0){game_over = -1;}
    game_over = moving_rect(x, x, y-20, next_x_len, delay-1000000, num_layers+1, game_over, count, colorBlock, colorBG, level);
    return game_over;
}

int get_new_len(int x, int x_prev, int x_len, int y, int num_layers, int colorBG){ //this function will return an int storing the horizontal length of the next stack
    if(num_layers == 1){return x_len;} //base case, if we're on the first stack, the next stack length will always equal the 1st
    if(x == x_prev){return x_len;} //next base case, when the 2 stacks perfectly align

    int offset = abs(x-x_prev); //get offset
    int new_len = x_len - offset;
    if(new_len <= 0){return 0;}

    if(x > x_prev){ //if top stack is right of bottom stack, clear out the right part that overflows
        LCD_DrawFillRectangle(x_prev + x_len, y, x+x_len, y+20, colorBG);
    }
    else{ //otherwise, we will clear out the left part that overflows
        LCD_DrawFillRectangle(x, y, x_prev, y+20, colorBG);
    }
    return new_len;

}

void you_win(){
    LCD_Clear(GREEN);
    LCD_DrawString(80, 100, BLUE, GREEN, "YOU WIN!!!! :)", 16, 0);
    return;
}
void you_lose(){
    LCD_Clear(RED);
    LCD_DrawString(80, 100, WHITE, BLACK, "you lose", 16, 0);
    return;
}
void your_score(int score, int level){
    char buffer[20];
    sprintf(buffer, "You got to level %d", level);
    LCD_Clear(YELLOW);
    LCD_DrawString(40,100, WHITE, BLACK, buffer, 16, 0);
    sprintf(buffer, "Your score is %d", score);
    LCD_DrawString(40,140, WHITE, BLACK, buffer, 16, 0);
    return;
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
    GPIOB -> PUPDR |= GPIO_PUPDR_PUPDR6_1; //pb6 as pulldown
    GPIOB -> MODER |= GPIO_MODER_MODER7_0; //set pb7 to ouptut
    GPIOB -> BSRR |= GPIO_BSRR_BR_7; //reset pb7 to 0
    GPIOB -> MODER |= GPIO_MODER_MODER9_0; //set pb9 to ouptut
    GPIOB -> BSRR |= GPIO_BSRR_BR_9; //reset pb9 to 0
    
    
}

void set7() {
    GPIOB -> BSRR = GPIO_BSRR_BS_7; //switched toggle to just turn on, will be turned off when new level is added
}


void init_exti() {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //enable syscfg subsystem
    //RCC_APB2ENR_SYSCFGCOMPEN

    SYSCFG -> EXTICR[0] &= ~ SYSCFG_EXTICR1_EXTI0;
    SYSCFG -> EXTICR[0] &= ~ SYSCFG_EXTICR2_EXTI6;

    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; //selects port b to syscfg subsystem
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PB; //selects port b6 to syscfg subsystem

    EXTI -> RTSR |= EXTI_RTSR_TR0; //configures extir_rtsr register
    EXTI -> RTSR |= EXTI_RTSR_TR6; //configures extir_rtsr register

    EXTI -> IMR |= EXTI_IMR_MR0; //configures IMR register
    EXTI -> IMR |= EXTI_IMR_MR6; //configures IMR register
    
    NVIC -> ISER[0] |= (1 << EXTI0_1_IRQn); //enable interrupts
    NVIC -> ISER[0] |= (1 << EXTI4_15_IRQn);

}

void EXTI0_1_IRQHandler(){

    EXTI->PR = (EXTI_PR_PR0);
    set7(); //pressed variable
}

void EXTI4_15_IRQHandler(){

    EXTI -> PR = (EXTI_PR_PR6);
    COLOR_INDEX += 1;
    if (COLOR_INDEX == 3) {
        COLOR_INDEX = 0;
    }

}

#endif