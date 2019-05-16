#ifndef _WIFI_SETUP
#define _WIFI_SETUP

enum led_flash_states {WIFI_LED_OFF, WIFI_LED_OFF_FLASH_ON, WIFI_LED_FLASH_FAST, WIFI_LED_ON_FLASH_OFF, WIFI_LED_ON};

enum {
	WIFI_MODE_AP,       //Default wifi mode as define in mos.yml
	WIFI_MODE_TERMINAL,
	WIFI_MODE_AP_TERMINAL
};

#define ROOT_URI				"/"
#define ROOT_INDEX_HTML_URI		"/index.html"
#define SETUP_HTML 				"setup.html"
#define PRODUCT_HTML 			"product.html"

#define WIFI_TIMER_PERIOD_MS 75
#define WIFI_MAX_BUTTON_HOLD_COUNT 5000/WIFI_TIMER_PERIOD_MS

void setup_wifi_timer_cb(void *arg);
void wifi_button_setup_init(void);
void enableWiFiSetupMode(void);
char *get_wifi_ap_ip_address(void);
char *get_wifi_sta_ip_address(void);
void saveConfig(void);
bool get_wifi_button_pressed(void);
void set_wifi_led(bool on);
void send_ayt_response(struct mg_connection *nc);

#endif
