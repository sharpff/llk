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

// 0x8001/0x8002 led short status
// 0x8101 App switch
// 0x8201 key click close
// 0x7001 key click open

typedef struct {
    uint8_t  light;      // on/off led switch
    uint8_t  mode;       // 1-slow blink, 2-fast blink, 3-user difine (status)
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
    {33, 32, 9, 1, 0, 5120, 1024, 0},
    {34, 33, 9, 1, 0, 5120, 1024, 0},
    {35, 34, 9, 1, 0, 5120, 1024, 0},
    {18, 35, 9, 1, 0, 5120, 1024, 0},
};

static ledDevice_t ledDevice = {
    0, 0, 0, 0, 0, 0, 0
};

static ledDevice_t ledBlinkDevice = {
    0, 0, 0, 0, 0, 0, 0
};

typedef struct {
    uint32_t a;
    uint32_t r;
    uint32_t g;
    uint32_t b;
}ledEffect_t;

static ledEffect_t ledEffectArray[LED_MAX_SIZE] = {0};
static uint8_t ledEffectSize = 0;

#define RED    0
#define BLUE   1
#define GREEN  2
#define BRIGHT 3

#define LED_RED   1
#define LED_BLUE  2
#define LED_GREEN 4

static TimerHandle_t ledTimerHandler = NULL;

static void ledSetVal(int val) {
    hal_pwm_set_duty_cycle(ledPWMArray[RED].id, val);
    hal_pwm_set_duty_cycle(ledPWMArray[GREEN].id, val);
    hal_pwm_set_duty_cycle(ledPWMArray[BLUE].id, val);
}

static void ledBlink(int timeout) {
    ledPWM_t* pwdPtr = NULL;
    if (ledBlinkDevice.color_r) {
        pwdPtr = &ledPWMArray[RED];
        pwdPtr->count++;
        if (pwdPtr->count%(timeout*2) == 0) {
            hal_pwm_set_duty_cycle(pwdPtr->id, ledDevice.color_r);
        } else if (pwdPtr->count%timeout == 0) {
            hal_pwm_set_duty_cycle(pwdPtr->id, 0);
        }
    }
    if (ledBlinkDevice.color_g) {
        pwdPtr = &ledPWMArray[GREEN];
        pwdPtr->count++;
        if (pwdPtr->count%(timeout*2) == 0) {
            hal_pwm_set_duty_cycle(pwdPtr->id, ledDevice.color_g);
        } else if (pwdPtr->count%timeout == 0) {
            hal_pwm_set_duty_cycle(pwdPtr->id, 0);
        }
    }
    if (ledBlinkDevice.color_b) {
        pwdPtr = &ledPWMArray[BLUE];
        pwdPtr->count++;
        if (pwdPtr->count%(timeout*2) == 0) {
            hal_pwm_set_duty_cycle(pwdPtr->id, ledDevice.color_b);
        } else if (pwdPtr->count%timeout == 0) {
            hal_pwm_set_duty_cycle(pwdPtr->id, 0);
        }
    }
}

static void ledBreathing(void) {
    int j;
    for (j = 0; ledDevice.light && j < ledDevice.color_r ; j+=2) {
        ledSetVal(j);
        halDelayms(5);
    }
    for (j = ledDevice.color_r; ledDevice.light && j > 0; j-=2) {
        ledSetVal(j);
        halDelayms(5);
    }
}

static void ledBlinkTimerCallback( TimerHandle_t tmr ) {
    if(ledDevice.light == 0) {
        hal_pwm_set_duty_cycle(ledPWMArray[BRIGHT].id, 0);
        return;
    }
    if (ledDevice.mode == 1) {
        ledBlink(2);
    } else if (ledDevice.mode == 2) {
        ledBlink(6);
    } else if (ledDevice.mode == 3) {
        ledBreathing();
    } 
    if (ledTimerHandler != NULL) {
        xTimerStart(ledTimerHandler, 0);
    }
}

int leLedProcessData(ledDevice_t* dev) {
    setDevFlag(DEV_FLAG_MODE, 1);
        halReboot();
    hal_pwm_set_duty_cycle(ledPWMArray[BRIGHT].id, dev->light);
    APPLOG("leLedProcessData light[%d] mode[%d] timeout[%d] argb[%d][%d][%d][%d]", 
        dev->light, dev->mode, dev->timeout, dev->brightness, 
        dev->color_r, dev->color_g, dev->color_b);
    if (dev->light == 0) {
        ledDevice.light = 0;
        xTimerStop(ledTimerHandler, 0);
        ledSetVal(0);
    } else {
        if (dev->mode > 0) {
            ledDevice.mode = dev->mode;
            switch (dev->mode) {
                case 200:
                case 201:
                hal_pwm_set_duty_cycle(ledPWMArray[RED].id, dev->color_r);
                hal_pwm_set_duty_cycle(ledPWMArray[GREEN].id, dev->color_g);
                hal_pwm_set_duty_cycle(ledPWMArray[BLUE].id, dev->color_b);
                memcpy(&ledBlinkDevice, dev, sizeof(ledDevice_t));
                if (ledTimerHandler != NULL) {
                    xTimerStart(ledTimerHandler, 0);
                }
                break;
                case 3:
                memcpy(&ledDevice, dev, sizeof(ledDevice_t));
                if (ledTimerHandler != NULL) {
                    xTimerStart(ledTimerHandler, 0);
                }
                break;
                default:
                break;
            }
        } else {
            // save rgb value and switch
            memcpy(&ledDevice, dev, sizeof(ledDevice_t));
            xTimerStop(ledTimerHandler, 0);
            hal_pwm_set_duty_cycle(ledPWMArray[RED].id, dev->color_r);
            hal_pwm_set_duty_cycle(ledPWMArray[GREEN].id, dev->color_g);
            hal_pwm_set_duty_cycle(ledPWMArray[BLUE].id, dev->color_b);
        }
    }
    return 0;
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
    *dataLen = 11;
    return 11;
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
    leLedProcessData(&currLedDevice);
    return 11;
}

void leLedBlueFastBlink(void) { // wifi connect
    uint8_t cmd[11] = {1, 1, 0, 4, 0, 0, 0, 0, 0, 4, 0};
    leLedWrite(cmd, 11);
}

void leLedBlueSlowBlink(void) { // wifi configure
    uint8_t cmd[11] = {1, 2, 0, 4, 0, 0, 0, 0, 0, 4, 0};
    leLedWrite(cmd, 11);
}

void leLedGreenFastBlink(void) { // zigbee join
    uint8_t cmd[11] = {1, 1, 30, 4, 0, 0, 0, 4, 0, 0, 0};
    leLedWrite(cmd, 11);
}

void leLedRedWhiteBlink(void) {
    uint8_t cmd[11] = {1, 2, 0, 4, 0, 0, 0, 4, 0, 0, 0};
    leLedWrite(cmd, 11);
}

void leLedGreenWhiteBlink(void) {
    uint8_t cmd[11] = {1, 1, 0, 4, 0, 4, 0, 0, 0, 0, 0};
    leLedWrite(cmd, 11);
}

void leLedRedYellowBlueBlink(void) {
    uint8_t cmd[11] = {1, 1, 0, 4, 0, 4, 0, 0, 0, 0, 0};
    leLedWrite(cmd, 11);
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

