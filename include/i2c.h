#ifndef __i2c_H__
#define __i2c_H__

void enable_ports_i2c(void) ;
void init_i2c(void) ;
void init_usart5(void);
void enable_tty_interrupt(void);
void read(int argc, char* argv[]);
void write(int argc, char* argv[]);
void print_high_score(int score, int order);

#endif /* __i2c_H__ */
