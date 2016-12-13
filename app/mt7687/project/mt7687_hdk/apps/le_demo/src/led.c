#include "leconfig.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "flash_map.h"
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
#define LED_ID_BLUE   33
#define LED_ID_GREEN  34
#define LED_ID_BRIGHT 18

typedef struct {
    uint8_t  light;      // on/off led switch
    uint8_t  mode;       // read/white green/white
    uint8_t  timeout;    // led auto off time
    uint8_t  wifimode;   // wifimode
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
    {LED_ID_BLUE, 32, 9, 1, 0, 5120, 1024, 0},
    {LED_ID_GREEN, 33, 9, 1, 0, 5120, 1024, 0},
    {LED_ID_BRIGHT, 35, 9, 1, 0, 5120, 1024, 0},
};

static ledDevice_t ledDevice = {
    0, 0, 0, 0, 1024, 1024, 1024, 1024
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

int leSetConfigMode(uint8_t mode);

static void ledSetVal(int val) {
    hal_pwm_set_duty_cycle(LED_ID_RED, val);
    hal_pwm_set_duty_cycle(LED_ID_GREEN, val);
    hal_pwm_set_duty_cycle(LED_ID_BLUE, val);
}

static void ledRestoreStatus(void) {
    ledDevice.mode = 0;
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
    if(ledEffectDev.size == 1) {
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, ledEffectDev.rgbValue[0].a);
        hal_pwm_set_duty_cycle(LED_ID_RED, ledEffectDev.rgbValue[0].r);
        hal_pwm_set_duty_cycle(LED_ID_GREEN, ledEffectDev.rgbValue[0].g);
        hal_pwm_set_duty_cycle(LED_ID_BLUE, ledEffectDev.rgbValue[0].b);
        goto reloadtimer;
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

reloadtimer:
    if (ledTimerHandler != NULL) {
        xTimerStart(ledTimerHandler, 0);
    }
}

static void leLedStartBlink(void) {
    if (ledTimerHandler != NULL) {
        xTimerStart(ledTimerHandler, 0);
    }
}

static void leLedStopBlink(void) {
    if (ledTimerHandler != NULL) {
        xTimerStop(ledTimerHandler, 0);
    }
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
    APPLOG("==========>leLedBlueFastBlink ledDevice.timeout[%d]", ledDevice.timeout);
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

static void leLedGreenFastBlink(void) { // zigbee join
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 2;
    ledEffectDev.timeout = 30;
    ledEffectDev.rgbValue[0].a = 512;
    ledEffectDev.rgbValue[1].g = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
}

void leLedGreenSlowBlink(void) { // ZigBee permit join
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 200;
    ledEffectDev.size = 2;
    ledEffectDev.timeout = 45;
    ledEffectDev.rgbValue[0].a = 512;
    ledEffectDev.rgbValue[1].g = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
    APPLOG("==========>leLedGreenSlowBlink ledDevice.timeout[%d]", ledDevice.timeout);
}

static void leLedRedWhiteBlink(uint8_t timeout) {
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

static void leLedGreenWhiteBlink(uint8_t timeout) {
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

static void leLedYellow(uint8_t timeout) {
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 1;
    ledEffectDev.timeout = timeout;
    ledEffectDev.rgbValue[0].a = 1024;
    ledEffectDev.rgbValue[0].r = 1024;
    ledEffectDev.rgbValue[0].g = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
}

static void leLedWhite(uint8_t timeout) {
    leLedStopBlink();
    memset(&ledEffectDev, 0, sizeof(ledEffect_t));
    ledEffectDev.light = 1;
    ledEffectDev.mode = 201;
    ledEffectDev.size = 1;
    ledEffectDev.timeout = timeout;
    ledEffectDev.rgbValue[0].a = 1024;
    ledEffectDev.rgbValue[0].r = 1024;
    ledEffectDev.rgbValue[0].g = 1024;
    ledEffectDev.rgbValue[0].b = 1024;
    ledEffectDev.timestamp = xTaskGetTickCount();
    leLedStartBlink();
}

static void leLedRedYellowBlueBlink(void) {
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
    APPLOG("==========>leLedRedYellowBlueBlink ledDevice.timeout[%d]", ledDevice.timeout);
}

void leLedReset(void) {
    ledDevice_t ledOriginData = {0, 0, 0, 0, 1024, 1024, 1024, 1024};
    APPLOG("leLedReset");
    halFlashErase(NULL, CM4_FLASH_USR_CONF_ADDR - GW_FLASH_CONF_SIZE, GW_FLASH_CONF_SIZE);
    halFlashWrite(NULL, (uint8_t *)&ledOriginData, sizeof(ledDevice_t), CM4_FLASH_USR_CONF_ADDR - GW_FLASH_CONF_SIZE, 0);
}

static int leLedWriteConfigData(ledDevice_t* device) {
    int ret;
    halFlashErase(NULL, CM4_FLASH_USR_CONF_ADDR - GW_FLASH_CONF_SIZE, GW_FLASH_CONF_SIZE);
    APPLOG("leLedWriteConfigData light[%d] mode[%d] timeout[%d] wifimode[%d] argb[%d][%d][%d][%d]", 
        device->light, device->mode, device->timeout, device->wifimode, device->brightness, 
        device->color_r, device->color_g, device->color_b);
    ret = halFlashWrite(NULL, (uint8_t *)device, sizeof(ledDevice_t), CM4_FLASH_USR_CONF_ADDR - GW_FLASH_CONF_SIZE, 0);
    return ret;
}

static int leLedReadConfigData(ledDevice_t* device) {
    int ret;
    ret = halFlashRead(NULL, (uint8_t*)device, sizeof(ledDevice_t), CM4_FLASH_USR_CONF_ADDR - GW_FLASH_CONF_SIZE, 0);
    APPLOG("leLedReadConfigData light[%d] mode[%d] timeout[%d] wifimode[%d] argb[%d][%d][%d][%d]", 
        device->light, device->mode, device->timeout, device->wifimode, device->brightness, 
        device->color_r, device->color_g, device->color_b);
    return ret;
}

int haalIsRepeater(void) {
    return leGetConfigMode();
}

int leGetConfigMode(void) {
    return ledDevice.wifimode;
}

int leSetConfigMode(uint8_t mode) {
    ledDevice.wifimode = mode;
    return leLedWriteConfigData(&ledDevice);
}

static int leLedProcessData(ledDevice_t* dev) {
    if(dev->wifimode != ledDevice.wifimode) {
        leSetConfigMode(dev->wifimode);
        reboot(1);
        return 0;
    }
    
    if (dev->light) {
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, dev->brightness);
    } else {
        hal_pwm_set_duty_cycle(LED_ID_BRIGHT, 0);
    }
    
    APPLOG("leLedProcessData light[%d] mode[%d] timeout[%d] wifimode[%d] argb[%d][%d][%d][%d]", 
        dev->light, dev->mode, dev->timeout, dev->timeout, dev->brightness,
        dev->color_r, dev->color_g, dev->color_b);
    if (dev->light == 0) {
        ledDevice.light = 0;
        ledDevice.mode = 0;
        ledDevice.timeout = 0;
        ledNeedRestoreStatus = 0;
        xTimerStop(ledTimerHandler, 0);
        ledSetVal(0);
    } else {
        ledDevice.light = dev->light;
        ledDevice.mode = dev->mode;
        ledDevice.timeout = dev->timeout;
        if (dev->mode > 0) {
            if (dev->mode == 1) {
                leLedRedWhiteBlink(dev->timeout);
            } else if (dev->mode == 2) {
                leLedGreenWhiteBlink(dev->timeout);
            } else if (dev->mode == 3) {
                leLedYellow(dev->timeout);
            } else if (dev->mode == 4) {
                leLedWhite(dev->timeout);
            } else if (dev->mode == 100) {
                leLedRedYellowBlueBlink();
            } else if (dev->mode == 101) {
                leLedGreenFastBlink();
            } else if (dev->mode == 102) {
                leLedGreenSlowBlink(); // ZigBee permit join status
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
            leLedWriteConfigData(dev);
        }
    }
    return 0;
}

void leLedSetDefault(void) {
    ledSetVal(0);
    leLedProcessData(&ledDevice);
}

int leLedRead(uint8_t *data, int* dataLen) {
    data[0] = ledDevice.light;
    data[1] = ledDevice.mode;
    data[2] = ledDevice.timeout;
    data[3] = ledDevice.brightness >> 8;
    data[4] = ledDevice.brightness & 0xFF;
    data[5] = ledDevice.color_r >> 8;
    data[6] = ledDevice.color_r & 0xFF;
    data[7] = ledDevice.color_g >> 8;
    data[8] = ledDevice.color_g & 0xFF;
    data[9] = ledDevice.color_b >> 8;
    data[10] = ledDevice.color_b & 0xFF;
    data[11] = ledDevice.wifimode;
    *dataLen = 12;
    return 12;
}

int leLedWrite(const uint8_t *data, int dataLen) {
    ledDevice_t currLedDevice;
    currLedDevice.light = data[0];
    currLedDevice.mode = data[1];
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
    currLedDevice.wifimode |= data[11];
    leLedProcessData(&currLedDevice);
    return 11;
}

void leLedInit(void) {
    uint32_t i, cycle;
    ledDevice_t device;
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
    leLedReadConfigData(&device);
    if (device.light == 0xFF) {
        leLedReset();
    } else {
        memcpy(&ledDevice, &device, sizeof(ledDevice_t));
    }
}
