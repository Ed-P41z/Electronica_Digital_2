/*
 * ADC.c
 *
 * Created: 1/22/2026 7:11:03 PM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>


void initADC()
{
	ADMUX	= 0;
	ADMUX	|= (1 << REFS0);
	ADMUX	|= (1 << ADLAR);
	ADMUX	&= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));	// Se configura el PC0 y la justificación
	
	ADCSRA	= 0;
	ADCSRA	|= (1 << ADPS2) | (1 << ADPS1);
	ADCSRA	|= (1 << ADIE);
	ADCSRA	|= (1 << ADEN);					// Se configura la interrupción y el prescaler
}

void ADC_to_Voltage(uint8_t ADC_Value, uint16_t *Voltage)
{
	*Voltage = ((uint32_t)ADC_Value * 500) / 255;
}

