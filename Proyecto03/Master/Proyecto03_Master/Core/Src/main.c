/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "bitmaps.h"
#include "Neopixel.h"
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// Estados lógicos del display
#define SEG_ON   GPIO_PIN_SET
#define SEG_OFF  GPIO_PIN_RESET

// Tamaño de buffers I2C
#define TXBUFFERSIZE 1
#define RXBUFFERSIZE 1

int count = 0;

// Buffers para comunicación I2C
uint8_t aTxBuffer[TXBUFFERSIZE];
uint8_t aRxBuffer[RXBUFFERSIZE];

// Estado recibido por I2C
volatile uint8_t sensor_byte_rx = 0;
volatile uint8_t i2c_rx_flag = 0;
uint8_t sensor_byte_prev = 0;

// Variables de prueba por UART
uint8_t ctrl_cmd2;
uint8_t rx_data2;

// Control del contador de parqueos
uint8_t show_num = 0;
uint8_t cont_parqueos = 8;

// Estado individual de cada parqueo
uint8_t car_state1 = 0;
uint8_t car_state2 = 0;
uint8_t car_state3 = 0;
uint8_t car_state4 = 0;
uint8_t car_state5 = 0;
uint8_t car_state6 = 0;
uint8_t car_state7 = 0;
uint8_t car_state8 = 0;

// Fondo principal de la TFT
extern const uint16_t fondo_parqueo[];

// Dimensiones de pantalla y fondo
#define LCD_W 320
#define LCD_H 240
#define FONDO_W 320
#define COLOR_TRANSPARENTE 0x738e

// Dimensiones de sprites
#define CARRO_AZUL1_W 36
#define CARRO_AZUL2_W 32
#define CARRO_AZUL3_W 25
#define CARRO_ROJO1_W 36
#define CARRO_ROJO2_W 33
#define CARRO_VERDE_W 48
#define LA_TROCONA_W 39
#define CARRO_H 60

// Buffer temporal para redibujado
#define CARRO_BUFFER_MAX_W 64
#define CARRO_BUFFER_MAX_H 100
uint16_t buffer_carro[CARRO_BUFFER_MAX_W * CARRO_BUFFER_MAX_H];

// Datos necesarios para animar un carro
typedef struct {
    int x;
    int y;
    int y_destino;
    int paso_y;

    int w;
    int h;

    const uint16_t *sprite;

    uint8_t activo;
    uint8_t visible;

    uint32_t tiempo_anterior;
    uint32_t periodo_ms;
} CarroAnimado;

// Carros asociados a los 8 espacios
CarroAnimado carro1;
CarroAnimado carro2;
CarroAnimado carro3;
CarroAnimado carro4;
CarroAnimado carro5;
CarroAnimado carro6;
CarroAnimado carro7;
CarroAnimado carro8;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
// Funciones agregadas para animación y parqueos
void IniciarCarro(CarroAnimado *c, int x, int y_inicio, int y_destino, int paso_y,
                  int w, int h, const uint16_t *sprite, uint32_t periodo_ms);
void ActualizarCarro(CarroAnimado *c);
void BorrarCarro(CarroAnimado *c);
void DibujarCarroCompuesto(CarroAnimado *c, int nueva_y);
void ProcesarCambioParqueo(uint8_t cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Restaura el fondo donde estaba el carro
void BorrarCarro(CarroAnimado *c)
{
    int x0 = c->x;
    int y0 = c->y;
    int x1 = c->x + c->w - 1;
    int y1 = c->y + c->h - 1;

    // Recorte contra pantalla
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= LCD_W) x1 = LCD_W - 1;
    if (y1 >= LCD_H) y1 = LCD_H - 1;

    if (x0 > x1 || y0 > y1) return;

    int bw = x1 - x0 + 1;
    int bh = y1 - y0 + 1;

    if (bw > CARRO_BUFFER_MAX_W || bh > CARRO_BUFFER_MAX_H) return;

    int k = 0;

    for (int py = y0; py <= y1; py++)
    {
        for (int px = x0; px <= x1; px++)
        {
            buffer_carro[k++] = fondo_parqueo[(py * FONDO_W) + px];
        }
    }

    LCD_Bitmap(x0, y0, bw, bh, buffer_carro);
}

// Redibuja solo el área afectada
void DibujarCarroCompuesto(CarroAnimado *c, int nueva_y)
{
    int vieja_y = c->y;

    // Área total a actualizar: posición vieja + posición nueva
    int x0 = c->x;
    int y0 = (vieja_y < nueva_y) ? vieja_y : nueva_y;

    int x1 = c->x + c->w - 1;

    int vieja_y_final = vieja_y + c->h - 1;
    int nueva_y_final = nueva_y + c->h - 1;
    int y1 = (vieja_y_final > nueva_y_final) ? vieja_y_final : nueva_y_final;

    // Recorte contra pantalla
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= LCD_W) x1 = LCD_W - 1;
    if (y1 >= LCD_H) y1 = LCD_H - 1;

    if (x0 > x1 || y0 > y1) return;

    int bw = x1 - x0 + 1;
    int bh = y1 - y0 + 1;

    if (bw > CARRO_BUFFER_MAX_W || bh > CARRO_BUFFER_MAX_H) return;

    int k = 0;

    for (int py = y0; py <= y1; py++)
    {
        for (int px = x0; px <= x1; px++)
        {
            // Primero se coloca el fondo
            uint16_t color = fondo_parqueo[(py * FONDO_W) + px];

            // Luego se revisa si este pixel cae dentro del carro nuevo
            int sx = px - c->x;
            int sy = py - nueva_y;

            if (sx >= 0 && sx < c->w && sy >= 0 && sy < c->h)
            {
                uint16_t pixel_sprite = c->sprite[(sy * c->w) + sx];

                if (pixel_sprite != COLOR_TRANSPARENTE)
                {
                    color = pixel_sprite;
                }
            }

            buffer_carro[k++] = color;
        }
    }

    // Una sola escritura a pantalla: fondo + carro ya mezclados
    LCD_Bitmap(x0, y0, bw, bh, buffer_carro);
}

// Configura una entrada o salida de carro
void IniciarCarro(CarroAnimado *c, int x, int y_inicio, int y_destino, int paso_y,
                  int w, int h, const uint16_t *sprite, uint32_t periodo_ms)
{
    if (c->visible)
    {
        BorrarCarro(c);
    }

    c->x = x;
    c->y = y_inicio;
    c->y_destino = y_destino;
    c->paso_y = paso_y;

    c->w = w;
    c->h = h;
    c->sprite = sprite;

    c->activo = 1;
    c->visible = 1;

    c->tiempo_anterior = HAL_GetTick();
    c->periodo_ms = periodo_ms;
}

// Avanza la animación por tiempo
void ActualizarCarro(CarroAnimado *c)
{
    if (!c->activo) return;

    uint32_t ahora = HAL_GetTick();

    if (ahora - c->tiempo_anterior < c->periodo_ms)
    {
        return;
    }

    c->tiempo_anterior = ahora;

    int nueva_y = c->y + c->paso_y;

    // Detener exactamente en la posición deseada
    if (c->paso_y > 0 && nueva_y >= c->y_destino)
    {
        nueva_y = c->y_destino;
        c->activo = 0;
    }
    else if (c->paso_y < 0 && nueva_y <= c->y_destino)
    {
        nueva_y = c->y_destino;
        c->activo = 0;
    }

    DibujarCarroCompuesto(c, nueva_y);

    c->y = nueva_y;
}

// Aplica el cambio de estado de un parqueo
void ProcesarCambioParqueo(uint8_t cmd)
{
    switch(cmd)
	    	    {
	    	        case '1':
	    	            if (car_state1 == 0){
	    	            	IniciarCarro(&carro1,
	    	                         22,          // x
	    	                         -CARRO_H,     // y inicial fuera de pantalla arriba
	    	                         40,           // y destino
	    	                         1,            // paso en y
	    	                         CARRO_AZUL1_W,
	    	                         CARRO_H,
	    	                         carro_azul1,
	    	                         25);          // periodo de movimiento
	    	            	car_state1 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(0, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state1 == 1){
	    	            	IniciarCarro(&carro1,
	    	                         22,          // x
	    	                         40,     // y inicial fuera de pantalla arriba
	    	                         -CARRO_H,           // y destino
	    	                         -1,            // paso en y
	    	                         CARRO_AZUL1_W,
	    	                         CARRO_H,
	    	                         carro_azul1,
	    	                         25);          // periodo de movimiento
	    	            	car_state1 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(0, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '2':
	    	            if (car_state2 == 0){
	    	            	IniciarCarro(&carro2,
	    	                         78,          // x
	    	                         -CARRO_H,     // y inicial fuera de pantalla arriba
	    	                         40,           // y destino
	    	                         1,            // paso en y
	    	                         CARRO_ROJO1_W,
	    	                         CARRO_H,
	    	                         carro_rojo1,
	    	                         25);          // periodo de movimiento
	    	            	car_state2 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(1, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state2 == 1){
	    	            	IniciarCarro(&carro2,
	    	                         78,          // x
	    	                         40,     // y inicial fuera de pantalla arriba
	    	                         -CARRO_H,           // y destino
	    	                         -1,            // paso en y
	    	                         CARRO_ROJO1_W,
	    	                         CARRO_H,
	    	                         carro_rojo1,
	    	                         25);          // periodo de movimiento
	    	            	car_state2 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(1, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '3':
	    	        	if (car_state3 == 0){
	    	            	IniciarCarro(&carro3,
	    	                         131,          // x
	    	                         -CARRO_H,     // y inicial fuera de pantalla arriba
	    	                         40,           // y destino
	    	                         1,            // paso en y
	    	                         LA_TROCONA_W,
	    	                         CARRO_H,
	    	                         la_trocona,
	    	                         25);          // periodo de movimiento
	    	            	car_state3 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(2, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state3 == 1){
	    	            	IniciarCarro(&carro3,
	    	                         131,          // x
	    	                         40,     // y inicial fuera de pantalla arriba
	    	                         -CARRO_H,           // y destino
	    	                         -1,            // paso en y
	    							 LA_TROCONA_W,
	    	                         CARRO_H,
	    							 la_trocona,
	    	                         25);          // periodo de movimiento
	    	            	car_state3 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(2, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '4':
	    	        	if (car_state4 == 0){
	    	            	IniciarCarro(&carro4,
	    	                         182,          // x
	    	                         -CARRO_H,     // y inicial fuera de pantalla arriba
	    	                         40,           // y destino
	    	                         1,            // paso en y
	    	                         CARRO_VERDE_W,
	    	                         CARRO_H,
	    	                         carro_verde,
	    	                         25);          // periodo de movimiento
	    	            	car_state4 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(3, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state4 == 1){
	    	            	IniciarCarro(&carro4,
	    	                         182,          // x
	    	                         40,     // y inicial fuera de pantalla arriba
	    	                         -CARRO_H,           // y destino
	    	                         -1,            // paso en y
	    							 CARRO_VERDE_W,
	    	                         CARRO_H,
	    							 carro_verde,
	    	                         25);          // periodo de movimiento
	    	            	car_state4 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(3, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '5':
	    	        	if (car_state5 == 0){
	    	            	IniciarCarro(&carro5,
	    	                         18,          // x
	    	                         LCD_H,     // y inicial fuera de pantalla arriba
	    	                         138,           // y destino
	    	                         -1,            // paso en y
	    	                         CARRO_VERDE_W,
	    	                         CARRO_H,
	    	                         carro_verde,
	    	                         25);          // periodo de movimiento
	    	            	car_state5 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(7, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state5 == 1){
	    	            	IniciarCarro(&carro5,
	    	                         18,          // x
	    	                         138,     // y inicial fuera de pantalla arriba
	    							 LCD_H,           // y destino
	    	                         1,            // paso en y
	    							 CARRO_VERDE_W,
	    	                         CARRO_H,
	    							 carro_verde,
	    	                         25);          // periodo de movimiento
	    	            	car_state5 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(7, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '6':
	    	        	if (car_state6 == 0){
	    	            	IniciarCarro(&carro6,
	    	                         80,          // x
	    							 LCD_H,     // y inicial fuera de pantalla arriba
	    	                         138,           // y destino
	    	                         -1,            // paso en y
	    	                         CARRO_AZUL2_W,
	    	                         CARRO_H,
	    	                         carro_azul2,
	    	                         25);          // periodo de movimiento
	    	            	car_state6 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(6, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state6 == 1){
	    	            	IniciarCarro(&carro6,
	    	                         80,          // x
	    	                         138,     // y inicial fuera de pantalla arriba
	    							 LCD_H,           // y destino
	    	                         1,            // paso en y
	    							 CARRO_AZUL2_W,
	    	                         CARRO_H,
	    							 carro_azul2,
	    	                         25);          // periodo de movimiento
	    	            	car_state6 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(6, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '7':
	    	        	if (car_state7 == 0){
	    	            	IniciarCarro(&carro7,
	    	            			 134,          // x
	    							 LCD_H,     // y inicial fuera de pantalla arriba
	    	                         138,           // y destino
	    	                         -1,            // paso en y
	    	                         CARRO_ROJO2_W,
	    	                         CARRO_H,
	    	                         carro_rojo2,
	    	                         25);          // periodo de movimiento
	    	            	car_state7 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(5, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state7 == 1){
	    	            	IniciarCarro(&carro7,
	    	                         134,          // x
	    	                         138,     // y inicial fuera de pantalla arriba
	    							 LCD_H,           // y destino
	    	                         1,            // paso en y
	    							 CARRO_ROJO2_W,
	    	                         CARRO_H,
	    							 carro_rojo2,
	    	                         25);          // periodo de movimiento
	    	            	car_state7 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(5, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;

	    	        case '8':
	    	        	if (car_state8 == 0){
	    	            	IniciarCarro(&carro8,
	    	            			 190,          // x
	    							 LCD_H,     // y inicial fuera de pantalla arriba
	    	                         138,           // y destino
	    	                         -1,            // paso en y
	    	                         CARRO_AZUL3_W,
	    	                         CARRO_H,
	    	                         carro_azul3,
	    	                         25);          // periodo de movimiento
	    	            	car_state8 = 1;
	    	            	cont_parqueos--;
	    	            	show_num = 1;
	    	            	setPixelColor(4, 255, 0, 0); // Rojo
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            else if (car_state8 == 1){
	    	            	IniciarCarro(&carro8,
	    	                         190,          // x
	    	                         138,     // y inicial fuera de pantalla arriba
	    							 LCD_H,           // y destino
	    	                         1,            // paso en y
	    							 CARRO_AZUL3_W,
	    	                         CARRO_H,
	    							 carro_azul3,
	    	                         25);          // periodo de movimiento
	    	            	car_state8 = 0;
	    	            	cont_parqueos++;
	    	            	show_num = 1;
	    	            	setPixelColor(4, 0, 255, 0); // Verde
	    	            	setBrightness(100);
	    	            	pixelShow();
	    	            }
	    	            break;
	    	    }
}

// Escribe los segmentos físicos del display
static void Display7Seg_Write(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                              uint8_t e, uint8_t f, uint8_t g, uint8_t dp)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9,  a  ? SEG_ON : SEG_OFF); // a
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, b  ? SEG_ON : SEG_OFF); // b
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,  c  ? SEG_ON : SEG_OFF); // c
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,  d  ? SEG_ON : SEG_OFF); // d
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7,  e  ? SEG_ON : SEG_OFF); // e
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, f  ? SEG_ON : SEG_OFF); // f
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0,  g  ? SEG_ON : SEG_OFF); // g
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,  dp ? SEG_ON : SEG_OFF); // dp
}

// Apaga todos los segmentos
void Display7Seg_Clear(void)
{
    Display7Seg_Write(0, 0, 0, 0, 0, 0, 0, 0);
}

// Muestra un dígito en el display
void Display7Seg_ShowDigit(uint8_t num)
{
    switch (num)
    {
        case 0:
            Display7Seg_Write(1, 1, 1, 1, 1, 1, 0, 0);
            break;

        case 1:
            Display7Seg_Write(0, 1, 1, 0, 0, 0, 0, 0);
            break;

        case 2:
            Display7Seg_Write(1, 1, 0, 1, 1, 0, 1, 0);
            break;

        case 3:
            Display7Seg_Write(1, 1, 1, 1, 0, 0, 1, 0);
            break;

        case 4:
            Display7Seg_Write(0, 1, 1, 0, 0, 1, 1, 0);
            break;

        case 5:
            Display7Seg_Write(1, 0, 1, 1, 0, 1, 1, 0);
            break;

        case 6:
            Display7Seg_Write(1, 0, 1, 1, 1, 1, 1, 0);
            break;

        case 7:
            Display7Seg_Write(1, 1, 1, 0, 0, 0, 0, 0);
            break;

        case 8:
            Display7Seg_Write(1, 1, 1, 1, 1, 1, 1, 0);
            break;

        case 9:
            Display7Seg_Write(1, 1, 1, 1, 0, 1, 1, 0);
            break;

        default:
            Display7Seg_Clear();
            break;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // Inicialización de buffers y banderas
  aTxBuffer[0] = 0;
  aRxBuffer[0] = 0;
  sensor_byte_rx = 0;
  i2c_rx_flag = 0;

  	  // Habilita la escucha I2C
  	  if (HAL_I2C_EnableListen_IT(&hi2c1) != HAL_OK){
  		  Error_Handler();
  	  }

  HAL_UART_Receive_IT(&huart2, &rx_data2, 1);

  // Inicialización de NeoPixels
  pixelClear();
  setPixelColor(0, 0, 255, 0); // Rojo
  setPixelColor(1, 0, 255, 0); // Verde
  setPixelColor(2, 0, 255, 0);
  setPixelColor(3, 0, 255, 0);

  setPixelColor(4, 0, 255, 0); // Rojo
  setPixelColor(5, 0, 255, 0); // Verde
  setPixelColor(6, 0, 255, 0);
  setPixelColor(7, 0, 255, 0);

  setBrightness(100);
  pixelShow();

  // Inicialización de la TFT
  LCD_Init();
  LCD_Clear(0x738e);

  LCD_Bitmap(0, 0, 320, 240, fondo_parqueo);

  // Muestra los espacios libres iniciales
  Display7Seg_ShowDigit(cont_parqueos);
  show_num = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // Actualiza las animaciones activas
	  ActualizarCarro(&carro1);
	  ActualizarCarro(&carro2);
	  ActualizarCarro(&carro3);
	  ActualizarCarro(&carro4);
	  ActualizarCarro(&carro5);
	  ActualizarCarro(&carro6);
	  ActualizarCarro(&carro7);
	  ActualizarCarro(&carro8);



	    // Actualiza el contador si cambió
	    if (show_num == 1){
	    	switch(cont_parqueos){
	    	case 0:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, cero);
		    	show_num = 0;
	    	break;

	    	case 1:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, uno);
		    	show_num = 0;
	    	break;

	    	case 2:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, dos);
		    	show_num = 0;
	    	break;

	    	case 3:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, tres);
		    	show_num = 0;
	    	break;

	    	case 4:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, cuatro);
		    	show_num = 0;
	    	break;

	    	case 5:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, cinco);
		    	show_num = 0;
	    	break;

	    	case 6:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, seis);
		    	show_num = 0;
	    	break;

	    	case 7:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, siete);
		    	show_num = 0;
	    	break;

	    	case 8:
	    		Display7Seg_ShowDigit(cont_parqueos);
	    		LCD_Bitmap(238, 50, 80, 149, ocho);
		    	show_num = 0;
	    	break;
	    	}
	    }

	    // Procesa el byte recibido por I2C
	    if (i2c_rx_flag)
	    {
	        i2c_rx_flag = 0;

	        uint8_t cambios = sensor_byte_rx ^ sensor_byte_prev;

	        for (uint8_t i = 0; i < 8; i++)
	        {
	            if (cambios & (1 << i))
	            {
	                ProcesarCambioParqueo('1' + i);
	            }
	        }

	        sensor_byte_prev = sensor_byte_rx;
	    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 78;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 105-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RESET_Pin|Disp_e_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Disp_g_Pin|Disp_dp_Pin|LCD_DC_Pin|Disp_a_Pin
                          |Disp_f_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Disp_b_Pin|Disp_c_Pin|Disp_d_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Disp_g_Pin Disp_dp_Pin Disp_a_Pin Disp_f_Pin */
  GPIO_InitStruct.Pin = Disp_g_Pin|Disp_dp_Pin|Disp_a_Pin|Disp_f_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Disp_b_Pin Disp_c_Pin Disp_d_Pin */
  GPIO_InitStruct.Pin = Disp_b_Pin|Disp_c_Pin|Disp_d_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Disp_e_Pin */
  GPIO_InitStruct.Pin = Disp_e_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Disp_e_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  // Pines exactos del display de 7 segmentos
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  // PC9 = a, PC7 = e
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // PB10 = b, PB4 = c, PB5 = d
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // PA10 = f, PA0 = g, PA1 = dp
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_0 | GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  Display7Seg_Clear();
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Callback de recepción UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) // Terminal para pruebas
    {
    	ctrl_cmd2 = rx_data2;

        //HAL_UART_Transmit(&huart2, &rx_data, 1, 100);

        HAL_UART_Receive_IT(&huart2, &rx_data2, 1);

    }
}

// Reactiva la escucha I2C
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
	HAL_I2C_EnableListen_IT(hi2c);
}

// Atiende la comunicación del maestro I2C
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
                          uint16_t AddrMatchCode)
{
    if (hi2c->Instance == I2C1)
    {
        if (TransferDirection == I2C_DIRECTION_TRANSMIT)
        {
            if (HAL_I2C_Slave_Seq_Receive_IT(hi2c, aRxBuffer, 1,
                                             I2C_FIRST_AND_LAST_FRAME) != HAL_OK)
            {
                HAL_I2C_EnableListen_IT(hi2c);
            }
        }
        else if (TransferDirection == I2C_DIRECTION_RECEIVE)
        {
            if (HAL_I2C_Slave_Seq_Transmit_IT(hi2c, aTxBuffer, 1,
                                              I2C_FIRST_AND_LAST_FRAME) != HAL_OK)
            {
                HAL_I2C_EnableListen_IT(hi2c);
            }
        }
    }
}

// Callback al terminar transmisión I2C
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    //aTxBuffer[0] = sensor_state;
}

// Guarda el dato recibido por I2C
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    if (I2cHandle->Instance == I2C1)
    {
        sensor_byte_rx = aRxBuffer[0];
        i2c_rx_flag = 1;
    }
}

// Recupera I2C si ocurre error
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
    uint32_t error = HAL_I2C_GetError(I2cHandle);

    if (I2cHandle->Instance == I2C1)
    {
        HAL_I2C_DisableListen_IT(I2cHandle);
        HAL_I2C_DeInit(I2cHandle);
        HAL_I2C_Init(I2cHandle);
        HAL_I2C_EnableListen_IT(I2cHandle);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
