#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>
#include "DAC.c"
#include "I2C.c"

void nano_wait(int);
void internal_clock();


int main(void) {

    
    setup_dac();
    internal_clock();
    init_wavetable();
   
    set_freq(0, 440.0)

    //call init_tim6 when interrupt is enabled
    init_tim6();  


}
