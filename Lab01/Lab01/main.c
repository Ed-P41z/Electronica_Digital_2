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

uint8_t	counter;
uint8_t pb_flag;
uint8_t estado_actual;
uint8_t estado_anterior;
uint8_t disp_1;
//uint8_t disp_value[] = {0x5F, 0x06, 0x3B, 0x2F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x59, 0x3E, 0x79, 0x71};
uint8_t disp_value[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 
0x07, 
0x7F,
0x6F, 
0x77, 
0x7C, 
0x39, 
0x5E, 
0x79, 
0x71  
};
#define	TCNT0_Value 134;
//*********************************************
// Function prototypes
void setup();
void initTMR0();

//*********************************************
// Main Function

int main(void)
{
	setup();	// Se manda a llamar la función de Setup
	
	PORTD = disp_value[5];
	
	while(1)	// Entra al bucle infinito en donde se ejecuta el programa
	{
		if (pb_flag == 1)	// Si la bandera de acción de PB0 está encendida entra al if
		{
			pb_flag = 0;		// Apaga la bandera de acción de los botones
			TIMSK0  = (1 << TOIE0);					// Se encienden las interrupciones del timer0
			TCNT0   = TCNT0_Value;					// Se carga el valor al TCNT0
			//counter++;			// Le suma al contador+
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
	DDRC	&= ~((1 << DDC1) | (1 << DDC2) | (1 << DDC3)); // Se configura PC1-PC3 como input
	PORTC	= 0x00;			// PORTC inicialmente apagado
	PORTC	|= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2);
	DDRB	= 0xFF;			// Se configura PORTB como input
	PORTD	= 0x00;			// PORTB con pull-up activado
	
	// Inicio de Timers
	initTMR0();	

	// Configuración de Interrupciones
	PCICR  |= (1 << PCIE1);						// Habilita las interrupciones pin-change para PORTB
	PCMSK1 |= (1 << PCINT0) | (1 << PCINT1);	// Habilita PCINT0 y PCINT1
	
	// Inicialización de Variables
	counter	= 6;
	pb_flag = 0;
	estado_actual	= 0xFF;
	estado_anterior	= 0xFF;
	
	sei(); // Se encienden las interrupciones globales
}

void initTMR0()
{
	TCCR0A  = 0;							// Se usa el modo normal
	TCCR0B  |= (1 << CS02) | (1 << CS00);	// Se configura el prescaler
	TCNT0   = TCNT0_Value;					// Se carga el valor al TCNT0
}


//*********************************************
// Interrupt routines

ISR(PCINT1_vect)
{
	estado_actual = PINC;  // Leer el estado actual de los botones
	
	if (((estado_anterior & (1 << PORTC0)) != 0) && ((estado_actual & (1 << PORTC0)) == 0))	// Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 1;																	// Si PB0 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
	}/*
	else if (((estado_anterior & (1 << PORTB1)) != 0) && ((estado_actual & (1 << PORTB1)) == 0)) // Se verifica si el botón está presionado y si hubo cambio de estado
	{
		pb_flag = 2;																	// Si PB1 está presionado y hubo cambio de estado, se enciende la bandera de acción de ese botón
	}*/

	estado_anterior = estado_actual;  // Guardar el estado actual a estado anterior
}
	
ISR(TIMER0_OVF_vect)
{
	TCNT0 = TCNT0_Value;	// Se carga el valor a TCNT0
	counter--;
	
	switch(counter)		// Se hace un switch dependiendo del valor de la bandera de los displays
	{
		case 5:				// Caso bandera = 0
		PORTD = disp_value[counter];	// Se muestra el valor de la lista en el display
		break;
		
		case 4:				// Caso bandera = 1
		PORTD = disp_value[counter];	// Se muestra el valor de la lista en el display
		break;
		
		case 3:				// Caso bandera = 2
		PORTD = disp_value[counter];	// Se muestra el valor de la lista en el display
		break;
		
		case 2:
		PORTD = disp_value[counter];	// Se muestra el valor de la lista en el display
		break;
		
		case 1:
		PORTD = disp_value[counter];	// Se muestra el valor de la lista en el display
		break;
		
		case 0:
		PORTD = disp_value[counter];	// Se muestra el valor de la lista en el display
		counter = 6;
		TIMSK0  = (0 << TOIE0);					// Se apagan las interrupciones del timer0
		break;
	}
}
