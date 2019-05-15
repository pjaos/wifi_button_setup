#ifndef _AD111X
#define _AD111X


/**
 * @brief Driver for ADS1113, ADS1114 and ADS1115 ADC device
 * 			using mongoose os
 **/

#define ADS115X_ADDR_PIN_LOW_SLAVE_ADDR  	0b01001000 // 0x48
#define ADS115X_ADDR_PIN_HIGH_SLAVE_ADDR 	0b01001001 // 0x49
#define ADS115X_ADDR_PIN_SDA_SLAVE_ADDR  	0b01001010 // 0x50
#define ADS115X_ADDR_PIN_SCL_SLAVE_ADDR 	0b01001011 // 0x51

#define OS_CFG_REG_BIT          	15
#define MUX_CFG_REG_BIT				12
#define PGA_CFG_REG_BIT				9
#define	MODE_CFG_REG_BIT			8
#define DR_CFG_REG_BIT				5
#define	COMP_MODE_CFG_REG_BIT		4
#define	COMP_POL_CFG_REG_BIT    	3
#define COMP_LAT_CFG_REG_BIT		2
#define	COMP_QUE_CFG_REG_BIT		0

#define CONVERSION_REG				0
#define CONFIG_REG					1
#define	LO_THREH_REG				2
#define	HI_THREH_REG				3

//Define the default state of all config register parts
#define DEFAULT_OS			0b0
#define DEFAULT_MUX 		0b000
#define DEFAULT_PGA 		0b010
#define DEFAULT_MODE		0b1
#define DEFAULT_DR			0b100
#define DEFAULT_COMPMODE	0b0
#define DEFAULT_COMPPOL		0b0
#define DEFAULT_COMPLAT		0b0
#define DEFAULT_COMPQUE		0b11

enum {
	FS_VOLTAGE_6_144,
	FS_VOLTAGE_4_096,
	FS_VOLTAGE_2_048,
	FS_VOLTAGE_1_024,
	FS_VOLTAGE_0_512,
	FS_VOLTAGE_0_256
};

enum {
	SAMPLES_PER_SECOND_8,
	SAMPLES_PER_SECOND_16,
	SAMPLES_PER_SECOND_32,
	SAMPLES_PER_SECOND_64,
	SAMPLES_PER_SECOND_128,
	SAMPLES_PER_SECOND_250,
	SAMPLES_PER_SECOND_475,
	SAMPLES_PER_SECOND_860
};

enum {
	ADC0,
	ADC1,
	ADC2,
	ADC3
};

#define FS_VOLTAGE_ERROR 	(int16_t)0x00010000
#define MUX_ERROR			FS_VOLTAGE_ERROR+1
#define SPS_ERROR			MUX_ERROR+1


uint16_t get_adc_value(uint16_t device_address, uint8_t adc, uint8_t fs_voltage_id, uint8_t samples_per_second, bool single_ended);

#endif
