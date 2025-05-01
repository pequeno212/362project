#ifndef TFTLCD_H
#define TFTLCD_H

// void init_spi1_slow();
// void enable_sdcard();
// void disable_sdcard();
// void init_sdcard_io();
// void sdcard_io_high_speed();
// void init_lcd_spi();
void initb();
void init_exti();
int moving_rect(int x, int x_prev, int y, int x_len, int delay, int num_layers, int game_ove, int *count, int colorBlock, int colorBG);
void your_score(score);
void set7();
void you_lose();
void you_win();
#endif /* TFTLCD_H */