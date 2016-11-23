#include "leconfig.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "halHeader.h"
#include <errno.h>
#include "timers.h"
#include "debug.h"
#include "hal_pwm.h"
#include "hal_gpio.h"
#include "io.h"

#define LED_BLINK_TIMEOUT (500/portTICK_PERIOD_MS)
#define LED_MAX_SIZE 4

#define LED_ID_RED    35
#define LED_ID_BLUE   34
#define LED_ID_GREEN  33
#define LED_ID_BRIGHT 18

typedef struct {
    uint8_t  light;      // on/off led switch
    uint8_t  special;    // read/white green/white
    uint8_t  timeout;    // led auto off time
    uint16_t brightness; // 100-1024
    uint16_t color_r;    // 0-1024
    uint16_t color_g;    // 0-1024
    uint16_t color_b;    // 0-1024
}ledDevice_t;

typedef struct {
    uint8_t id;
    uint8_t gid;
    uint8_t mux;
    uint8_t clock;
    uint32_t state;
    uint32_t frequency;
    uint32_t duty;
    uint32_t count;
}ledPWM_t;

ledPWM_t ledPWMArray[LED_MAX_SIZE] = {
    {LED_ID_RED, 34, 9, 1, 0, 5120, 1024, 0},
    {LED_ID_BLUE, 33, 9, 1, 0, 5120, 1024, 0},
    {LED_ID_GREEN, 32, 9, 1, 0, 5120, 1024, 0},
    {LED_ID_BRIGHT, 35, 9, 1, 0, 5120, 1024, 0},
};

static ledDevice_t ledDevice = {
    0, 0, 0, 1024, 1024, 1024, 1024
};

typedef struct {
    uint16_t a;
    uint16_t r;
    uint16_t g;
    uint16_t b;
}ledRGBValue_t;

typedef struct {
    uint8_t light;
    uint8_t mode;
    uint8_t size;
    uint8_t oldlight;
    uint32_t curr;
    uint32_t timeout;
    uint32_t timestamp;
    uint32_t count;
    ledRGBValue_t rgbValue[LED_MAX_SIZE];
}ledEffect_t;

static ledEffect_t ledEffectDev;

static TimerHandle_t ledTimerHandler = NULL;
static uint8_t ledNeedRestoreStatus = 0;

static void ledSetVal(int val) {
    hal_pwm_set_duty_cycle(LED_ID_RED, val);
    hal_pwm_set_duty_cycle(LED_ID_GREEN, val);
    hal_pwm_set_duty_cycle(LED_ID_BLUE, val);
}

void ledRestoreStatus(void) {
    ledDevice.special = 0;
    if(ledNeedRestoreStatus) {
        ledDevice.light = 1;
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, ledDevice.brightness);
        hal_pwm_set_duty_cycle(LED_ID_RED, ledDevice.color_r);
        hal_pwm_set_duty_cycle(LED_ID_GREEN, ledDevice.color_g);
        hal_pwm_set_duty_cycle(LED_ID_BLUE, ledDevice.color_b);
    } else {
        ledDevice.light = 0;
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, 0);
        hal_pwm_set_duty_cycle(LED_ID_RED, 0);
        hal_pwm_set_duty_cycle(LED_ID_GREEN, 0);
        hal_pwm_set_duty_cycle(LED_ID_BLUE, 0);
    }
}

static void ledBlinkTimerCallback( TimerHandle_t tmr ) {
    int timeout = 2;
    if(ledEffectDev.light == 0) {
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, 0);
        return;
    }
    if (ledEffectDev.size == 0) {
        return;
    }
    if (ledEffectDev.timeout) {
        //APPLOG("ledBlinkTimerCallback timeout[%d][%d][%d]", xTaskGetTickCount(), ledEffectDev.timestamp, ledEffectDev.timeout);
        if(xTaskGetTickCount() - ledEffectDev.timestamp > ledEffectDev.timeout*1000/portTICK_PERIOD_MS) {
            APPLOG("ledBlinkTimerCallback timeout, restore former status");
            ledRestoreStatus();
            return;
        }
    }
    if (ledEffectDev.mode == 200) {
        timeout = 4;
    }
    if (ledEffectDev.count%timeout == 0) {
        int index = ledEffectDev.curr%ledEffectDev.size;
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, ledEffectDev.rgbValue[index].a);
        hal_pwm_set_duty_cycle(LED_ID_RED, ledEffectDev.rgbValue[index].r);
        hal_pwm_set_duty_cycle(LED_ID_GREEN, ledEffectDev.rgbValue[index].g);
        hal_pwm_set_duty_cycle(LED_ID_BLUE, ledEffectDev.rgbValue[index].b);
        ledEffectDev.curr++;
    }
    ledEffectDev.count++;
    if (ledTimerHandler != NULL) {
        xTimerStart(ledTimerHandler, 0);
    }
}

void leLedStartBlink(void) {
    if (ledTimerHandler != NULL) {
        xTimerStart(ledTimerHandler, 0);
    }
}

void leLedStopBlink(void) {
    if (ledTimerHandler != NULL) {
        xTimerStop(ledTimerHandler, 0);
    }
}

int leLedProcessData(ledDevice_t* dev) {
    hal_pwm_set_duty_cycle(LED_ID_BRIGHT, dev->light);
    APPLOG("leLedProcessData light[%d] special[%d] timeout[%d] argb[%d][%d][%d][%d]", 
        dev->light, dev->special, dev->timeout, dev->brightness, 
        dev->color_r, dev->color_g, dev->color_b);
    if (dev->light == 0) {
        ledDevice.light = 0;
        ledDevice.special = 0;
        ledDevice.timeout = 0;
        ledNeedRestoreStatus = 0;
        xTimerStop(ledTimerHandler, 0);
        ledSetVal(0);
    } else {
        ledDevice.light = dev->light;
        ledDevice.special = dev->special;
        ledDevice.timeout = dev->timeout;
        if (dev->special > 0) {
            if (dev->special == 1) {
                leLedRedWhiteBlink(dev->timeout);
            } else if (dev->special == 2) {
                leLedGreenWhiteBlink(dev->timeout);
            } else if (dev->special == 100) {
                leLedRedYellowBlueBlink();
            } else if (dev->special == 101) {
                leLedGreenFastBlink();
            } else {
                // do nothing
            }
        } else {
            // save rgb value and switch
            ledNeedRestoreStatus = 1;
            memcpy(&ledDevice, dev, sizeof(ledDevice_t));
            xTimerStop(ledTimerHandler, 0);
            hal_pwm_set_duty_cycle(LED_ID_RED, dev->color_r);
            hal_pwm_set_duty_cycle(LED_ID_GREEN, dev->color_g);
            hal_pwm_set_duty_cycle(LED_ID_BLUE, dev->color_b);
        }
    }
    return 0;
}

int leLedRead(uint8_t *data, int* dataLen) {
    data[0] = ledDevice.light;
    data[1] = ledDevice.special;
    data[2] = ledDevice.timeout;
    data[3] = ledDevice.brightness >> 8;
    data[4] = ledDevice.brightness & 0xFF;
    data[5] = ledDevice.color_r >> 8;
    data[6] = ledDevice.color_r & 0xFF;
    data[7] = ledDevice.color_g >> 8;
    data[8] = ledDevice.color_g & 0xFF;
    data[9] = ledDevice.color_b >> 8;
    data[10] = ledDevice.color_b & 0xFF;
    *dataLen = 11;
    return 11;
}

int leLedWrite(const uint8_t *data, int dataLen) {
    ledDevice_t currLedDevice;
    currLedDevice.light = data[0];
    currLedDevice.special = data[1];
    currLedDevice.timeout = data[2];
    currLedDevice.brightness = data[3];
    currLedDevice.brightness <<= 8;
    currLedDevice.brightness |= data[4];
    currLedDevice.color_r = data[5];
    currLedDevice.color_r <<= 8;
    currLedDevice.color_r |= data[6];
    currLedDevice.color_g = data[7];
    currLedDevice.color_g <<= 8;
    currLedDevice.color_g |= data[8];
    currLedDevice.color_b = data[9];
    currLedDevice.color_b <<= 8;
    currLedDevice.color_b |= data[10];
    leLedProcessData(&currLedDevice);
    return 11;
}

void leLedBlueFastBlink(void) { // wifi connect
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 2;
    ledEffectDev.rgbValue[0].a = 512;
    ledEffectDev.rgbValue[0].b = 1024;
    leLedStartBlink();
}

void leLedBlueSlowBlink(void) { // wifi configure
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 200;
    ledEffectDev.size = 2;
    ledEffectDev.rgbValue[0].a = 512;
    ledEffectDev.rgbValue[0].b = 1024;
    leLedStartBlink();
}

void leLedGreenFastBlink(void) { // zigbee join
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 2;
    ledEffectDev.timeout = 30;
    ledEffectDev.rgbValue[0].a = 512;
    ledEffectDev.rgbValue[1].b = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
}

void leLedRedWhiteBlink(uint8_t timeout) {
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 2;
    ledEffectDev.timeout = timeout;
    ledEffectDev.rgbValue[0].a = 1024;
    ledEffectDev.rgbValue[0].r = 1024;
    ledEffectDev.rgbValue[1].a = 1024;
    ledEffectDev.rgbValue[1].r = 1024;
    ledEffectDev.rgbValue[1].g = 1024;
    ledEffectDev.rgbValue[1].b = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
}

void leLedGreenWhiteBlink(uint8_t timeout) {
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 2;
    ledEffectDev.timeout = timeout;
    ledEffectDev.rgbValue[0].a = 1024;
    ledEffectDev.rgbValue[0].g = 1024;
    ledEffectDev.rgbValue[1].a = 1024;
    ledEffectDev.rgbValue[1].r = 1024;
    ledEffectDev.rgbValue[1].g = 1024;
    ledEffectDev.rgbValue[1].b = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
}

void leLedRedYellowBlueBlink(void) {
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 3;
    ledEffectDev.rgbValue[0].a = 1024;
    ledEffectDev.rgbValue[0].r = 1024;
    ledEffectDev.rgbValue[1].a = 1024;
    ledEffectDev.rgbValue[1].r = 1024;
    ledEffectDev.rgbValue[1].g = 1024;
    ledEffectDev.rgbValue[2].a = 1024;
    ledEffectDev.rgbValue[2].b = 1024;
    leLedStartBlink();
}

void leLedAllOff(void) {
    uint8_t cmd[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    leLedWrite(cmd, 11);
}

void leLedInit(void) {
    int i, cycle;
    for(i=0; i< LED_MAX_SIZE; i++) {
        hal_gpio_init(ledPWMArray[i].gid);
        hal_pinmux_set_function(ledPWMArray[i].gid, ledPWMArray[i].mux);
        hal_gpio_deinit(ledPWMArray[i].gid);
    }
    hal_pwm_init(1); // 2M clock
    for(i=0; i< LED_MAX_SIZE; i++) {
        hal_pwm_set_frequency(ledPWMArray[i].id, ledPWMArray[i].frequency, &cycle);
        hal_pwm_start(ledPWMArray[i].id);
        hal_pwm_set_duty_cycle(ledPWMArray[i].id, 0);
    }
    if (ledTimerHandler == NULL) {
        ledTimerHandler = xTimerCreate("led_blink_timer",
                                       LED_BLINK_TIMEOUT,
                                       pdFALSE,
                                       NULL,
                                       ledBlinkTimerCallback);        
    }
}

