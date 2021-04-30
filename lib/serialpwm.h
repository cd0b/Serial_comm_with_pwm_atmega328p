#ifndef _SERIALPWM_H
#define _SERIALPWM_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define BUFFERSIZE 64

typedef unsigned char byte;

extern volatile byte serialpwm_data_buffer[BUFFERSIZE];

void serialpwm();
int sendpwm(const void*, unsigned int);
void recvpwm(void*, unsigned int);

#endif


