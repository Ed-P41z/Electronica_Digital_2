/*
 * Lab06.c
 *
 ;
 ; Universidad del Valle de Guatemala
 ; Departamento de Ingenieria Mecatronica, Electronica y Biomedica
 ; Semestre 1 2026
 ;
 ; Created: 12/03/2026 10:21:22
 ; Author : Edvin Paiz
 ; Carnet : 23072
 ; Description : UART para control de videojuegos
 */ 

//***************************************************//
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "UART/UART.h"

volatile uint8_t estado_actual;
volatile uint8_t estado_anterior;
volatile uint8_t pb_flag;
volatile char accion;

//***************************************************//
// Function prototypes
void setup();

//***************************************************//
// Main function
int main(void)
{
	setup(); // Se llama el setup
	sei();
	while(1)
	{
		if (pb_flag == 1)
		{
			pb_flag = 0;
			switch(accion)
			{
				case '1':
				writeChar('1');
				break;
				
				case '2':
				writeChar('2');
				break;
				
				case '3':
				writeChar('3');
				break;
				
				case '4':
				writeChar('4');
				break;
				
				case 'a':
				writeChar('A');
				break;
				
				case 'b':
				writeChar('B');
				break;
			}
		}
	}
}


//***************************************************//
// NON-Interrupt subroutines
void setup()
{
	cli(); // Se apagan las interruciones globales
	
	// Configurar presclaer de sistema
	//CLKPR	= (1 << CLKPCE);
	//CLKPR	= (1 << CLKPS2); // 16 PRESCALER -> 1MHz
	
	// Configuración de Pines
	DDRD	&= ~((1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7));			// Se configura PORTD como entrada
	PORTD	|= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);			// PORTD con Pull-up
	
	// Inicialización de Variables
	pb_flag = 0;
	estado_actual = 0;
	estado_anterior = 0;
	accion = '0';
	
	// Inicio de UART
	initUART();				// Se llama la función de inicio del UART compatible para la STM32 
	
	// Inicio de PWM
	
	// Inicio de ADC
	
	// Configuración de Interrupciones
	PCICR |= (1 << PCIE2);   // Habilita pin change interrupts para PORTD
	PCMSK2 |= (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22) | (1 << PCINT23); // PORTD2 al PORTD7
	
	sei(); // Se encienden las interrupciones globales
}

//***************************************************//
// Interrupt routines

//-----------------Interrupción de Pin-Change-----------------
ISR(PCINT2_vect)
{
	estado_actual = PIND & ((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));  // Leer el estado actual de los botones
	
	if (((estado_anterior & (1 << PORTD2)) != 0) && ((estado_actual & (1 << PORTD2)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC4 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
		accion = 'b';
	}
	else if (((estado_anterior & (1 << PORTD3)) != 0) && ((estado_actual & (1 << PORTD3)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC5 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
		accion = 'a';
	}
	else if (((estado_anterior & (1 << PORTD4)) != 0) && ((estado_actual & (1 << PORTD4)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC5 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
		accion = '4';
	}
	else if (((estado_anterior & (1 << PORTD5)) != 0) && ((estado_actual & (1 << PORTD5)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC5 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
		accion = '3';
	}
	else if (((estado_anterior & (1 << PORTD6)) != 0) && ((estado_actual & (1 << PORTD6)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC5 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
		accion = '2';
	}
	else if (((estado_anterior & (1 << PORTD7)) != 0) && ((estado_actual & (1 << PORTD7)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PC5 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
		accion = '1';
	}
	estado_anterior = estado_actual;  // Guardar el estado actual a estado anterior
}
