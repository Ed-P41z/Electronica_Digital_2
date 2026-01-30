/*
 * Lab03.c
 *
 ;
 ; Universidad del Valle de Guatemala
 ; Departamento de Ingenieria Mecatronica, Electronica y Biomedica
 ; IE2023 Semestre 1 2025
 ;
 ; Created: 29/01/2026 10:21:22
 ; Author : Edvin Paiz
 ; Carnet : 23072
 ; Description : SPI MASTER
 */ 

//***************************************************//
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "SPI/SPI.h"
#include "ADC/ADC.h"
#include "UART/UART.h"

uint8_t valorSPI = 0;
volatile uint8_t adc_read = 0;
volatile uint8_t adc_flag = 0;
volatile uint8_t counter_ADC = 0;
volatile uint8_t adc_pot1 = 0;
volatile uint8_t adc_pot2 = 0; 
volatile uint8_t spi_counter = 0;
volatile uint8_t lec_spi = 0;
volatile uint8_t led_flag = 0;
volatile uint8_t spiValor;
volatile uint8_t expect_led = 0;

//***************************************************//
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);

//***************************************************//
// Main function
int main(void)
{
	initSPI(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
	setup();
	initADC();
	initUART();
	SPCR |= (1 << SPIE);
	sei();
	ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
	while(1)
	{
		if (led_flag == 1)
		{
			refreshPORT(spiValor);
			led_flag = 0;
		}
		if (adc_flag == 1) // Ingresa cada vez que se actualiza
		{
			adc_flag = 0; // Se apaga la bandera de acción de ADC
			switch(counter_ADC) // Cambia de servo dependiendo de counter_ADC
			{
				case 0:
				counter_ADC++;		// Se suma el contador que sirve para multiplexar
				adc_pot1 = adc_read;
				ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0));
				ADMUX |= (1<<MUX0); // Cambia para leer ADC de PC1
				ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
				break;
				
				case 1:
				counter_ADC = 0;					// Regresa al caso 0 de la multiplexación
				adc_pot2 = adc_read;
				ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)); // Cambia para leer ADC de PC0
				ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
				break;
			}
		}
		else
		{
		}
	}
}


//***************************************************//
// NON-Interrupt subroutines
void setup()
{
	// Puerto de salida para leds (8 bits)
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	DDRB |= (1 << DDB0)	| (1 << DDB1);
	
	// Puero de salida para leds
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1));
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	
	
}

void refreshPORT(uint8_t valor)
{
	if (valor & 0b10000000)
	{
		PORTB |= (1 << PORTB1);
		}else{
		PORTB &= ~(1 << PORTB1);
	}
	if (valor &	0b01000000){
		PORTB |= (1 << PORTB0);
	}
	else{
		PORTB &= ~(1 << PORTB0);
	}
	if (valor &	0b00100000){
		PORTD |= (1 << PORTD7);
	}
	else{
		PORTD &= ~(1 << PORTD7);
	}
	if (valor &	0b00010000){
		PORTD |= (1 << PORTD6);
	}
	else{
		PORTD &= ~(1 << PORTD6);
	}
	if (valor &	0b00001000){
		PORTD |= (1 << PORTD5);
	}
	else{
		PORTD &= ~(1 << PORTD5);
	}
	if (valor &	0b00000100){
		PORTD |= (1 << PORTD4);
	}
	else{
		PORTD &= ~(1 << PORTD4);
	}
	if (valor &	0b00000010){
		PORTD |= (1 << PORTD3);
	}
	else{
		PORTD &= ~(1 << PORTD3);
	}
	if (valor &	0b00000001){
		PORTD |= (1 << PORTD2);
	}
	else{
		PORTD &= ~(1 << PORTD2);
	}
}

//***************************************************//
// Interrupt routines

ISR(ADC_vect){
	adc_read = ADCH;
	adc_flag = 1;
}


ISR(SPI_STC_vect)
{
	uint8_t rx = SPDR;

	if (rx == 'c') {
		if (spi_counter == 0) 
		{ 
			spi_counter = 1; 
			SPDR = adc_pot1; 
		}
		else 
		{ 
			spi_counter = 0; 
			SPDR = adc_pot2; 
		}
		return;
	}

	if (expect_led == 1) {
		spiValor = rx;      // aquí aceptas 0x00 también
		led_flag = 1;
		expect_led = 0;
	}

	if (rx == 'L') {        // header para LEDs
		expect_led = 1;
	}
}