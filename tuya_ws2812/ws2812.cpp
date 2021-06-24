/*
 * @Author: hequan 
 * @Date: 2021-06-24 18:42:43 
 * @Last Modified by: hequan
 * @Last Modified time: 2021-06-24 23:49:03
 */
#include "ws2812.h"
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

Adafruit_NeoPixel pixels;
ws2812_T ws2812_data;
Color_T color_data;

static void rainbow(int wait);
static void gradient(Color_T color, ws2812_T data);
static void theaterChase(Color_T color);
static void blink(Color_T color, ws2812_T data);
static void flow(Color_T color, ws2812_T data);

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
        //渐变和呼吸感觉没啥区别。。就写一样算了
        case GRADIENT: //渐变
            gradient(color_data, ws2812_data);
            break;
        case JUMP: //跳变
            theaterChase(color_data);
            break;
        case BREATHE: //呼吸
            gradient(color_data, ws2812_data);
            break;
        case BLINK: //闪烁
            blink(color_data, ws2812_data);
            break;
        case FLOW: //流水
            flow(color_data, ws2812_data);
            break;
        case RAINBORW: //彩虹
            rainbow(color_data.change_time);
            break;

        default:
            break;
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
static void rainbow(int wait)
{
    static unsigned long count;
    static long firstPixelHue;
    // Hue of first pixel runs 3 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
    // means we'll make 3*65536/256 = 768 passes through this outer loop:
    if (firstPixelHue >= 3 * 65536)
        firstPixelHue = 0;
    for (; firstPixelHue < 3 * 65536; firstPixelHue += 256)
    {
        if (millis() - count < wait)
            return;
        for (int i = 0; i < pixels.numPixels(); i++)
        { // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
        }
        pixels.show(); // Update strip with new contents
        count = millis();
    }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
static void theaterChase(Color_T color)
{
    static unsigned long count;
    static int a, b;
    static uint8_t color_count;

    for (; color_count < color.color_len; color_count++)
    {
        if (a >= 10)
            a = 0;
        if (b >= 3)
            b = 0;

        for (; a < 10; a++)
        { // Repeat 10 times...
            for (; b < 3; b++)
            { //  'b' counts from 0 to 2...
                if (millis() - count < color.change_time * 3)
                    return;
                pixels.clear(); //   Set all pixels in RAM to 0 (off)
                // 'c' counts up from 'b' to end of strip in steps of 3...
                for (int c = b; c < pixels.numPixels(); c += 3)
                {
                    pixels.setPixelColor(c, color.color[color_count]); // Set pixel 'c' to value 'color'
                }
                pixels.show(); // Update strip with new contents
                count = millis();
            }
        }
    }
    if (color_count >= color.color_len)
        color_count = 0;
}

static void gradient(Color_T color, ws2812_T data)
{
    static unsigned long count;
    static uint8_t bled, dir, color_count;

    for (; color_count < color.color_len; color_count++)
    {
        pixels.fill(color.color[color_count], 0, data.led_len);
        if (dir == 0)
        {
            for (; bled < 255; bled++)
            {
                if (millis() - count > color.change_time / 5)
                {
                    pixels.setBrightness(bled);
                    pixels.show();
                    count = millis();
                }
                else
                    return;
            }
            if (bled >= 255)
                dir = 1;
        }
        if (dir == 1)
        {
            for (; bled > 0; bled--)
            {
                if (millis() - count > color.change_time)
                {
                    pixels.setBrightness(bled);
                    pixels.show();
                    count = millis();
                }
                else
                    return;
            }
            if (bled <= 0)
                dir = 0;
        }
    }
    if (color_count >= color.color_len)
        color_count = 0;
}

static void blink(Color_T color, ws2812_T data)
{
    static unsigned long count;
    static uint8_t color_count;

    for (; color_count < color.color_len; color_count++)
    {
        if (millis() - count > color.change_time * 10)
        {
            pixels.fill(color.color[color_count], 0, data.led_len);
            pixels.show();
            count = millis();
        }
        else
            return;
    }
    if (color_count >= color.color_len)
        color_count = 0;
}

static void flow(Color_T color, ws2812_T data)
{
    static unsigned long count;
    static uint8_t show_num, color_count;

    for (; color_count < color.color_len; color_count++)
    {
        for (; show_num < data.led_len; show_num++)
        {
            if (millis() - count > color.change_time)
            {
                pixels.setPixelColor(show_num, color.color[color_count]);
                pixels.show();
                count = millis();
            }
            else
                return;
        }
        if (show_num >= data.led_len)
            show_num = 0;
    }
    if (color_count >= color.color_len)
        color_count = 0;
}