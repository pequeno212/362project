#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>
#include "DAC.c"
#include "I2C.c"

void nano_wait(int);
void internal_clock();


int main(void) {
    internal_clock();

   

    enable_ports();
    setup_dma();
    enable_dma();

    init_wavetable();
    setup_dac();

    //call init_tim6 when interrupt is enabled
    init_tim6();




    // Have fun.
    dialer();
}
