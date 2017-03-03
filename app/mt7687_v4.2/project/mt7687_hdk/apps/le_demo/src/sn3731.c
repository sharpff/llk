#include "leconfig.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "flash_map.h"
#include "halHeader.h"
#include <errno.h>
#include "timers.h"
#include "debug.h"
#include "hal_gpio.h"
#include "hal_i2c_master.h"
#include "sn3731.h"

/************************************
// SN3731的从地址:
// AdPin Connect to GND 0xE8
// AdPin Connect to VDD 0xEE 
// AdPin Connect to SCL 0xEA
// AdPin Connect to SDA 0xEC
/************************************/
#define	I2C1_SLAVE_ADDRESS  	0xe8
hal_i2c_port_t i2c_port = HAL_I2C_MASTER_0;

#define RGB_INT HAL_GPIO_1
#define RGB_SDA HAL_GPIO_0

#define Reg_MainAddress							0xFD//寄存器主地址
//寄存器地址段选择(定义数据区域为1-8桢的数据)
#define Reg_Section_Fram(i)						((i)-1)//i=1-8
#define Reg_Section_CTRLReg						0x0B	
#define Software_Shutdown_Mode					0x00
#define Normal_Operation_Mode					0x01

//1-144								
#define Reg_PWM(i)       						(0x25+(i)-2) //PWM地址偏移变量
//控制寄存器地址定义
#define Reg_ModeConfig						0x00 	//模式配置寄存器
#define Reg_PictureAddress					0x01	//图片选择寄存器
#define Reg_FramePlay						0x02	//画面自动播放寄存器
#define Reg_FrameTime						0x03	//画面延时寄存器 
#define Reg_0x04H							0x04	//NC
#define Reg_BlinkCtrl							0x05	//闪烁模式寄存器
#define Reg_AudioSync						0x06	//音乐同步寄存器
#define Reg_StatusReg						0x07	//画面状态寄存器(只读)
#define Reg_BreathTime						0x08	//呼吸时间设置寄存器
#define Reg_Breath							0x09	//呼吸控制寄存器
#define Reg_ShutDown						0x0A	//关断寄存器
#define Reg_AGC								0x0B	//AGC控制寄存器
#define Reg_AudioADCRate					0x0C	//音频采样寄存器

#define SN_REG_SWITCH_BASE 0x00
#define SN_REG_BLINK_BASE  0x12
#define SN_REG_PWM_BASE    0x24


#define LED_MAX_SIZE       14

typedef struct _point {
    uint8_t col;
    uint8_t row;
} point_t;

// led901,led902,led903,led904,led908,led909,led910,led911,led912,led913,led906,led907,led905,led914
point_t arrays_r[LED_MAX_SIZE] = {{9,11},{9,12},{9,13},{9,14},{1,11},{1,12},{1,13},{1,14},{1,15},{1,16},{4,10},{4,11},{9,9},{9,10}};
point_t arrays_g[LED_MAX_SIZE] = {{8,11},{8,12},{8,13},{8,14},{2,11},{2,12},{2,13},{2,14},{2,15},{2,16},{5,10},{5,11},{8,9},{8,10}};
point_t arrays_b[LED_MAX_SIZE] = {{7,11},{7,12},{7,13},{7,14},{3,11},{3,12},{3,13},{3,14},{3,15},{3,16},{6,10},{6,11},{7,9},{7,10}};

void i2c_init() {
    hal_i2c_config_t i2c_init;
    hal_gpio_init(HAL_GPIO_27);
    hal_gpio_init(HAL_GPIO_28);
    hal_pinmux_set_function(HAL_GPIO_27,HAL_GPIO_27_I2C1_CLK);
    hal_pinmux_set_function(HAL_GPIO_28,HAL_GPIO_28_I2C1_DATA);
    i2c_init.frequency = HAL_I2C_FREQUENCY_400K;
    hal_i2c_master_init(i2c_port,&i2c_init);
}

void i2c_deinit() {
    hal_i2c_master_deinit(i2c_port);
    hal_gpio_deinit(HAL_GPIO_27);
    hal_gpio_deinit(HAL_GPIO_28);
}

void i2c_send(uint8_t *data, int length) {
    hal_i2c_master_send_polling(i2c_port,I2C1_SLAVE_ADDRESS,data,length);
    hal_gpt_delay_ms(200);
}

void i2c_receive(uint8_t *data, int length) {
    hal_i2c_master_receive_polling(i2c_port,I2C1_SLAVE_ADDRESS,data,length - 1);
    hal_gpt_delay_ms(200);
}

void i2c_write_reg(uint8_t reg, uint8_t data) {
    uint8_t datas[2];
    datas[0] = reg;
    datas[1] = data;
    i2c_send(datas, 2);      
}

void i2c_write_regs(uint8_t reg, uint8_t* data, int length) {
    i2c_send(&reg, 1);
    i2c_send(data, length);      
}

void i2c_select_section(uint8_t address, uint8_t reg) {
    i2c_write_reg(address, reg);
}

void sn3731_init()
{
    unsigned char i;
    i2c_write_reg(0xfd,0x0b);
    i2c_write_reg(0x00,0x00);

    i2c_write_reg(0x01,0x00);
    //i2c_write_reg(0x02,0x00);

    i2c_write_reg(0x03,0x00);
    i2c_write_reg(0x04,0x00);
    i2c_write_reg(0x05,0x00);
    i2c_write_reg(0x06,0x00);
    i2c_write_reg(0x07,0x00);
    i2c_write_reg(0x08,0x00);
    i2c_write_reg(0x09,0x00);
    //i2c_write_reg(0x0a,0x00);
    i2c_write_reg(0x0b,0x00);
    i2c_write_reg(0x0c,0x00);

    for(i=1;i<9;i++) { //clear frame data
        i2c_select_section(Reg_MainAddress,Reg_Section_Fram(i));
        i2c_write_regs(0x00,all_led_off,18);
        i2c_write_regs(0x12, all_led_off, 18); // blink off
        i2c_write_regs(0x24, all_pwm_off, 144);				
    }
    
    i2c_select_section(Reg_MainAddress,Reg_Section_CTRLReg);//select main addr
    i2c_write_reg(Reg_ShutDown,Normal_Operation_Mode);//disable the software shutdown
    i2c_select_section(Reg_MainAddress,Reg_Section_Fram(1));//turn to frame 1
}

void sn3731_led_on_all(uint8_t i) { 
	i2c_select_section(Reg_MainAddress,Reg_Section_Fram(i));
	i2c_write_regs(0x00,all_led_on,18);
	i2c_write_regs(0x24,all_pwm_on,144);	
}

void sn3731_led_off_all(uint8_t i) {
	i2c_select_section(Reg_MainAddress,Reg_Section_Fram(i));
	i2c_write_regs(0x00,all_led_off,18);
	i2c_write_regs(0x24, all_pwm_off, 144);	
}

void sn3731_delete_frame_pwm(uint8_t i) {
	i2c_select_section(Reg_MainAddress,Reg_Section_Fram(i));
	i2c_write_regs(0x24,all_pwm_off,144);
}

void sn3731_delete_frame_onoff(uint8_t i) {
	i2c_select_section(Reg_MainAddress,Reg_Section_Fram(i));
	i2c_write_regs(0x00,all_led_off,18);
}

void sn3731_pwm_inmatrix(uint8_t row, uint8_t col, uint8_t status, uint8_t brightness) {
	int i,j;
	if  ( (row < 1 ) || (row> 9) ) return;
	if  ( (col < 1 ) || (col> 16) ) return;

	if(status) {
		if (col <= 8 ) {
			g_dram[row] |= (0x0001<<(col-1));
			i = g_dram[row];
			j = i;
  			i2c_write_reg(SN_REG_SWITCH_BASE+(row-1)*2,j);
		} else {
			g_dram[row+1] |= (0x0001<<(col-1));
			i = g_dram[row+1];
			j = (i >> 8);
 			i2c_write_reg(SN_REG_SWITCH_BASE+(row-1)*2+1,j);
		}
	} else {
		if (col <= 8 ) {
			g_dram[row]&= (~(0x0001<<(col-1)));
			i = g_dram[row];
			j = i;
  			i2c_write_reg(SN_REG_SWITCH_BASE+(row-1)*2,j);
		} else {
			g_dram[row+1]&= (~(0x0001<<(col-1)));
			i = g_dram[row+1];
			j = (i >> 8); 
 			i2c_write_reg(SN_REG_SWITCH_BASE+(row-1)*2+1,j);
		}
		
	}
	i2c_write_reg(SN_REG_PWM_BASE+(row-1)*16+col-1,brightness);
}

void sn3731_led_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    int i;
    for(i=0; i<LED_MAX_SIZE; i++) {
         sn3731_pwm_inmatrix(arrays_r[i].row, arrays_r[i].col, 1, red);
         sn3731_pwm_inmatrix(arrays_r[i].row, arrays_r[i].col, 1, green);
         sn3731_pwm_inmatrix(arrays_r[i].row, arrays_r[i].col, 1, blue);
    }
}
