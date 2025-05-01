#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>
#include "lcd.h"
#include "TFTLCD.h"
#include "dac.h"
#include "COLOR_INDEX.h"


void nano_wait(int);
void internal_clock();


int main(void) {
    //COLOR_INDEX = 0;
    int background[] = {WHITE, BLUE, RED};
    int block[] = {BLACK, RED, WHITE};
    internal_clock();

    //TFT LDC instantiation
    initb();
    //set7();
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
    LCD_Clear(background[COLOR_INDEX]);
    

    while(!(GPIOB -> ODR & (1 << 7))){
        nano_wait(1000000);
        LCD_Clear(background[COLOR_INDEX]);
        

    }; //wait until first stack is pressed

    int count = 0;
    int level = 1;
    int game = moving_rect(0, 0, 320, 100, 10000000, 0, 0, &count, block[COLOR_INDEX], background[COLOR_INDEX], &level);
    if(level > 1){
         you_win();
         nano_wait(1000000000);
         your_score(count, level);
        happy_music();

     }
        
    else{
        you_lose();

        sad_music();

    }

   
}       
    /*
    if(game == 0){ //game is won
        init_tim6(); //sound when game is won, can be changed with set_freq
        you_win();
        // nano_wait(3000000); //does not wait for 3 seconds
        // stop_tim6();
       
    }
        */
    //LCD_Clear(0xF000);
