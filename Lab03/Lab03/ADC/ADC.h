/*
 * ADC.h
 *
 * Created: 1/22/2026 7:10:53 PM
 *  Author: edvin
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#ifndef ADC_H_
#define ADC_H_

void initADC(void);

void ADC_to_Voltage(uint8_t ADC_Value, uint16_t *Voltage);

void ADC_to_Decimal(uint8_t ADC_Value, uint16_t *Decimal);


#endif /* ADC_H_ */