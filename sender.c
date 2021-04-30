#include <avr/io.h>
#include "./../lib/serialpwm.h"

int main(void)
{
	byte buffer[5];
    serialpwm();
	buffer[0] = 0x51;
	buffer[1] = 0xaa;
	buffer[2] = 0xaa;
	buffer[3] = 0xaa;
	buffer[4] = 0xaa;
	
	sendpwm(buffer,5);
	
    while (1) 
    {
    }
}

