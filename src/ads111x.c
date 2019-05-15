/*
 *  Created on: 2 Mar 2018
 *      Author: pja
 *  Responsible for providing an interface to ADS1113/4/5 16 Bit I2C ADC devices.
 */
#include "mgos.h"
#include "mgos_i2c.h"

#include "ads111x.h"

/**
 * @brief Get the PGA value for the ADS111x device.
 * @param fs_voltage_id The full scale voltage. Any of the following
 *        values are valid.
 * 			FS_VOLTAGE_6_144
 *			FS_VOLTAGE_4_096
 *			FS_VOLTAGE_2_048
 *			FS_VOLTAGE_1_024
 *			FS_VOLTAGE_0_512
 *			FS_VOLTAGE_0_256
 **/
static int8_t get_pga_value(uint8_t fs_voltage_id) {
        int return_val = -1;

        if( fs_voltage_id == FS_VOLTAGE_6_144 ) {
            return_val = 0b000;
        }
        else if( fs_voltage_id == FS_VOLTAGE_4_096 ) {
            return_val = 0b001;
        }
        else if( fs_voltage_id == FS_VOLTAGE_2_048 ) {
            return_val = 0b010;
        }
        else if( fs_voltage_id == FS_VOLTAGE_1_024 ) {
            return_val = 0b011;
        }
        else if( fs_voltage_id == FS_VOLTAGE_0_512 ) {
            return_val = 0b100;
        }
        else if( fs_voltage_id == FS_VOLTAGE_0_256 ) {
            return_val = 0b101;
        }

        return return_val;
}

/**
 * @brief Get the data rate value.
 * @param samples_per_second Samples per second value.
 * 	SAMPLES_PER_SECOND_8,
 *	SAMPLES_PER_SECOND_16,
 *	SAMPLES_PER_SECOND_32,
 *	SAMPLES_PER_SECOND_64,
 *	SAMPLES_PER_SECOND_128,
 *	SAMPLES_PER_SECOND_250,
 *	SAMPLES_PER_SECOND_475,
 *	SAMPLES_PER_SECOND_860
 * @return The data rate value or -1 if samples_per_second is invalid.
 **/
static int8_t get_data_rate_value(uint8_t samples_per_second) {
	int data_rate_value = -1;

	if( samples_per_second == SAMPLES_PER_SECOND_8 ) {
		data_rate_value = 0b000;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_16 ) {
		data_rate_value = 0b001;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_32 ) {
		data_rate_value = 0b010;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_64 ) {
		data_rate_value = 0b011;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_128 ) {
		data_rate_value = 0b100;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_250 ) {
		data_rate_value = 0b101;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_475 ) {
		data_rate_value = 0b110;
	}
	else if ( samples_per_second == SAMPLES_PER_SECOND_860 ) {
		data_rate_value = 0b111;
	}

	return data_rate_value;
}

/**
 * @brief Get the mux value for the ADS111x device.
 * @param adc The ADC to be selected (0,1,2 & 3 are valid for ADS115).
 *            Valid options are
 * 			  ADC0
 *            ADC1
 *            ADC2
 *            ADC3
 * @param single_ended If 1 then the ADC is single ended. If 0 then
 *                     the ADC is differential.
 *
 * @return The mux value
 *
 * 000 : AINP = AIN0 and AINN = AIN1 (default)
 * 001 : AINP = AIN0 and AINN = AIN3
 * 010 : AINP = AIN1 and AINN = AIN3
 * 011 : AINP = AIN2 and AINN = AIN3
 * 100 : AINP = AIN0 and AINN = GND
 * 101 : AINP = AIN1 and AINN = GND
 * 110 : AINP = AIN2 and AINN = GND
 * 111 : AINP = AIN3 and AINN = GND
 **/
static int8_t get_mux_value(uint8_t adc, bool single_ended) {
	int mux_value = -1;

	if( single_ended ) {
		if( adc == ADC0 ) {
			mux_value = 0b100;
		}
		else if( adc == ADC1 ) {
			mux_value = 0b101;
		}
		else if( adc == ADC2 ) {
			mux_value = 0b110;
		}
		else if( adc == ADC3 ) {
			mux_value = 0b111;
		}
	}
	else {
		if( adc == ADC0 ) {
			mux_value = 0b000;
		}
		else if( adc == ADC1 ) {
			mux_value = 0b001;
		}
		else if( adc == ADC2 ) {
			mux_value = 0b010;
		}
		else if( adc == ADC3 ) {
			mux_value = 0b011;
		}
	}

	return mux_value;
}

/**
 * @brief Read a 16 bit value from the ADS111x device
 * @param device_address The I2C device address.
 * @param reg_address The register address.
 * @return The value read from the register.
 */
static uint16_t read_16_bit_reg(uint16_t device_address, uint8_t reg_address) {
	struct mgos_i2c *i2c_conn = mgos_i2c_get_global();

	return (uint16_t)mgos_i2c_read_reg_w(i2c_conn, device_address, reg_address);
}

/**
 * @brief Write a 16 bit value from the ADS111x device
 * @param device_address The I2C device address.
 * @param reg_address The register address.
 * @param reg_value The value to write to the register.
 * @return 1 on success, 0 on error.
 */
static bool write_16_bit_reg(uint16_t device_address, uint8_t reg_address, uint16_t reg_value) {
	struct mgos_i2c *i2c_conn = mgos_i2c_get_global();

	return mgos_i2c_write_reg_w(i2c_conn, device_address, reg_address, reg_value);
}

/**
 * @brief Wait for an ADC conversion to complete.
 * @param device_address The I2C device address.
 * @return void
 */
static void waitfor_adc_conversion_completion(uint16_t device_address) {
	int cfg_reg_state = 0;

	while(1) {
		cfg_reg_state = read_16_bit_reg(device_address, CONFIG_REG);

        if( cfg_reg_state&(1<<OS_CFG_REG_BIT) ) {
            break;
        }

	}

}

/**
 * @brief Setup an ADC.
 * @param device_address The address of the I2C device
 * @param adc The ADC to be selected (0,1,2 & 3 are valid for ADS115).
 *            Valid options are
 * 			  ADC0
 *            ADC1
 *            ADC2
 *            ADC3
 * @param fs_voltage_id The full scale voltage. Any of the following
 *        values are valid.
 * 			FS_VOLTAGE_6_144
 *			FS_VOLTAGE_4_096
 *			FS_VOLTAGE_2_048
 *			FS_VOLTAGE_1_024
 *			FS_VOLTAGE_0_512
 *			FS_VOLTAGE_0_256
 * @param samples_per_second Samples per second value.
 * 	SAMPLES_PER_SECOND_8,
 *	SAMPLES_PER_SECOND_16,
 *	SAMPLES_PER_SECOND_32,
 *	SAMPLES_PER_SECOND_64,
 *	SAMPLES_PER_SECOND_128,
 *	SAMPLES_PER_SECOND_250,
 *	SAMPLES_PER_SECOND_475,
 *	SAMPLES_PER_SECOND_860
 * @param single_ended If 1 then the ADC is single ended. If 0 then
 *                     the ADC is differential.
 * @return The ADC value or FS_VOLTAGE_ERROR, MUX_ERROR, SPS_ERROR (all greater than 65535)
 */
uint16_t get_adc_value(uint16_t device_address, uint8_t adc, uint8_t fs_voltage_id, uint8_t samples_per_second, bool single_ended) {
	uint16_t adc_value = 0;
	uint16_t reg_value = 0;
	int8_t pga_value = get_pga_value(fs_voltage_id);
	int8_t mux_value = get_mux_value(adc, single_ended);
	int8_t rate_value = get_data_rate_value(samples_per_second);

	if( pga_value == -1 ) {
		return FS_VOLTAGE_ERROR;
	}

	if( mux_value == -1 ) {
		return MUX_ERROR;
	}

	if( rate_value == -1 ) {
		return SPS_ERROR;
	}

	reg_value	=	1					<< OS_CFG_REG_BIT         |
					mux_value       	<< MUX_CFG_REG_BIT        |
					pga_value       	<< PGA_CFG_REG_BIT        |
					DEFAULT_MODE    	<< MODE_CFG_REG_BIT       |
					rate_value      	<< DR_CFG_REG_BIT         |
					DEFAULT_COMPMODE	<< COMP_MODE_CFG_REG_BIT  |
					DEFAULT_COMPPOL   	<< COMP_POL_CFG_REG_BIT   |
					DEFAULT_COMPLAT   	<< COMP_LAT_CFG_REG_BIT   |
					DEFAULT_COMPQUE   	<< COMP_QUE_CFG_REG_BIT;

	//Called in case a conversion is in progress
	waitfor_adc_conversion_completion(device_address);

	//Initiate a conversion
	write_16_bit_reg(device_address, CONFIG_REG, reg_value);

	//Wait for the conversion to complete
	waitfor_adc_conversion_completion(device_address);

	//Read the ADC value
	adc_value = read_16_bit_reg(device_address, CONVERSION_REG);

	return adc_value;
}

/**
 * @brief The mgos lin init function.
 **/
bool mgos_ads111x_init(void) {

	return true;
}
