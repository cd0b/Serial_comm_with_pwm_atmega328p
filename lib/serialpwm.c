#include "./serialpwm.h"
#include <string.h>

// IO Registers.
#define OUTPORT		PORTB
#define OUTDDR		DDRB
#define OUTPIN		1
#define INPORT		PORTB
#define INDDR		DDRB
#define INPIN		0

#define SETPIN(PORT,PIN)		((PORT)|=(1<<(PIN)))
#define CLRPIN(PORT,PIN)		((PORT)&=~(1<<(PIN)))
#define TGLPIN(PORT,PIN)		((PORT)^=(1<<(PIN)))

// Timer Registers.
#define TIMERCTLREGA		TCCR1A
#define TIMERCTLREGB		TCCR1B
#define TIMERCTLREGC		TCCR1C
#define TIMERCTRREG			TCNT1
#define TIMERCOMPREG		OCR1A
#define TIMERCAPTREG		ICR1
#define TIMERMASKREG		TIMSK1
#define TIMERFLAGREG		TIFR1
#define Clear_timer_flags() { TIMERFLAGREG = 0x27; }

// Timer Interrupt  Vectors.
#define TIMER_OVERFLOW TIMER1_OVF_vect
#define TIMER_CAPTURE TIMER1_CAPT_vect

void set_port();
void set_timer_send();
void set_timer_recv();
void init_timer();

struct CTL {
	volatile byte direction: 1;
	volatile byte edge: 1;
	volatile byte status: 1;
	volatile byte bus: 1;
	volatile byte bufend: 1;
	volatile byte rtime: 1;
};

struct CTL control;
volatile byte serialpwm_data_buffer[BUFFERSIZE] = {0};
volatile byte* buffer_ptr;
#define END_OF_BUFFER (&serialpwm_data_buffer[BUFFERSIZE])
#define START_OF_BUFFER (&serialpwm_data_buffer[0])

// Direction.
#define RECV		0
#define SEND		1

// Edge.
#define FALLING		0
#define RISING		1

// Status.
#define WAITING 0
#define RECEIVED 1

// Bufend.
#define END 1
#define NOT_END 0

// Bus.
#define AVAILABLE 1
#define BUSY 0


extern void wait(unsigned int);




void serialpwm() {
	
	control.direction = RECV;
	control.edge = RISING;
	control.status = WAITING;
	control.bus = AVAILABLE;
	control.bufend = NOT_END;
	
	set_port();
	init_timer();
	
}



void set_port() {
	SETPIN(OUTDDR, OUTPIN);
	CLRPIN(INDDR, INPIN);
}


void init_timer() {
	if(control.status != WAITING) { return; }
	if(control.direction == RECV) {
		asm("cli");
		set_timer_recv();
		asm("sei");
	} else {
		asm("cli");
		set_timer_send();
		asm("sei");
	}
}


void set_timer_recv() {
	buffer_ptr = START_OF_BUFFER;
	control.edge = RISING;
	TIMERCTRREG = 0x0000;
	TIMERCOMPREG = 0x0000;
	TIMERCTLREGA = 0x00;
	TIMERCTLREGB = 0x42;
	TIMERCTLREGC = 0x00;
	TIMERMASKREG = 0x20;
	Clear_timer_flags();
}

void set_timer_send() {
	buffer_ptr = START_OF_BUFFER;
	control.bufend = NOT_END;
	TIMERCTRREG = 0x0000;
	TIMERCOMPREG = 0x01ff;
	TIMERCTLREGA = 0xc2;
	TIMERCTLREGB = 0x0a;
	TIMERCTLREGC = 0x00;
	TIMERMASKREG = 0x01;
	Clear_timer_flags();
}



int sendpwm(const void* buf, unsigned int length) {
	control.bus = AVAILABLE;
	wait(0xffff);
	if(control.bus != AVAILABLE) { return -1};
	memcpy((void*)serialpwm_data_buffer, buf, length);
	control.direction = SEND;
	init_timer();
	while(control.direction == SEND);
	init_timer();
	return 0;
}

void recvpwm(void* buf, unsigned int length) {
	while(control.status == WAITING);
	memcpy(buf, (const void*)serialpwm_data_buffer, length);
	control.status = WAITING;
	init_timer();
}




ISR(TIMER_OVERFLOW) {
	if(buffer_ptr == END_OF_BUFFER) {
		TIMERCOMPREG = 0x01ff;
		if(control.bufend == END) {
			control.direction = RECV;
			return;
		}
		control.bufend = END;
		return;
	}
	TIMERCOMPREG = (0x01ff - (0x80 + *buffer_ptr));
	++buffer_ptr;
}


int start_time = 0;
ISR(TIMER_CAPTURE) {
	control.bus = BUSY;
	int capt = TIMERCAPTREG;
	TIMERCTLREGB ^= 0x40;
	Clear_timer_flags();

	if(control.edge == RISING) {
		start_time = capt;
		control.edge = FALLING;
	}else {
		int data = ((capt-start_time)%0x01ff)-0x80;
		*buffer_ptr = data;
		++buffer_ptr;
		if(buffer_ptr == END_OF_BUFFER) { control.status = RECEIVED; }
		control.edge = RISING;
		TIMERCTRREG = 0;
	}
}