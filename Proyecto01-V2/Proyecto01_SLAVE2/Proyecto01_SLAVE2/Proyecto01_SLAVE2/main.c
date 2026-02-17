/*
 * Proyecto01_SLAVE2.c
 *
 * Created: 5/02/2026 19:40:26
 * Author : Angie
 */ 


//************************************************************************************
//================================== ESCLAVO =====================================

// Encabezado (librerías)
# include <avr/io.h>
# include <stdint.h>
# define F_CPU 16000000
# include <util/delay.h>
# include <avr/interrupt.h>
# include "I2C/I2C.h" 
# include "ADC/ADC.h"
# include "HX711/HX711.h"
#include "UART/UART.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
# define SlaveAddress 0x40

uint8_t buffer = 0;
uint8_t valor_ADC;
volatile uint8_t adc_flag = 0;
uint8_t estado;
uint16_t distance = 0;
uint16_t tiempo = 0;
int32_t offset;
volatile int32_t hx_raw = 0;
volatile uint16_t peso_g = 0;
//int32_t offset = 193771;   // el que mediste (o el que obtengas en tare)
int32_t scale  = 394;      // cuentas por gramo


//************************************************************************************
// Function prototypes

void distancia();

//************************************************************************************
// Main Function
int main(void)
{
	
	initHX711();
	_delay_ms(200);               // estabilizar HX711

	// 3) Calcular offset (tara) al encender (sin carga)
	offset = HX711_Tare(16);

	// 4) Init I2C Slave
	I2C_Slave_Init(SlaveAddress);

	// 5) Habilitar interrupciones globales
	sei();

	// 6) Loop principal: mantener peso actualizado aunque el master no lo pida
	while (1)
	{
		if (HX711_IsReady())              // solo cuando hay dato nuevo
		{
			int32_t raw = HX711_ReadRaw();  // lectura cruda (signed)

			int32_t diff = raw - offset;    // quitar tara
			if (diff < 0) diff = -diff;     // valor absoluto (porque tu señal está invertida)

			// convertir a gramos (entero)
			uint16_t g = (uint16_t)(diff / scale);

			// guardar global para que ISR lo envíe
			peso_g = g;
		}

		_delay_ms(2);
	}
}

//************************************************************************************
// NON-INterrupt subroutines


//************************************************************************************
// Interrupt subroutines
ISR(TWI_vect){
	estado = TWSR & 0xFC; //Nos quedamos unicamente con los bits de estado TWI Status
	switch(estado){
		//**************************
		// Slave debe recibir dato
		//**************************
		case 0x60: //SLA+W recibido
		case 0x70: //General call
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE); //Indica "si te escuche"
			break;
		
		case 0x80: //Dato recibido, ACK enviado
		case 0x90: //Dato recibido General call, ACK enviado
			buffer = TWDR; //Ya puedo utilizar los datos
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		//*****************************
		// Slave debe transmitir dato
		//*****************************
		//en cada case hay un comando que ya está predeterminado (ver presentacion)
		case 0xA8: // SLA+R recibido -> primer byte
		TWDR = (uint8_t)(peso_g >> 8); // MSB
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;

		case 0xB8: // Byte transmitido, ACK recibido -> segundo byte
		TWDR = (uint8_t)(peso_g & 0xFF); // LSB
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		//IMPORTANTE: que pasa si quiero enviar más de un dato?
		//Se puede hacer un arreglo por cada vez que envío un dato (ver grabación clase 1:08:11)
		
		case 0xC0: //Dato transmitido, NACK recibido
		case 0xC8: //Último dato transmitido
			TWCR = 0; //Limpio la interfaz para rebibir nuevo dato
			//TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN); //Reiniciarse
			TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE); // codigo correcto?
			break;
			
		case 0xA0: // STOP o repeated START recibido como slave
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		//**********************
		// Cualquier error
		//**********************
		default:
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
			break;
		
	}
	
}


