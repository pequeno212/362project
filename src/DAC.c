#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <stdint.h>
#include <stdio.h>

void nano_wait(int);


//=============================================================================
// Part 1: 7-segment display update with DMA
//=============================================================================

// 16-bits per digit.
// The most significant 8 bits are the digit number.
// The least significant 8 bits are the segments to illuminate.
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
extern const char font[];
// Print an 8-character string on the 8 digits
void print(const char str[]);
// Print a floating-point value.
void printfloat(float f);
M_PI = 3.14159156;



//============================================================================
// enable_ports()
//============================================================================
void enable_ports(void) {

    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB -> MODER &= ~ 0x6FFFFF; // resets pb 0-10
    GPIOB -> MODER |= 0x155555; // sets pb 0-10 as output

    GPIOC -> MODER &= ~ 0xFFFF; // resets pc 0-7
    GPIOC -> MODER |= 0x5500; //sets 4-7 as output
    GPIOC -> OTYPER |= 0xF0; //sets pc4-7 to open drain
    GPIOC -> PUPDR |= 0x55; //sets 0-3 as pulled down
}

//============================================================================
// setup_dma() + enable_dma()
//============================================================================
void setup_dma(void) {
    //channel 5

    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5 -> CCR &= ~ DMA_CCR_EN;
    DMA1_Channel5 -> CMAR = (uint32_t)&msg;
    DMA1_Channel5 -> CPAR = (uint32_t) &GPIOB->ODR;
    DMA1_Channel5 -> CNDTR = 8;
    DMA1_Channel5 -> CCR |= DMA_CCR_DIR;
    //DMA1_Channel5 -> CCR |= DMA_CCR_TCIE; //interrup when done (not sure if this is right)
    //DMA1_Channel5 -> CCR &= ~ DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1;
    DMA1_Channel5 -> CCR |= DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_CIRC;

}

void enable_dma(void) {
    DMA1_Channel5 -> CCR |= DMA_CCR_EN;
}


//=============================================================================
// Part 3: Analog-to-digital conversion for a volume level.
//=============================================================================
uint32_t volume = 2048;


//============================================================================
// Varables for boxcar averaging.
//============================================================================
#define BCSIZE 32
int bcsum = 0;
int boxcar[BCSIZE];
int bcn = 0;

//===========================================================================
// Part 4: Create an analog sine wave of a specified frequency
//===========================================================================
void dialer(void);

// Parameters for the wavetable size and expected synthesis rate.
#define N 1000
#define RATE 20000
short int wavetable[N];
int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;

//===========================================================================
// init_wavetable()
// Write the pattern for a complete cycle of a sine wave into the
// wavetable[] array.
//===========================================================================
void init_wavetable(void) {
    for(int i = 0; i < N; i++)
    wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

//============================================================================
// set_freq()
//============================================================================
void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else
            step1 = (f * N / RATE) * (1<<16);
    }
}

//============================================================================
// setup_dac()
//============================================================================
void setup_dac(void) {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    //pin for dac out is pinA4
    
    GPIOA -> MODER |= 0x300;
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;
    DAC1 -> CR &= ~ 0xEC0;
    DAC1 -> CR |= DAC_CR_TEN1;
    DAC1 -> CR |= DAC_CR_EN1;

}

//============================================================================
// Timer 6 ISR
//============================================================================
// Write the Timer 6 ISR here.  Be sure to give it the right name.
void TIM6_DAC_IRQHandler() {
    /* increment offset0 by step0
    increment offset1 by step1
    if offset0 is >= (N << 16)
    decrement offset0 by (N << 16)
    if offset1 is >= (N << 16)
    decrement offset1 by (N << 16)

    int samp = sum of wavetable[offset0>>16] and wavetable[offset1>>16]
    multiply samp by volume
    shift samp right by 17 bits to ensure it's in the right format for `DAC_DHR12R1` 
    increment samp by 2048
    copy samp to DAC->DHR12R1       
    */

    TIM6 -> SR &= ~ TIM_SR_UIF;
    
    offset0 = offset0 + step0;
    offset1 = offset1 + step1;
    if (offset0 >= (N << 16)){
        offset0 = offset0 - (N << 16);
    }
    if (offset1 >= (N << 16)){
        offset1 = offset1 - (N << 16);
    }

    int samp = wavetable[offset0>>16] +  wavetable[offset1>>16];
    samp = samp * volume;
    samp = samp >> 17;
    samp = samp + 2048;
    DAC->DHR12R1 = samp;

}

//============================================================================
// init_tim6()
//============================================================================
void init_tim6(void) {

    //instead of invoking DAC at freq, only do if interrupt is aknolwedged
    //or call init_tim6 only when interrupt is enabled

    int div = 48000000/ RATE;
    int pscalar = div / 100;
    int arrscalar = 100;

    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6 -> PSC = pscalar - 1;
    TIM6 -> ARR = arrscalar - 1;
    TIM6 -> DIER |= TIM_DIER_UIE;

    NVIC -> ISER[0] = (1 << TIM6_DAC_IRQn);
    TIM6 -> CR2 &= ~ TIM_CR2_MMS;
    TIM6 -> CR2 |= TIM_CR2_MMS_1;


    TIM6 -> CR1 |= TIM_CR1_CEN;
}

/** 
 * pseudocode: exti, have interrupts unmasekd when win == true, right now its set for B0, so ideally the rest of the code would set b0 to 1 to trigger audio
 */
void init_exti() {
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN; //enable syscfg subsystem

  SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; //selects port b0 to syscfg subsystem
  EXTI -> RTSR |= EXTI_RTSR_TR0; //configures extir_rtsr register
  EXTI -> IMR |= EXTI_IMR_MR0; //configures IMR register
 
  
  NVIC -> ISER[0] = (1 << EXTI0_1_IRQn); //MIGHT NEED TO SWITCH |= FOR =

}

void EXTI0_1_IRQHandler(){
  
  EXTI->PR = (EXTI_PR_PR0);
  //EXTI->PR |= EXTI_PR_PR1;
  init_tim6();

}

