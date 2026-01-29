/*
 * Lab02.c
 *
 * Created: 22/01/2026 12:01:42 PM
 * Author: edvin
 * Description: El lab consiste en realizar una cuenta regresiva con un display
 */
//*********************************************
// Encabezado (Libraries)
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LCD/LCD.h"
#include "ADC/ADC.h"
#include "UART/UART.h"

uint8_t	counter;
uint8_t adc_read;
uint8_t adc_flag;
uint8_t counter_ADC;
uint8_t mostrar_menu;
uint8_t envio_ascii;
uint8_t recibido_ascii;
uint8_t verif;
uint8_t counter_UART;
uint16_t Voltage;
uint16_t Decimal;
uint16_t u_adc;
uint16_t d_adc;
uint16_t c_adc;
uint16_t m_adc;
char recibido;
char r;
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
	
	LCD_CMD8bitBD(0x01); // Se inicializa la LCD
	_delay_ms(2);
	
	LCD_Set_Cursor8bitBD(1,1);	// Se coloca el cursor al inicio para mandar el menú inicial
	LCD_Write_String8bitBD("S1:   S2:  S3:");	// Se manda la primera fila de títutlos
	LCD_Set_Cursor8bitBD(1,2);	// Se coloca el cursor al inicio de la segunda fila
	LCD_Write_String8bitBD("0.00V 0000 000");	// Se manda la segunda fila de los valores default
	
	while(1)	// Entra al bucle infinito en donde se ejecuta el programa
	{	
		r = recibido;	// El valor recibido en el UART se guarda en una variable que determina si hubo un cambio en el UART
		if (r != 0)		// Si detecta cambio en el valor de r entra a la acción del UART
		{
			recibido = 0;

			if (r == '+')	// Si el valor de r es el caracter '+' entra al if
			{
				writeString("\n Suma a contador ");
				if (counter_UART < 255) 
				{
					counter_UART++;	// Detecta que no haya oferflow y luego suma 1 al contador
				}
				mostrar_menu = 1; // Vuelve a mostrar el menú en UART
			}
			else if (r == '-')	// Si el valor de r es el caracter '-' entra al if
			{
				writeString("\n Resta a contador: ");
				if (counter_UART > 0)
				{
					 counter_UART--;	// Detecta que no haya underflow y luego resta 1 al contador
				}
				mostrar_menu = 1;	// Vuelve a mostrar el menú en UART
			}
			else if (r == '1')	// Si el valor de r es el caracter '1' entra al if
			{
				mostrar_menu = 1;	// Enciende la bandera para volver a mostrar el menú
				writeString("\n S1: ");
				c_adc = Voltage / 100;
				d_adc = (Voltage % 100) / 10;
				u_adc = Voltage % 10;			// Lógica para separa el contador en centenas, decenas y unidades
				writeChar(c_adc + '0');
				writeChar('.');
				writeChar(d_adc + '0');
				writeChar(u_adc + '0');
				writeChar('V');					// Envía caracter por caracter el valor del voltaje en formato 0.00V
				
				writeString("  S2: ");
				m_adc = Decimal / 1000;
				c_adc = (Decimal % 1000) / 100;
				d_adc = (Decimal % 100) / 10;
				u_adc = Decimal % 10;			// Lógica para separa el contador en millares, centenas, decenas y unidades
				writeChar(m_adc + '0');
				writeChar(c_adc + '0');
				writeChar(d_adc + '0');
				writeChar(u_adc + '0');			// Envía caracter por caracter el valor del valor de 0000-1023
			}

			c_adc = counter_UART / 100;
			d_adc = (counter_UART % 100) / 10;
			u_adc = counter_UART % 10;			// Una vez se operó el contador del UART lo descompone en centenas, decenas y unidades

			LCD_Set_Cursor8bitBD(12,2);			// Coloca el cursor de manera que se modifique la fila 2 posición 12
			LCD_Write_Char8bitBD(c_adc + '0');
			LCD_Write_Char8bitBD(d_adc + '0');
			LCD_Write_Char8bitBD(u_adc + '0');	// Manda el valor del contador del UART caracter por caracter
		}
		else if (mostrar_menu)	// Si la bandera de mostrar menú está encendida entra al if
		{
			writeString("\n -------------------------------------------------------------------- ");
			writeString("\n Presione +, -, o 1 para interactuar con el menu: ");
			writeString("\n 1 Muestra el estado de los potenciometros.");
			writeString("\n + Suma al contador.");
			writeString("\n - Resta al contador.");
			mostrar_menu = 0;  // Ya se mostró el menú, no volver a mostrar hasta que sea necesario
		}
		else if (adc_flag == 1) // Ingresa cada vez que se actualiza
		{
			adc_flag = 0; // Se apaga la bandera de acción de ADC
			
			switch(counter_ADC) // Cambia de servo dependiendo de counter_ADC
			{
				case 0:
				counter_ADC++;		// Se suma el contador que sirve para multiplexar
				ADC_to_Voltage(adc_read, &Voltage);

				c_adc = Voltage / 100;
				d_adc = (Voltage % 100) / 10;
				u_adc = Voltage % 10;			// Lógica para separa el contador en centenas, decenas y unidades

				LCD_Set_Cursor8bitBD(1,2);		// Coloca el cursor para modificar el voltaje en el LCD

				LCD_Write_Char8bitBD(c_adc + '0');
				LCD_Write_Char8bitBD('.');
				LCD_Write_Char8bitBD(d_adc + '0');
				LCD_Write_Char8bitBD(u_adc + '0');
				LCD_Write_Char8bitBD('V');			// Envía caracter por caracter el voltaje en formato 0.00V
				ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0));
				ADMUX |= (1<<MUX0); // Cambia para leer ADC de PC1
				break;
				
				case 1:
				counter_ADC = 0;					// Regresa al caso 0 de la multiplexación
				ADC_to_Decimal(adc_read, &Decimal);

				m_adc = Decimal / 1000;               
				c_adc = (Decimal % 1000) / 100;       
				d_adc = (Decimal % 100) / 10;         
				u_adc = Decimal % 10;                 // Lógica para decomponer Decimal en millares, centenas, decenas y unidades

				LCD_Set_Cursor8bitBD(7,2);			// Coloca el cursor para que modifique el valor de  S2

				LCD_Write_Char8bitBD(m_adc + '0');
				LCD_Write_Char8bitBD(c_adc + '0');
				LCD_Write_Char8bitBD(d_adc + '0');
				LCD_Write_Char8bitBD(u_adc + '0');	// Envía caracter por caracter el valor de Decimal de 0000-1023
				
				c_adc = counter_UART / 100;
				d_adc = (counter_UART % 100) / 10;
				u_adc = counter_UART % 10;			// Lógica para descomponer el contador de UART en centenas, decenas y unidades
				
				LCD_Set_Cursor8bitBD(12,2);			// Se coloca el cursor de manera que modifique el contador de UART en el LCD
				LCD_Write_Char8bitBD(c_adc + '0');
				LCD_Write_Char8bitBD(d_adc + '0');
				LCD_Write_Char8bitBD(u_adc + '0');	// Manda caracter por caracter el valor del contador al LCD
				
				ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)); // Cambia para leer ADC de PC0
				break;
			}
			
			ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
		}
	}
}

//*********************************************
// NON-Interrupt subroutines
void setup()
{
	cli(); // Se apagan las interruciones globales
	
	// Configurar presclaer de sistema
	//CLKPR	= (1 << CLKPCE);
	//CLKPR	= (1 << CLKPS2); // 16 PRESCALER -> 1MHz
	
	// Configuración de Pines
	DDRD |= 0xFC;			// Se configura PORTD como salida
	PORTD &= ~0xFC;			// PORTD inicialmente apagado
	DDRC	= 0xFF;			// Se configura PORTC como salida
	DDRC	&= ~((1 << DDC0) | (1 << DDC1)); // Se configura PC0 y PC1 como input
	PORTC	= 0x00;			// PORTC inicialmente apagado
	PORTC &= ~((1<<PORTC0) | (1<<PORTC1));
		
	// Inicio de Timers
	initTMR0();	
	
	// Inicio de ADC
	initADC();
	ADCSRA	|= (1 << ADSC);	// Se hace la primera lectura del ADC
	
	// Inicio del LCD
	initLCD8bitsBD();
	
	// Inicio de UART
	initUART();

	// Configuración de Interrupciones
	
	
	// Inicialización de Variables
	counter	= 0;
	mostrar_menu = 1;
	
	sei(); // Se encienden las interrupciones globales
}

void initTMR0()
{
	TCCR0A  = 0;							// Se usa el modo normal
	TCCR0B  |= (1 << CS02) | (1 << CS00);	// Se configura el prescaler
	TCNT0   = TCNT0_Value;					// Se carga el valor al TCNT0
	TCNT0   = TCNT0_Value;					// Se carga el valor al TCNT0
	TIMSK0  = (1 << TOIE0);					// Se encienden las interrupciones del timer0
}

//*********************************************
// Interrupt routines

ISR(USART_RX_vect)
{
	recibido = UDR0;  // Leer el carácter recibido desde el registro de UART
	
	if (envio_ascii == 1)
	{
		recibido_ascii = recibido;  // Se guarda el valor recibido
		envio_ascii = 0;	// Se apaga la bandera para no reingresar al if en el menú principal
		verif = 1;			// Se activa la verificación para salir del bucle principal en el main
	}
}

ISR(ADC_vect)
{
	adc_read = ADCH;			// Se guarda ADCH
	adc_flag = 1;
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = TCNT0_Value;	// Se carga el valor a TCNT0
	counter++;	// Se resta al valor del contador del display para la cuenta regresiva del juego
	
	if (counter == 5)	// Una vez el contador llega a cero entonces se enciende la bander de inicio del juego
	{
		 ADCSRA	|= (1 << ADSC);	// Se realiza la lectura de ADC
		 counter = 0;
	}
	
}