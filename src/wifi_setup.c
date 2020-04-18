#include "mgos.h"
#include "mgos_ro_vars.h"
#include "mgos_vfs.h"
#include "mgos_rpc.h"
#include "mgos_http_server.h"

#include "wifi_setup.h"

static char sta_ip[16];
static char ap_ip[16];

static void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg);

/**
 * @brief We use two GPIO pins for Wifi setup.
 *        This file contains the code that handles the Wifi setup in a fairly standard way.
 *
 * One pin, wifi_button_gpio (defined in mos.yml) has a button connected that if held down
 * for a short period (until the Wifi LED flashes fast) allows the user to put the device in
 * Wifi setup mode.
 * The other pin, wifi_led_gpio (defined in mos.yml) has an LED connected to it and flashes
 * fast as detailed above. When connectting to the Wifi network this LED flashes spending
 * more time on until connected to the Wifi network and served an IP address by a DHCP
 * server at which point this LED stays in the on state without flashing.
 */

/**
 * @brief Init gpio pins used for handling the WiFi interface.
 *        We use one input pin connected to a button and one output pin connected to an LED.
 */
static void init_gpio(void) {

    mgos_gpio_set_pull(mgos_sys_config_get_platform_wifi_button_gpio(), MGOS_GPIO_PULL_UP);
    mgos_gpio_set_mode(mgos_sys_config_get_platform_wifi_button_gpio(), MGOS_GPIO_MODE_INPUT);

    mgos_gpio_set_mode(mgos_sys_config_get_platform_wifi_led_gpio(), MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_write(mgos_sys_config_get_platform_wifi_led_gpio(), 0);

}

/**
 * @brief get the state of the wifi button.
 * @return true if button pressed.
 */
bool get_wifi_button_pressed(void) {
    if( mgos_gpio_read(mgos_sys_config_get_platform_wifi_button_gpio()) ) {
        return false;
    }
    return true;
}

/**
 * @brief Set the state of the wifi LED.
 * @param on If true the LED is on.
 */
void set_wifi_led(bool on) {
    mgos_gpio_write(mgos_sys_config_get_platform_wifi_led_gpio(), on);
}

/**
 * @brief Change the state of the Wifi status LED.
 */
void toggle_wifi_led(void) {
	mgos_gpio_toggle(mgos_sys_config_get_platform_wifi_led_gpio());
}



/**
 * @brief Save the state of all config attributes.
 **/
void saveConfig(void) {
	save_cfg(&mgos_sys_config, NULL);
}



/***
 * @brief Get the WiFi AP IP address.
 *        A valid IP address does not mean the unit is running in AP mode.
 */
char *get_wifi_ap_ip_address(void) {
	return ap_ip;
}

/***
 * @brief Get the WiFi station IP address. Maybe a zero length string if
 *        no DHCP address has been assigned.
 */
char *get_wifi_sta_ip_address(void) {
	return sta_ip;
}




/**
 * @brief Checks for a long button press to enter setup mode
 *        and flashes the Wifi LED to indicate Wifi state.
 *
 *        Hold the wifi button down for TIMER0_PERIOD_MS seconds
 *        (the wifi led will flash fast as soon as the wifi button
 *        has been held down long enough to enter setup mode) and
 *        then release to enter setup mode.
 *        The Wifi LED states are
 *        Off = Wifi not connected
 *        Off flashing On = Wifi connecting
 *        On flashing off = Wifi connected
 *        On = Wifi connected and IP address allocated
 **/
void setup_wifi_timer_cb(void *arg) {
    static uint32_t buttonHoldCount 	= 0;
    static int     	ledFlashCount		= 0;
    uint8_t 		wifiLedState		= WIFI_LED_OFF;
    uint8_t 		enteringSetupMode	= 0;
    struct 			mgos_net_ip_info 	ip_info;
    bool 			wifiButonPressed 	= get_wifi_button_pressed();
    int 			wifiStatus 			= mgos_wifi_get_status();

    if( wifiButonPressed ) {
        buttonHoldCount++;
        if( buttonHoldCount >= WIFI_MAX_BUTTON_HOLD_COUNT ) {
            wifiLedState=WIFI_LED_FLASH_FAST;
            enteringSetupMode=1;
        }
    }
    else {
    	//User has released the button after holding it down for a time
    	//in order to setup wifi.
        if( buttonHoldCount >= WIFI_MAX_BUTTON_HOLD_COUNT ) {
            mgos_gpio_write(mgos_sys_config_get_platform_wifi_led_gpio(), 0);
            enableWiFiSetupMode();
        }
        buttonHoldCount=0;
    }

    if( !enteringSetupMode ) {
        //If not in setup mode
        if( !mgos_sys_config_get_user_setup_mode() ) {
            if( mgos_sys_config_get_user_wifi_mode() == WIFI_MODE_TERMINAL || mgos_sys_config_get_user_wifi_mode() == WIFI_MODE_AP_TERMINAL) {
                wifiLedState=WIFI_LED_OFF;

                if( wifiStatus == MGOS_WIFI_CONNECTING ) {
                    wifiLedState=WIFI_LED_OFF_FLASH_ON;
                }
                else if( wifiStatus == MGOS_WIFI_CONNECTED ) {
                    wifiLedState=WIFI_LED_ON_FLASH_OFF;
                }
                else if( wifiStatus == MGOS_WIFI_IP_ACQUIRED ) {
                    wifiLedState=WIFI_LED_ON;
                }
            }
            else if(  mgos_sys_config_get_user_wifi_mode() == WIFI_MODE_AP ) {
                wifiLedState=WIFI_LED_ON;
            }
        }
    }

    //If in setup mode and the button has been held down long enough flash LED fast
    if( wifiLedState == WIFI_LED_OFF ) {
        set_wifi_led(false);
    }
    else if( wifiLedState == WIFI_LED_FLASH_FAST ) {
    	LOG(LL_INFO, ("Release switch to reset to factory defaults.\n") );
    	toggle_wifi_led();
    }
    else if( wifiLedState == WIFI_LED_OFF_FLASH_ON ) {
        ledFlashCount++;
        if( ledFlashCount%6 == 0 ) {
            set_wifi_led(true);
            ledFlashCount=0;
        }
        else {
            set_wifi_led(false);
        }
    }
    else if( wifiLedState == WIFI_LED_ON_FLASH_OFF ) {
        ledFlashCount++;
        if( ledFlashCount%6 == 0 ) {
            set_wifi_led(false);
            ledFlashCount=0;
        }
        else {
            set_wifi_led(true);
        }
    }
    else if( wifiLedState == WIFI_LED_ON ) {
        set_wifi_led(true);
    }

    //The AP IP address should not change while we're running so only do this once
    if( strlen(ap_ip) == 0 ) {
		if (mgos_net_get_ip_info(MGOS_NET_IF_TYPE_WIFI, MGOS_NET_IF_WIFI_AP, &ip_info)) {
			mgos_net_ip_to_str(&ip_info.ip, ap_ip);
	        fprintf(stdout, "AP IP address = %s\n", ap_ip);
		}
    }

    //We don't check wifiStatus == MGOS_WIFI_IP_ACQUIRED as we may miss this state during polling.
	if (mgos_net_get_ip_info(MGOS_NET_IF_TYPE_WIFI, MGOS_NET_IF_WIFI_STA, &ip_info)) {
		if( &ip_info.ip ) {
			mgos_net_ip_to_str(&ip_info.ip, sta_ip);
		}
	}

    (void)arg;
}

/**
 * @brief callback function to cause a reboot.
 */
static void reboot_cb(void *arg) {
	mgos_system_restart();

	(void)arg;
}

/**
 * @brief Trigger a restart in the future.
 * @param milli_seconds The number of milliseconds from the call time before the restart occurs.
 */
static void set_restart_ms(int milli_seconds) {
    mgos_set_timer(milli_seconds, MGOS_TIMER_REPEAT, reboot_cb, NULL);
}

/**
 * @brief Enable setup mode SSID. In setup mode the SSID starts with the
 * product ID and ends with the MAC address.
 **/
static void setupModeSSID(void) {
  const char *ssid_prefix = mgos_sys_config_get_ydev_product_id();
  if( ssid_prefix == NULL ) {
    ssid_prefix="UNSETPRODUCTID";
  }
  struct mg_str mac = mg_mk_str(mgos_sys_ro_vars_get_mac_address());
  LOG(LL_INFO, ("-----------------------------------> mgos_sys_config_get_device_id: %s (mac: %.*s)",
                 mgos_sys_config_get_device_id(), mac.len, mac.p));

   //The default SSID will have a prefix of the product ID followed
   //by the last three digits of the devices WiFi MAC address.
   static char deafultSSID[33];
   snprintf(deafultSSID, 32, "%s_%s", ssid_prefix, mac.p+6 );

   //We want a default SSID starting with the product ID
   mgos_sys_config.wifi.ap.enable=1;
   mgos_sys_config.wifi.ap.ssid=deafultSSID;
   mgos_sys_config.wifi.sta.enable=0;
   mgos_sys_config.wifi.sta.ssid="";
   mgos_sys_config.wifi.sta.pass="";

   mgos_sys_config.user.setup_mode=1;
   mgos_sys_config.user.wifi_mode=WIFI_MODE_AP_TERMINAL;

   mgos_wifi_setup_ap(&mgos_sys_config.wifi.ap);
   mgos_wifi_setup_sta(&mgos_sys_config.wifi.sta);

   saveConfig();
}

/**
 * @brief Called to enter wifi setup mode.
 */
void enableWiFiSetupMode(void) {

    mgos_config_reset(MGOS_CONFIG_LEVEL_USER);

    setupModeSSID();

    set_restart_ms(0);

}

/**
 * @brief Convert from mg_str struct to C string (null terminated).
 * 		  The caller must free the memory allocated to hold the C string.
 */
static char * mg_str2c_str(struct mg_str *mgstr)
{
	char * textBuf;

	if( mgstr->len > 0 ) {
		textBuf = malloc(mgstr->len + 1);
		strncpy(textBuf, mgstr->p, mgstr->len);
		textBuf[mgstr->len] = '\0';
		return textBuf;
	}
	return "";
}


/**
 * @brief handle all http requests and filter out the root and index.html requests.
 *        If a / or /index.html is made then return either the setup html or product html
 *        depending on whether we are in setup mode.
 */
static void root_handler(struct mg_connection *mg_con, int ev, void *p, void *user_data) {
	static struct mg_serve_http_opts s_http_server_opts;

	if (ev == MG_EV_HTTP_REQUEST) {
		int root_request = 0;
		struct http_message *hm = (struct http_message *) p;
		char *uri = NULL;

		if (hm->uri.len > 0) {
			uri = mg_str2c_str(&hm->uri);
			if (strcmp(uri, ROOT_URI) == 0 || strcmp(uri, ROOT_INDEX_HTML_URI) == 0) {
				root_request = 1;
			}
			free(uri);
			uri = NULL;
		}

		if (root_request) {

			if ( mgos_sys_config_get_user_setup_mode() ) {

				mg_http_serve_file(mg_con, (struct http_message *) p, SETUP_HTML, mg_mk_str("text/html"), mg_mk_str(""));

			} else {

				mg_http_serve_file(mg_con, (struct http_message *) p, PRODUCT_HTML, mg_mk_str("text/html"), mg_mk_str(""));

			}

		} else {

			//As it's not a root request serve the file requested.
			if (hm != NULL) {
				mg_serve_http(mg_con, hm, s_http_server_opts);
			}

		}

	}

	(void)user_data;
}


/*
 * @brief Callback handler that is called once the wifi is configured.
 *        This will save the WiFi config persistently and reboot.
 * @param ri
 * @param cb_arg
 * @param fi
 * @param args
 */
static void mgos_rpc_wifi_setup_complete(struct mg_rpc_request_info *ri,
                                       void *cb_arg,
                                       struct mg_rpc_frame_info *fi,
                                       struct mg_str args) {
	saveConfig();
	mg_rpc_send_responsef(ri, NULL);
	set_restart_ms(500); //Wait for RPC response to get through before restart

	(void)cb_arg;
	(void)fi;
	(void)args;
}



/*
 * @brief When scanning for WiFi channels format the result as JSON data.
 * @param out
 * @param ap
 */
static int wifi_scan_result_printer(struct json_out *out, va_list *ap) {
    int len = 0;
    int num_res = va_arg(*ap, int);
    LOG(LL_INFO, ("%s: Networks found = %d", __FUNCTION__, num_res) );

    const struct mgos_wifi_scan_result *res = va_arg(*ap, const struct mgos_wifi_scan_result *);

    for (int i = 0; i < num_res; i++) {
        if (i > 0) len += json_printf(out, ", ");
        len +=
            json_printf(out, "{ssid: %Q, auth: %d, channel: %d, rssi: %d}",
                        res[i].ssid, res[i].auth_mode, res[i].channel, res[i].rssi);
        LOG(LL_INFO, ("SSID: %s, AUTH MODE: %d, CHNL: %d, RSSI: %d", res[i].ssid, res[i].auth_mode, res[i].channel, res[i].rssi) );
    }

    return len;
}

/*
 * @brief Callback handler to scan for WiFi channels.
 * @param ri
 * @param cb_arg
 * @param fi
 * @param args
 */
static void mgos_rpc_wifi_scan_handler(struct mg_rpc_request_info *ri,
                                       void *cb_arg,
                                       struct mg_rpc_frame_info *fi,
                                       struct mg_str args) {

    mgos_wifi_scan(wifi_scan_cb, ri);

    (void) args;
    (void) cb_arg;
    (void) fi;
}

/*
 * @brief Callback handler for that is called as the WiFi channels are scanned.
 * @param n
 * @param res
 * @param arg
 */
static void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg) {

    struct mg_rpc_request_info *ri = (struct mg_rpc_request_info *) arg;

    LOG(LL_INFO, ("%s: n = %d", __FUNCTION__, n) );
    if (n < 0) {
        mg_rpc_send_errorf(ri, n, "wifi scan failed");
        return;
    }
    mg_rpc_send_responsef(ri, "{results: [%M]}", wifi_scan_result_printer, n, res);
}

/**
 * @brief Handle DNS request in a captive DNS env.
 *        This is used in setup mode so that aqny URL takes the
 *        user to the local AP IP address.
 * @param mg_connection
 * @param ev
 * @param ev_data
 * @param user_data
 */
static void captive_dns_ev_handler(struct mg_connection *c, int ev, void *ev_data,
                                   void *user_data) {
    struct mg_dns_message *msg = (struct mg_dns_message *) ev_data;
    struct mbuf reply_buf;
    int i;

    if (ev != MG_DNS_MESSAGE) return;

    mbuf_init(&reply_buf, 512);
    struct mg_dns_reply reply = mg_dns_create_reply(&reply_buf, msg);
    for (i = 0; i < msg->num_questions; i++) {
        char rname[256];
        struct mg_dns_resource_record *rr = &msg->questions[i];
        mg_dns_uncompress_name(msg, &rr->name, rname, sizeof(rname) - 1);
        fprintf(stdout, "CAPTIVE_DNS: type = %d name = %s\n", rr->rtype, rname);
        if (rr->rtype == MG_DNS_A_RECORD) {
            uint32_t ip = inet_addr(mgos_sys_config.wifi.ap.ip);
            mg_dns_reply_record(&reply, rr, NULL, rr->rtype, 10, &ip, 4);
            fprintf(stdout, "CAPTIVE_DNS: AP IP = %s\n", mgos_sys_config.wifi.ap.ip);
        }
    }
    mg_dns_send_reply(c, &reply);
    mbuf_free(&reply_buf);
    (void) user_data;
}

/*
 * @brief Perform WiFi init for generic wifi configuration.
 */
void mgos_wifi_button_setup_init(void) {

    struct mg_rpc *con = mgos_rpc_get_global();

    if( mgos_sys_config_get_user_setup_mode() ) {
        struct mg_connection *c = mg_bind(mgos_get_mgr(), "udp://:53", captive_dns_ev_handler, 0);
        mg_set_protocol_dns(c);
        LOG(LL_INFO, ("CAPTIVE_DNS: enabled") );

//Setting SSID here causes crash when SSID and password is entered.
//Needs investigation.
//        setupModeSSID();
    }

    init_gpio();

    mgos_set_timer(WIFI_TIMER_PERIOD_MS, MGOS_TIMER_REPEAT, setup_wifi_timer_cb, NULL);

    mg_rpc_add_handler(con, "WiFi.Scan", "", mgos_rpc_wifi_scan_handler, NULL);
    mg_rpc_add_handler(con, "WiFi.SetupComplete", "", mgos_rpc_wifi_setup_complete, NULL);
    mgos_register_http_endpoint("/", root_handler, NULL);

    //Note !!!
    //On esp8266 board if this log message is not present then the init fails.
    //Investigation required to determine the reason for this.
    LOG(LL_INFO, ("%s: Complete", __FUNCTION__) );

}
