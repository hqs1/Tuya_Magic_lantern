/*
 * @Author: hequan 
 * @Date: 2021-06-24 18:42:31 
 * @Last Modified by: hequan
 * @Last Modified time: 2021-06-24 23:46:38
 */
#include <TuyaWifi.h>
#include "dp_config.h"
#include "ws2812.h"

//#define MCU_DEBUG

#ifdef MCU_DEBUG
#include <SoftwareSerial.h>
SoftwareSerial debugSerial(10, 11); // RX, TX
#endif

TuyaWifi my_device;
int count_down_value = 0;
unsigned long time_count;

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
    //倒计时服务
    if (count_down_value != 0 && (millis() - time_count) >= 1000)
    {
        count_down_value--;
        time_count = millis();
        if (count_down_value == 0)
        {
            ws2812_data.state = !ws2812_data.state;
            dp_process(DPID_SWITCH_LED, &ws2812_data.state, 1);
        }
        my_device.mcu_dp_update(DPID_COUNTDOWN, count_down_value, 1);
    }
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

    uint32_t val, hue, sat;

    switch (dpid)
    {
    case DPID_SWITCH_LED:
        count_down_value = 0;
        ws2812_data.state = value[0];
        if (ws2812_data.state == 0)
        {
            pixels.clear();
            pixels.show();
        }
        else //恢复静态下灯状态
        {
            if (ws2812_data.ws_mode == STATIC) //静态直接显示
                ws2812_Static();
        }
        break;
    // case DPID_WORK_MODE:
    //     break;
    case DPID_COUNTDOWN:
        count_down_value = my_device.mcu_get_dp_download_data(dpid, value, length);
        time_count = millis();
        break;
    case DPID_MUSIC_DATA:
    case DPID_CONTROL_DATA:
        hue = __str2short(__asc2hex(value[1]), __asc2hex(value[2]), __asc2hex(value[3]), __asc2hex(value[4]));
        sat = __str2short(__asc2hex(value[5]), __asc2hex(value[6]), __asc2hex(value[7]), __asc2hex(value[8]));
        val = __str2short(__asc2hex(value[9]), __asc2hex(value[10]), __asc2hex(value[11]), __asc2hex(value[12]));
        ws2812_data.ws_mode = STATIC;
        color_data.color_len = 1;
        color_data.color[0] = pixels.gamma32(pixels.ColorHSV(hue * 182, sat / 4, val / 4));
        if (ws2812_data.ws_mode == STATIC) //静态直接显示
            ws2812_Static();
        break;
    case DPID_DREAMLIGHT_SCENE_MODE:
        val = value[8] * 2.55;
        color_data.color_len = (length - 9) / 3;
        color_data.change_time = value[4] / 3;
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
        if (ws2812_data.ws_mode == STATIC) //静态直接显示
            ws2812_Static();
        break;
    case DPID_LIGHTPIXEL_NUMBER_SET:
#ifdef MCU_DEBUG
        debugSerial.print("set_linght_len:");
        debugSerial.println(value[3]);
#endif
        pixels.clear();
        pixels.show();
        ws2812_data.led_len = value[3];
        pixels.updateLength(ws2812_data.led_len);
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
    my_device.mcu_dp_update(DPID_COUNTDOWN, count_down_value, 1);
}
