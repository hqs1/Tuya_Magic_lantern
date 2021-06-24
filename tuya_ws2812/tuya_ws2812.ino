/*
 * @Author: hequan 
 * @Date: 2021-06-24 18:42:31 
 * @Last Modified by:   hequan 
 * @Last Modified time: 2021-06-24 18:42:31 
 */
#include <TuyaWifi.h>
#include "dp_config.h"
#include "ws2812.h"

// #define MCU_DEBUG

#ifdef MCU_DEBUG
#include <SoftwareSerial.h>
SoftwareSerial debugSerial(10, 11); // RX, TX
#endif

TuyaWifi my_device;

unsigned char dp_bool_value = 0;
long dp_value_value = 0;
unsigned char dp_enum_value = 0;
unsigned char dp_string_value[8] = {"Hi,Tuya"};
unsigned char dp_raw_value[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
int dp_fault_value = 0x01;

unsigned char dp_array[][2] = {
    {DPID_SWITCH_LED, DP_TYPE_BOOL},
    {DPID_WORK_MODE, DP_TYPE_ENUM},
    {DPID_COUNTDOWN, DP_TYPE_VALUE},
    {DPID_MUSIC_DATA, DP_TYPE_STRING},
    {DPID_CONTROL_DATA, DP_TYPE_STRING},
    {DPID_DREAMLIGHT_SCENE_MODE, DP_TYPE_RAW},
    {DPID_LIGHTPIXEL_NUMBER_SET, DP_TYPE_VALUE},
};

void setup()
{
#ifdef MCU_DEBUG
    debugSerial.begin(9600);
#endif
    ws2812_Init();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    my_device.init(pid, mcu_ver);
    my_device.set_dp_cmd_total(dp_array, 6);
    my_device.dp_process_func_register(dp_process);
    my_device.dp_update_all_func_register(dp_update_all);
}

void loop()
{
    ws2812_Server();
    my_device.uart_service();
}

/**
 * @description: DP download callback function.
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} length
 * @return {unsigned char}
 */
unsigned char dp_process(unsigned char dpid, const unsigned char value[], unsigned short length)
{

    uint16_t val, hue, sat;
    switch (dpid)
    {
    case DPID_SWITCH_LED:
        digitalWrite(LED_BUILTIN, value[0]);
        ws2812_data.state = value[0];
        if (ws2812_data.state == 0)
        {
            pixels.clear();
            pixels.show();
        }
        break;
    // case DPID_WORK_MODE:
    //     break;
    case DPID_COUNTDOWN:
        break;
    case DPID_MUSIC_DATA:
        break;
    case DPID_CONTROL_DATA:
        break;
    case DPID_DREAMLIGHT_SCENE_MODE:
        val = value[8] * 2.55;
        color_data.color_len = (length - 9) / 3;
        ws2812_data.ws_mode = (wsMode)value[2];

        for (uint8_t i = 0; i < color_data.color_len; i++)
        {
            //HSV - > RGB
            hue = value[9 + i * 3];
            hue <<= 8;
            hue += value[10 + i * 3];
            sat = value[11 + i * 3] * 2.55;
            color_data.color[i] = pixels.gamma32(pixels.ColorHSV(hue * 182, sat, val));
#ifdef MCU_DEBUG
            debugSerial.print("==========");
            debugSerial.println(i);
            debugSerial.print("hue_RAW:");
            debugSerial.print(value[9 + i * 3], HEX);
            debugSerial.print(" ");
            debugSerial.println(value[10 + i * 3], HEX);
            debugSerial.print("val:");
            debugSerial.println(val);
            debugSerial.print("hue:");
            debugSerial.println(hue);
            debugSerial.print("sat:");
            debugSerial.println(sat);
#endif
        }
#ifdef MCU_DEBUG
        debugSerial.print("color_len:");
        debugSerial.println(color_data.color_len);
#endif
        uint8_t odd_value, gop_num, show_num;
        if (ws2812_data.ws_mode == STATIC) //静态直接显示
        {
            if (color_data.color_len == 1)
            {
                pixels.fill(color_data.color[0], 0, ws2812_data.led_len);
            }
            else
            {
                odd_value = ws2812_data.led_len % color_data.color_len;
                if (odd_value == ws2812_data.led_len)
                {
                    for (uint8_t i = 0; i < ws2812_data.led_len; i++)
                    {
                        pixels.setPixelColor(i, color_data.color[i]);
                    }
                }
                else
                {
                    if (odd_value != 0)
                    {
                        show_num = ws2812_data.led_len - odd_value;
                    }
                    gop_num = show_num / color_data.color_len;
                    for (int i = 0; i < color_data.color_len; i++)
                    {
                        for (int j = 0; j < gop_num; j++)
                        {
                            pixels.setPixelColor(i * gop_num + j, color_data.color[i]);
                        }
                    }
                }
            }
            pixels.show();
        }
        break;
    case DPID_LIGHTPIXEL_NUMBER_SET:
        pixels.updateLength(value[0]);
        ws2812_data.led_len = value[0];
        break;
    default:
        break;
    }
    my_device.mcu_dp_update(dpid, value, length);
    return SUCCESS;
}

/**
 * @description: Upload all DP status of the current device.
 * @param {*}
 * @return {*}
 */
void dp_update_all(void)
{
    my_device.mcu_dp_update(DPID_SWITCH_LED, ws2812_data.state, 1);
    my_device.mcu_dp_update(DPID_LIGHTPIXEL_NUMBER_SET, ws2812_data.led_len, 1);
}
