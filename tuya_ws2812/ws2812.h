/*
 * @Author: hequan 
 * @Date: 2021-06-24 18:42:37 
 * @Last Modified by: hequan
 * @Last Modified time: 2021-06-24 23:33:03
 */
#ifndef WS2812_H
#define WS2812_H
#include <Adafruit_NeoPixel.h>

#define WS2812_PIN 2

enum wsMode
{
    STATIC,
    GRADIENT,
    JUMP,
    BREATHE,
    BLINK,
    FLOW = 10,
    RAINBORW
};

typedef struct
{
    uint32_t color[15];
    uint8_t color_len;
    uint8_t change_time;
} Color_T;

typedef struct
{
    uint8_t state;
    wsMode ws_mode;
    uint32_t colour;
    uint8_t led_len;
} ws2812_T;

void ws2812_Init();
void ws2812_Server();
void ws2812_Static();
u32 __str2short(u32 a, u32 b, u32 c, u32 d);
u8 __asc2hex(u8 asccode);

extern ws2812_T ws2812_data;
extern Adafruit_NeoPixel pixels;
extern Color_T color_data;
#endif