#include <avr/io.h>
#include "./../lib/serialpwm.h"


int main(void)
{
	serialpwm();
    byte buffer[5];
	recvpwm(buffer,5);
	
	DDRD = 0xff;
	PORTD = buffer[0];
	
    while (1) 
    {
    }
}

