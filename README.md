# WiFi button setup
This lib allows ESP32 and ESP8266 product to configure the WiFi.

## Introduction

In order to use this library a GPI pin (typically GPI0 as an input) is connected to a
push button switch to ground. Another GPIO pin (typically GPIO2) is connected
to an LED through a resistor.
In order to connect to the Wifi The push button must be held down until (about 5
seconds) the LED flashes. The user can then connect to a WiFi network (the SSID
of the WiFi network will start with the product name and the password will be
12345678) via a mobile or tablet. Once the mobile of tablet is connected to the
devices WiFi network they can open a browser to 192.168.4.1 or any
url (E.G a.com) and the web page that will allow them to enter the WiFi
SSID and password is displayed. A scan of WiFi networks is made and a button is
display allowing the user to rescan Wifi networks. The WiFi signal level is
displayed and the SSID of the loudest signal is selected however the user can
select other SSIDs from a drop down list. Once the user has entered the WiFi
password a button can be selected to connect the device to the users WiFi
network.

### API

The mgos_wifi_button_setup_init() function is called automatically when the
library is included in mos.yaml. However a product.html file should be added
to your projects fs folder as this will be displayed in place of an index.html
file when the WiFi is setup on your device.

### Example Application
No code is required as detailed above execpt that the library is included in mos.yml.
The wifi parameters are not part of this li but are shown here for completness.

```
config_schema:
 - ["wifi.ap.enable",             "b", true,       {title: "Enable WiFi AP"}]
 - ["wifi.sta.enable",            "b", true,       {title: "Enable WiFi station"}]
 - ["wifi.ap.pass",               "s", "12345678", {title: "The default AP password when setting up."}]

 - ["platform", 					        "o", 	           {title: "Platform config. Defines connectivity of hardware."}]
 - ["platform.wifi_button_gpio", 	"i", 0,          {title: "GPIO pin used to setup wifi (input from button)"}]
 - ["platform.wifi_led_gpio", 		"i", 2,          {title: "GPIO pin used to setup wifi (output to LED)."}]

 - ["user", 						          "o",             {title: "Holds attributes that the user may configure."}]
 - ["user.setup_mode", 				    "i", 1,          {title: "If true then the unit is in setup mode. This occurs after the setup button has been held down for a time and released."}]
 - ["user.wifi_mode", 				    "i", 0,          {title: "The wifi mode (WIFI_MODE_AP=0 or WIFI_MODE_TERMINAL=1)."}]
 - ["user.product_id", 				    "s", "PRODID",   {title: "The Y product ID."}]
```

```
#include "mgos.h"

enum mgos_app_init_result mgos_app_init(void) {

    return MGOS_APP_INIT_SUCCESS;
}
```
