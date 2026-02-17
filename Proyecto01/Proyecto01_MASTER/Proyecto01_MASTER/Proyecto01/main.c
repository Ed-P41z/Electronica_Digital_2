/*
 * Proyecto01.c
 *
 * Created: 5/02/2026 19:26:24
 * Author : Angie
 */ 


//************************************************************************************
// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
//#include <util/delay.h>
#include "I2C/I2C.h"
#include "LCD/LCD.h"

#define slave1 0x30
#define slave2 0x40

#define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectira
#define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir


uint8_t direccion;
uint8_t temp;
uint8_t bufferI2C = 0;
uint32_t lectura_adc; 
uint32_t centenas;
uint32_t decenas;
uint32_t unidades;
uint8_t refreshLCD = 0;

//************************************************************************************
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);
void show_LCD();

void setup()
{
	//DDRB |= (1 << DDB5);
	initLCD8bitsBD();
	DDRB |= (1<<DDB5);
	PORTB &= ~(1<<PORTB5);
}

//************************************************************************************
// Main Function
int main(void)
{
	setup();
	
	//Puerto de salida para leds (en este caso sería el LCD)
	
	I2C_Master_Init(100000, 1); //Inicializar como MAster F_SCL 100kHz, prescaler 1
	
	while (1)
	{
		
		PORTB |= (1 << PORTB5); //Solo sirve para ver si está enviando datos
		
		if(!I2C_Master_Start()) return; //Si no recibe dato solo se regresa
		
		if(!I2C_Master_Write(slave1W)){ //Mandar la dirección de escritura, si no recibo el ACK manda el stop
			I2C_Master_Stop();
			return;
		}
		I2C_Master_Write('R'); //Comando para leer "te voy a leer"
		
		//Ahora configuro para leer
		if (!I2C_Master_RepeatedStart()){ // Si no recibe nada reinicia
			I2C_Master_Stop();// SI no recibo nada termino la comunicación con return
			return;
		}
		if(!I2C_Master_Write(slave1R)){ //Si si se da la comunicación entonces leer
			I2C_Master_Stop();
			return;
		}
		//Esto se ejecuta en el caso que si se da la comunicación
		I2C_Master_Read(&bufferI2C, 0); //Esto es solo para recibir un solo byte de info ya que no se manda ACK
		
		//Esto es para recopilar varios bytes ya que si se manda ACK
		//I2C_Master_Read(&bufferI2C1, 1);
		//I2C_Master_Read(&bufferI2C2, 1);
		//I2C_Master_Read(&bufferI2C3, 1);
		
		I2C_Master_Stop(); //Finalizamos
		
		PORTB &= ~(1 << PORTB5); //Apago el led para indicar que ya no hay comunicación
		
		lectura_adc = bufferI2C;
		
		show_LCD();
		_delay_ms(10);
	}
}

//************************************************************************************
// NON-INterrupt subroutines
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

void show_LCD()
{
	
	LCD_Set_Cursor8bitBD(2, 1);
	LCD_Write_String8bitBD("S1: ");
	LCD_Set_Cursor8bitBD(1,2);
	
	// Convertir a ASCII y enviar dígito por dígito
	centenas = lectura_adc/100;					// Unicamente se queda la parte entera
	decenas = (lectura_adc % 100) / 10;			// Se utiliza residuo para obtener decenas
	unidades = lectura_adc % 10;				// Se utiliza residuo para obtener unidades
	
	LCD_Write_Char8bitBD(centenas + '0');
	LCD_Write_Char8bitBD(decenas + '0');
	LCD_Write_Char8bitBD(unidades + '0');
	
}
//************************************************************************************
// Interrupt subroutines
