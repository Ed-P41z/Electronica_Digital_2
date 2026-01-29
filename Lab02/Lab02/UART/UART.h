/*
 * UART.h
 *
 * Created: 1/28/2026 1:54:00 AM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#ifndef UART_H_
#define UART_H_

void initUART();

void writeChar(char caracter);

void writeString (char* cadena);

#endif /* UART_H_ */