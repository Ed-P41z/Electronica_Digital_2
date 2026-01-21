/*
 * Lab01.c
 *
 * Created: 15/01/2026 7:16:42 PM
 * Author: edvin
 * Description: El lab consiste en realizar una cuenta regresiva con un display
 */
//*********************************************
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Display_Lib/Disp_Lib.h"

uint8_t	counter;
uint8_t pb_flag;
uint8_t estado_actual;
uint8_t estado_anterior;
uint8_t estado_actual_B;
uint8_t estado_anterior_B;
uint8_t jugador_1;
uint8_t jugador_2;
uint8_t ganador;
uint8_t start;
#define	TCNT0_Value 134;
//*********************************************
// Function prototypes
void setup();
void initTMR0();
void juego(uint8_t *jugador);

//*********************************************
// Main Function

int main(void)
{
	setup();	// Se manda a llamar la función de Setup
	
	show_display(5);
	
	while(1)	// Entra al bucle infinito en donde se ejecuta el programa
	{
		if (pb_flag == 1)	// Si la bandera de acción de PB0 está encendida entra al if
		{
			pb_flag = 0;		// Apaga la bandera de acción de los botones
			counter = 6;
			start = 0;
			ganador = 0;
			jugador_1 = 0x00;
			jugador_2 = 0x00;
			PORTB = (PORTB & 0xF0);
			PORTC = (PORTC & ~((1<<PORTC2)|(1<<PORTC3)|(1<<PORTC4)|(1<<PORTC5))) | (1<<PORTC0) | (1<<PORTC1);
			TCNT0   = TCNT0_Value;					// Se carga el valor al TCNT0
			TIMSK0  = (1 << TOIE0);					// Se encienden las interrupciones del timer0	
		}
		else if (start == 1)
		{
			if (pb_flag == 2)
			{
				pb_flag = 0;		// Apaga la bandera de acción de los botones
				switch(ganador)
				{
					case 0:
					juego(&jugador_1);
					PORTB = (PORTB & 0xF0) | (jugador_1 & 0x0F);
					if (ganador == 1)
					{
						PORTB = 0x0F;
						show_display(1);
					}
					break;
				
					case 1:
					start = 0;
					break;
				}
			}
			else if (pb_flag == 3)
			{
				pb_flag = 0;		// Apaga la bandera de acción de los botones
				switch(ganador)
				{
					case 0:
					juego(&jugador_2);
					PORTC = ((jugador_2 & 0x0F) << 2);
					if (ganador == 1)
					{
						PORTC = 0x3C;
						show_display(2);
					}
					break;
				
					case 1:
					start = 0;
					break;
				}
			}
		}
		else   // El else no hace nada
		{
		}
	}	
}

//*********************************************
// NON-Interrupt subroutines
void setup()
{
	cli(); // Se apagan las interruciones globales
	
	// Configurar presclaer de sistema
	CLKPR	= (1 << CLKPCE);
	CLKPR	= (1 << CLKPS1) | (1 << CLKPS2); // 64 PRESCALER -> 250kHz
	
	// Configuración de Pines
	DDRD	= 0xFF;			// Se configura PORTD como salida
	PORTD	= 0x00;			// PORTD inicialmente apagado
	DDRC	= 0xFF;			// Se configura PORTC como salida
	DDRC	&= ~((1 << DDC0) | (1 << DDC1)); // Se configura PC0 y PC1 como input
	PORTC	= 0x00;			// PORTC inicialmente apagado
	PORTC	|= (1 << PORTC0) | (1 << PORTC1); // Pull-up para PC0 y PC1
	DDRB	= 0xFF;			// Se configura PORTB como salida
	DDRB	&= ~(1 << DDB5);	// PB5 se configura como input
	PORTB	= 0x00;			// PORTB inicialmente apagado
	PORTB	|= (1 << PORTB5); // Pull-up para PB5
	
	// Inicio de Timers
	initTMR0();	

	// Configuración de Interrupciones
	PCICR  |= (1 << PCIE1) | (1 << PCIE0);		// Habilita las interrupciones pin-change para PORTB y PORTC
	PCMSK0 |= (1 << PCINT5);	// Habilita PCINT0 y PCINT1
	PCMSK1 |= (1 << PCINT0) | (1 << PCINT1);	// Habilita PCINT0 y PCINT1
	
	// Inicialización de Variables
	counter	= 6;
	pb_flag = 0;
	estado_actual	= 0xFF;
	estado_anterior	= 0xFF;
	jugador_1 = 0;
	jugador_2 = 0;
	start = 0;
	
	sei(); // Se encienden las interrupciones globales
}

void initTMR0()
{
	TCCR0A  = 0;							// Se usa el modo normal
	TCCR0B  |= (1 << CS02) | (1 << CS00);	// Se configura el prescaler
	TCNT0   = TCNT0_Value;					// Se carga el valor al TCNT0
}

void juego(uint8_t *jugador)
{
	if (*jugador == 0) 
	{
		*jugador = 1;
	} 
	else if (*jugador < 0x04) 
	{
		*jugador <<= 1;
	} 
	else
	{ 
		ganador = 1;
	}
}

//*********************************************
// Interrupt routines

ISR(PCINT0_vect)
{
	estado_actual_B = PINB;  // Leer el estado actual de los botones
	
	if (((estado_anterior_B & (1 << PORTB5)) != 0) && ((estado_actual_B & (1 << PORTB5)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PB0 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
	}
	estado_anterior_B = estado_actual_B;  // Guardar el estado actual a estado anterior
}

ISR(PCINT1_vect)
{
	estado_actual = PINC;  // Leer el estado actual de los botones
	
	if (((estado_anterior & (1 << PORTC0)) != 0) && ((estado_actual & (1 << PORTC0)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 2;																	// Si PB0 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
	}
	
	else if (((estado_anterior & (1 << PORTC1)) != 0) && ((estado_actual & (1 << PORTC1)) == 0)) // Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 3;																	// Si PB1 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
	}

	estado_anterior = estado_actual;  // Guardar el estado actual a estado anterior
}
	
ISR(TIMER0_OVF_vect)
{
	TCNT0 = TCNT0_Value;	// Se carga el valor a TCNT0
	counter--;
	
	if (counter == 0)
	{
		start = 1;
	}
	
	switch(counter)		// Se hace un switch dependiendo del valor de la bandera de los displays
	{
		case 5:				// Caso bandera = 0
		show_display(counter);	// Se muestra el valor de la lista en el display
		break;
		
		case 4:				// Caso bandera = 1
		show_display(counter);	// Se muestra el valor de la lista en el display
		break;
		
		case 3:				// Caso bandera = 2
		show_display(counter);	// Se muestra el valor de la lista en el display
		break;
		
		case 2:
		show_display(counter);	// Se muestra el valor de la lista en el display
		break;
		
		case 1:
		show_display(counter);	// Se muestra el valor de la lista en el display
		break;
		
		case 0:
		show_display(counter);	// Se muestra el valor de la lista en el display
		counter = 6;
		TIMSK0  = (0 << TOIE0);					// Se apagan las interrupciones del timer0
		break;
	}
}
