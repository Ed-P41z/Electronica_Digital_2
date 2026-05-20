/*
 * Neopixel.h
 *
 *  Created on: May 14, 2026
 *      Author: edvin
 */

#ifndef INC_NEOPIXEL_H_
#define INC_NEOPIXEL_H_

#include "main.h"
#include "math.h"

// Numero de leds
#define numPixels 8

// valores de ancho de pulso del uno y el cero
#define CCR_0 34
#define CCR_1 67
// colocar el timer y canal usado
extern TIM_HandleTypeDef htim1;
// coloque el timer usado
#define neoPixel_timer htim1
#define neoPixel_canal TIM_CHANNEL_1

// función para aplicar el brillo con la función gamma
#define GAMMA_CORRECTION 2.2f // Valor de correción gamma adaptada a ... (no se lee en la grabación)
#define GAMMA 2.2f // Valor de correción gamma
#define MAX_BRIGHTNESS 255 // Valor máximo de brillo


void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void setBrightness(uint8_t b);
void pixelShow(void);
void pixelClear(void);

uint8_t Gamma_correccion(uint8_t color, float brillo_); // No se usa en general

#endif /* INC_NEOPIXEL_H_ */
