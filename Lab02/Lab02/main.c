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

uint8_t	counter;
uint8_t adc_read;
uint8_t adc_flag;
uint8_t counter_ADC;
uint16_t Voltage;
uint16_t u_adc;
uint16_t d_adc;
uint16_t c_adc;
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
	
	
	initLCD8bits();
	LCD_Set_Cursor8bit(1,1);
	LCD_Write_String8bit("S1:");
	LCD_Set_Cursor8bit(1,2);
	LCD_Write_String8bit("0.00V");
	
	while(1)	// Entra al bucle infinito en donde se ejecuta el programa
	{
		if (adc_flag)
		{
			adc_flag = 0;

			ADC_to_Voltage(adc_read, &Voltage);

			/*unsigned long V_local;
			uint8_t adc_local;

			adc_flag = 0;

			adc_local = adc_read;   // atómico
			V_local = ((unsigned long)adc_local * 500UL) / 255UL;

			c_adc = V_local / 100;
			d_adc = (V_local % 100) / 10;
			u_adc = V_local % 10;
			*/
			c_adc = Voltage / 100;
			d_adc = (Voltage % 100) / 10;
			u_adc = Voltage % 10;

			LCD_Set_Cursor8bit(1,2);   // <-- clave: volver al inicio del campo

			LCD_Write_Char8bit(c_adc + '0');
			LCD_Write_Char8bit('.');
			LCD_Write_Char8bit(d_adc + '0');
			LCD_Write_Char8bit(u_adc + '0');
			LCD_Write_Char8bit('V');
		}
		
		/*if (adc_flag == 1) // Ingresa cada vez que se actualiza
		{
			adc_flag = 0; // Se apaga la bandera de acción de ADC
			
			switch(counter_ADC) // Cambia de servo dependiendo de counter_ADC
			{
				case 0:
				counter_ADC++;		// Se suma el contador que sirve para multiplexar
				dutyCycle4 = ADC_to_PWM_ServoT1B(adc_read);	// Se llama a la función que mapea el ADC al servo
				updateDutyCycle_T1B(dutyCycle4);	// Se llama la función que hace la actualización al registro
				ADMUX	&= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));	// Se selecciona pc0
				break;
				
				case 1:
				counter_ADC = 0;
				dutyCycle1 = ADC_to_PWM_ServoT0A(adc_read);	// Se llama a la función que mapea el ADC al servo
				updateDutyCycle_T0A(dutyCycle1);	// Se llama la función que hace la actualización al registro
				ADMUX	&= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
				ADMUX	|= (1 << MUX0);	// Se selecciona pc1
				break;
			}
			
			ADCSRA	|= (1 << ADSC);				// Se realiza la lectura de ADC
		}*/
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
	DDRD	= 0xFF;			// Se configura PORTD como salida
	PORTD	= 0x00;			// PORTD inicialmente apagado
	DDRC	= 0xFF;			// Se configura PORTC como salida
	DDRC	&= ~((1 << DDC0) | (1 << DDC1)); // Se configura PC0 y PC1 como input
	PORTC	= 0x00;			// PORTC inicialmente apagado
	PORTC &= ~((1<<PORTC0) | (1<<PORTC1));
		
	// Inicio de Timers
	initTMR0();	
	
	// Inicio de ADC
	initADC();
	ADCSRA	|= (1 << ADSC);	// Se hace la primera lectura del ADC

	// Configuración de Interrupciones
	
	
	// Inicialización de Variables
	counter	= 0;
	
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