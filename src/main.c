#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>
#include "lcd.h"
#include "TFTLCD.h"

void nano_wait(int);
void internal_clock();

int main(void) {
    internal_clock();

    //TFT LDC instantiation
    initb();
    togglexn();
    init_exti();

    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    LCD_Setup();
    LCD_Clear(0xFFFF);

    int game = moving_rect(0, 0, 320, 100, 10000000, 0, 0, 0);
    if(game == -1){ //game is lost
        LCD_Clear(0xF000);
    }
    if(game == 0){ //game is won
        LCD_Clear(0x0000);
    }
    //LCD_Clear(0xF000);
}