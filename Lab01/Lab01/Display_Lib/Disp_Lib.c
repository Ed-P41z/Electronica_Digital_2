/*
 * Disp_Lib.c
 *
 * Created: 1/20/2026 11:36:36 PM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void show_display(uint8_t counter)
{
	uint8_t disp_value[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
	uint8_t value = disp_value[counter];
	PORTD = value;
	
}
