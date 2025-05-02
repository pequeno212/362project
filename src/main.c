#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>
#include "lcd.h"
#include "TFTLCD.h"
#include "dac.h"
#include "i2c.h"
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

    enable_ports_i2c();
    init_i2c();
    int valread[8];
   
    

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


    for (int i = 0; i < 3; i++) {
        char read_bytes[4];
        eeprom_read(i * 4, read_bytes, 4);
        valread[i] = (read_bytes[0] << 24) | (read_bytes[1] << 16) | (read_bytes[2] << 8) | read_bytes[3];

    }

    for (int i = 0; i < 3; i++) {
        if (count > valread[i]) {
            for (int j = 2; j > i; j--) {
                valread[j] = valread[j - 1];
            }
            valread[i] = count;
            break;  
        }
    }


    for (int i = 0; i < 3; i++) {
        char score_bytes[4];
        score_bytes[0] = (valread[i] >> 24) & 0xFF;
        score_bytes[1] = (valread[i] >> 16) & 0xFF;
        score_bytes[2] = (valread[i] >> 8) & 0xFF;
        score_bytes[3] = valread[i] & 0xFF;

        eeprom_write(i * 4, score_bytes, 4);
        
    }

    
    for (int i = 0; i < 3; i++) {
        print_high_score(valread[i], i+1);
    }


}