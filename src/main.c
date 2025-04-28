#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>
#include "lcd.h"
#include "TFTLCD.h"
#include "dac.h"


void nano_wait(int);
void internal_clock();


// themes struct
typedef struct {
    uint16_t background;
    uint16_t block;
  } Theme;
  

// theme select menu
static const Theme themes[] = {
    {.background = WHITE, .block = BLACK} // B&W
    {.background = BLUE, .block = RED} // Blue and Red
    {.background = MAGENTA, .block = WHITE}
  }

int COLOR_INDEX = 0; //global variable



int main(void) {
    internal_clock();

    //TFT LDC instantiation
    initb();
    togglexn();
    init_exti();
    

    //DAC installation
    setup_dac();
    internal_clock();
    init_wavetable();
    set_freq(0, 440.0);

    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    LCD_Setup();
    LCD_Clear(themes.bg[COLOR_INDEX]);


    int game = moving_rect(0, 0, 320, 100, 10000000, 0, 0, 0, themes.block[COLOR_INDEX]]);
    if(game == -1){ //game is lost
        you_lose();
        
        
    }
    if(game == 0){ //game is won
        init_tim6(); //sound when game is won, can be changed with set_freq
        you_win();
        // nano_wait(3000000); //does not wait for 3 seconds
        // stop_tim6();
       
    }
    //LCD_Clear(0xF000);
}