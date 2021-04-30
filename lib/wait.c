#include <avr/interrupt.h>

typedef unsigned char byte;

volatile byte rtime = 0;

static void start_compare_timer() {
	TCNT0 = 0x00;
	OCR0A = 0xff;
	TCCR0A = 0x02;
	TCCR0B = 0x02;
	TIMSK0 = 0x02;
	TIFR0 = 0x07;
}
static void stop_compare_timer() {
	TCCR0B = 0x00;
	TCCR0A = 0x00;
	TIMSK0 = 0x00;
	TIFR0 = 0x07;
}

void wait(unsigned int time) {
	start_compare_timer();
	asm("sei");
	while(time > 0) {
		rtime = 0;
		byte stime;
		if(time >= 0xff) {
			stime = 0xff;
			}else {
			stime = time;
		}
		OCR0A = stime;
		time -= stime;
		while(rtime == 0);
	}
	stop_compare_timer();
}

ISR(TIMER0_COMPA_vect) {
	rtime = 1;
}