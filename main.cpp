#include "global.h"
#include "avr/io.h"
#include "avr/interrupt.h"

#define CHANNEL_PORT PORTB
#define CHANNEL_F_PIN_N 3
#define CHANNEL_N_PIN_N 4

#define ACTIVATE_FUEL sbi(CHANNEL_PORT, CHANNEL_F_PIN_N)
#define DEACTIVATE_FUEL cbi(CHANNEL_PORT, CHANNEL_F_PIN_N)
#define ACTIVATE_NITROUS sbi(CHANNEL_PORT, CHANNEL_N_PIN_N)
#define DEACTIVATE_NITROUS cbi(CHANNEL_PORT, CHANNEL_N_PIN_N)
#define ACTIVATE_WITH_NOISE noiseTimer = 0
#define ACTIVATE_NITROUS_WITH_DELAY nitrousTimer = 0
#define ACTIVATE ACTIVATE_FUEL; ACTIVATE_NITROUS_WITH_DELAY

#define CLEAR_NITROUS_TIMER nitrousTimer = -1
#define CLEAR_NOISE_TIMER noiseTimer = -1

#define BUTTON_PORT PORTB
#define BUTTON_PIN PINB
#define BUTTON_PIN_N 1
#define BUTTON_MASK (1 << BUTTON_PIN_N)
#define BUTTON_PRESSED !(BUTTON_PIN & BUTTON_MASK)

#define DELAY_MULTIPLY (PINB & (1 << 0))

#define TIMER_COUNTER_INIT 106; // 1000 Hz on 9.6 MHz / 64

int nitrousTimer = -1;
int noiseTimer = -1;

int nitrousTrigger = 20;
int noiseTrigger = 20;

void activate(void) {
	ACTIVATE_FUEL;
	ACTIVATE_NITROUS_WITH_DELAY;
}

ISR(INT0_vect) {
	if (BUTTON_PRESSED) {
		// Button pressed
		ACTIVATE_WITH_NOISE;
	} else {
		// Button released
		DEACTIVATE_FUEL;
		DEACTIVATE_NITROUS;

		CLEAR_NITROUS_TIMER;
		CLEAR_NOISE_TIMER;
	}
}

ISR(TIM0_OVF_vect) {
	TCNT0 = TIMER_COUNTER_INIT;
	if (noiseTimer != -1 && ++noiseTimer >= noiseTrigger) {
		if (BUTTON_PRESSED) {
			ACTIVATE;
		}
		CLEAR_NOISE_TIMER;
	}

	if (nitrousTimer != -1 && ++nitrousTimer >= nitrousTrigger) {
		ACTIVATE_NITROUS;
		CLEAR_NITROUS_TIMER;
	}
}

ISR (ADC_vect)
{
	nitrousTrigger = ADCH + (DELAY_MULTIPLY ? ADCH : 0);

	ADCSRA |= (1 << ADSC);
}

void adcInit (void) {
    ADMUX |= (1 << MUX0) | (1 << ADLAR);
    ADCSRA |=  (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void portInit(void) {
	PORTB = 0;
	DDRB |= (1 << CHANNEL_F_PIN_N) | (1 << CHANNEL_N_PIN_N);
}

void timer0Init(void) {
	TCCR0B = (1 << CS01) | (1 << CS00);
	TCNT0 = TIMER_COUNTER_INIT;
	sbi(TIMSK0, TOIE0);
}

void intInit(void) {
	MCUCR |= (1 << ISC00);
	GIMSK |= (1 << INT0);
}

void init() {
	portInit();
	timer0Init();
	intInit();
	adcInit();
	sei();
}

int main(void) {
	init();

	while(1) {}

	return 0;
}

