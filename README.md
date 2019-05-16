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
No code is required as detailed above execpt that the library is included in mos.yml

```
#include "mgos.h"

enum mgos_app_init_result mgos_app_init(void) {

    return MGOS_APP_INIT_SUCCESS;
}
```
