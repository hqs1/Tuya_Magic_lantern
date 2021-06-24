/*
 * @Author: hequan 
 * @Date: 2021-06-24 18:42:43 
 * @Last Modified by:   hequan 
 * @Last Modified time: 2021-06-24 18:42:43 
 */
#include "ws2812.h"
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Adafruit_NeoPixel pixels(12, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels;
ws2812_T ws2812_data;
Color_T color_data;
void ws2812_Init()
{
    pixels.setPin(WS2812_PIN);
    pixels.updateLength(14);
    pixels.updateType(NEO_GRB + NEO_KHZ400);
    ws2812_data.led_len = 14;
    // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
    // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
#endif
    // END of Trinket-specific code.
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

void ws2812_Server()
{
    if (ws2812_data.state)
    {
        switch (ws2812_data.ws_mode)
        {
        case GRADIENT: //渐变
            /* code */
            break;
        case JUMP: //跳变
            /* code */
            break;
        case BREATHE: //呼吸
            /* code */
            break;
        case BLINK: //闪烁
            /* code */
            break;
        case FLOW: //流水
            /* code */
            break;
        case RAINBORW: //彩虹
            /* code */
            break;

        default:
            break;
        }
    }
}
