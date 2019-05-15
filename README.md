# ADS1113/4/5
Mongoose OS driver for the ADS1113 ADS114 and ADS115 ADC devices.

## Introduction

These devices are ADC's and this deiver provides an API to MGOS_APP_INIT_SUCCESSthe ADC functionality.
See http://www.ti.com/lit/ds/symlink/ads1115.pdf for more
information on the devices.

### API

The get_adc_value() function is all the user needs to call
in order to read the ADC value.

### Example Application

#### mos.yml

The driver uses the Mongoose native I2C driver. It is configured by setting
up the `SDA`, `SCL` pins and setting the I2C bus speed.

```
config_schema:
- ["i2c.enable", "b", true, {title: "Enable I2C"}]
- ["i2c.freq", "i", 100000, {title: "I2C bus speed"}]
- ["i2c.unit_no", "i", 0, {title: "Which hardware unit to use, 0 or 1"}]
- ["i2c.sda_gpio", "i", 22, {title: "GPIO to use for SDA"}]
- ["i2c.scl_gpio", "i", 21, {title: "GPIO to use for SCL"}]
```

#### Application

```
#include "mgos.h"
#include "ads111x.h"

#define  0x48

static void adc_read_cb(void *arg) {
  static int adc_value = 0;

  adc_value = get_adc_value((uint16_t)I2C_DEV_ADDRESS, (uint8_t)ADC0, (uint8_t)FS_VOLTAGE_2_048, (uint8_t)SAMPLES_PER_SECOND_128, (bool)false);

  LOG(LL_INFO, ("%s: adc value = %d", __FUNCTION__, adc_value);

}


enum mgos_app_init_result mgos_app_init(void) {

    mgos_set_timer(1000, MGOS_TIMER_REPEAT, adc_read_cb, NULL);

    return MGOS_APP_INIT_SUCCESS;
}
```
