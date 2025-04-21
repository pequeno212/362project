#ifndef __DAC_H__
#define __DAC_H__

void init_wavetable(void);
void set_freq(int chan, float f) ;
void setup_dac(void);
void TIM6_DAC_IRQHandler();
void init_tim6(void);


#endif /* __DAC_H__ */