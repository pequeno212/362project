  // ------------------------------------------------------------------------------------------
  // ------------------------------------------ EVAN ------------------------------------------
  
  //============================================================================
  // TFTLCD stepup + display
  //============================================================================
#define SHELL


void nano_wait(int);
void internal_clock();

#ifdef SHELL
#include "stm32f0xx.h"
#include <stdint.h>
#include "lcd.h"
#include "fifo.h"
#include "tty.h"
#include <stdio.h>

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
void moving_rect(){
    int X_MAX = 240; //the lcd is 240 pixels wide 320 pixels long
    int Y_MAX = 320;
    int x = 0;
    int y = Y_MAX-20;
    int x_len = 100;
    int y_len = 20;
    int moving_left = 0;
    int moving_right = 1;
    while(x_len > 0){
        // nano_wait(20000000);
        if(moving_right){
            LCD_DrawFillRectangle(x, y, x+x_len, y+y_len, 0x0f0f);
            LCD_DrawFillRectangle(0, y, x, y+y_len, 0xFFFF);
            x++;
            if(x+x_len >= X_MAX-2){
                moving_left = 1;
                moving_right = 0;
            }            
        }
        else if(moving_left){
            LCD_DrawFillRectangle(x, y, x+x_len, y+y_len, 0x0f0f);
            LCD_DrawFillRectangle(x+x_len, y, X_MAX, y+y_len, 0xFFFF);
            x--;
            if(x <= 2){
                moving_left = 0;
                moving_right = 1;
            }            
        }
    }
}
//============================================================================
// END OF LCD DISPLAY
//============================================================================


// int main() {
//     internal_clock();
//     setbuf(stdin,0);
//     setbuf(stdout,0);
//     setbuf(stderr,0);
//     LCD_Setup();
//     LCD_Clear(0xFFFF);
//     moving_rect();
//     // moving_rect();
//     command_shell();
// }
#endif