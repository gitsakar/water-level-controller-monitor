#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "settings/settings.h"
#include "debug.h"

extern uint8_t gpio_status(void);
extern void config_task(void *pvParameters);
extern void bcast_snd(void *pvParameters);

uint8_t sensor_state;

uint32 user_rf_cal_sector_set(void) {
	return (1024 - 5);	/*Specific for flash size of 32Mbit/4MB with sector size of 4KB*/
}

static void _start_ap_mode(void) {
	wifi_set_opmode(SOFTAP_MODE);

	struct softap_config ap_config = {
		.ssid = "NODE_CONFIG",
		.ssid_hidden = 0,
		.channel = 11,
		.ssid_len = 0,
		.authmode = AUTH_WPA_WPA2_PSK,
		.password = "aaaaaaaa",
		.max_connection = 1,
		.beacon_interval = 100,
	};

	wifi_softap_dhcps_stop();

	struct ip_info info;
	IP4_ADDR(&info.ip, 10, 10, 10, 1);
	IP4_ADDR(&info.gw, 10, 10, 10, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &info);

	struct dhcps_lease dhcp_range;
	dhcp_range.enable = true;
	IP4_ADDR(&dhcp_range.start_ip, 10, 10, 10, 2);
	IP4_ADDR(&dhcp_range.end_ip, 10, 10, 10, 2);

	wifi_softap_set_dhcps_lease(&dhcp_range);
	wifi_softap_set_dhcps_lease_time(3);
	wifi_softap_dhcps_start();

	wifi_softap_set_config(&ap_config);
}

static void _connect_to_wifi(void) {
	_S_SETTINGS_INFO *all_setng = get_all_settings();
	struct station_config sta_conf;
	strncpy(sta_conf.ssid, all_setng->ssid, sizeof(sta_conf.ssid));
	strncpy(sta_conf.password, all_setng->passwd, sizeof(sta_conf.password));

	wifi_set_opmode(STATION_MODE);
	wifi_station_set_config(&sta_conf);

	// wifi_station_connect();	/*this api should not be called from user_init*/
}

/*
 * Reads the sensor pin status and configuration pin status
 * If a user has pressed configuration pin, priority is given to configuraion pin
 * over sensor pins
*/
void user_init(void) {
	sensor_state = gpio_status();

	settings_init();
	if(sensor_state == WEB_MODE) {
		_start_ap_mode();
		xTaskCreate(config_task, "Config", 512, NULL, 1, NULL);
	} else {
		_connect_to_wifi();
		xTaskCreate(bcast_snd, "Bcast Snd", 512, NULL, 1, NULL);
		// _sensor_core_function(sensor_state);
	}
}
