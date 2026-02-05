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
	initSPI(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE); // Se inicia la comunicación SPI en el slave
	setup(); // Se llama el setup
	initADC(); // Se inicia el ADC
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

void refreshPORT(uint8_t valor) // Función para escribir a los leds en PORTB y PORTD
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
	adc_read = ADCH; // Se guarda el valor del ADC HIGH y enciende la bandera de adc_flag para main
	adc_flag = 1;
}


ISR(SPI_STC_vect)
{
	uint8_t rx = SPDR;

	if (rx == 'c') // Si se recibe 'c' en el SPDR se escriben los datos de los potenciómetros al SPDR
	{	
		if (spi_counter == 0) // Entra para cargar el valor del potenciómetro 1 al SPDR
		{ 
			spi_counter = 1; 
			SPDR = adc_pot1; // Carga el valor del potenciómetro 1 al SPDR
		}
		else // Entra para cargar el valor del potenciómetro 2 al SPDR
		{ 
			spi_counter = 0;
			SPDR = adc_pot2;	// Carga el valor del potenciómetro 2 al SPDR 
		}
		return;
	}

	if (expect_led == 1) 
	{ // Ingresa cuando la bandera de expect_led se enciende
		spiValor = rx;      // Se guarda el valor de SPRD en una variable
		led_flag = 1;		// Se enciende la bandera de led_flag para el main
		expect_led = 0;		// Se apaga expect_led
	}

	if (rx == 'L') // Si SPDR recibe L enciende una bandera llamada expect_led
	{        
		expect_led = 1;
	}
}