#include "esp_common.h"
#include "esp_system.h"
#include "gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "settings/settings.h"

#include "debug.h"

#define ETS_GPIO_INTR_ENABLE()  _xt_isr_unmask(1 << ETS_GPIO_INUM)
#define ETS_GPIO_INTR_DISABLE() _xt_isr_mask(1 << ETS_GPIO_INUM)

extern void mon_task(void *pvParameters);
extern void bcast_lsn(void *pvParameters);
extern void webpage_task(void *pvParameters);
extern void reg_service(void *pvParameters);
extern void prcs_sen_data(void *pvParameters);

extern void gpio_init(void);
extern void mon_mode_gpio_init(void);

extern uint8_t water_low_pin;
extern uint8_t water_spill_pin;

uint32 user_rf_cal_sector_set(void) {
	return (1024 - 5);	/*Specific for flash size of 32Mbit/4MB with sector size of 4KB*/
}

static void _start_ap_mode(char *r_ssid, char *r_passwd) {
	wifi_set_opmode(SOFTAP_MODE);

	struct softap_config ap_config = {
		.ssid_hidden = 0,
		.channel = 11,
		.ssid_len = 0,
		.authmode = AUTH_WPA_WPA2_PSK,
		.max_connection = 4,
		.beacon_interval = 100,
	};
	strncpy(ap_config.ssid, r_ssid, sizeof(ap_config.ssid));
	strncpy(ap_config.password, r_passwd, sizeof(ap_config.password));

	wifi_softap_dhcps_stop();

	struct ip_info info;
	IP4_ADDR(&info.ip, 10, 10, 10, 1);
	IP4_ADDR(&info.gw, 10, 10, 10, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &info);

	struct dhcps_lease dhcp_range;
	dhcp_range.enable = true;
	IP4_ADDR(&dhcp_range.start_ip, 10, 10, 10, 2);
	IP4_ADDR(&dhcp_range.end_ip, 10, 10, 10, 5);

	wifi_softap_set_dhcps_lease(&dhcp_range);
	wifi_softap_set_dhcps_lease_time(3);
	wifi_softap_dhcps_start();

	wifi_softap_set_config(&ap_config);
}

static void _connect_to_wifi(char *r_ssid, char *r_passwd) {
	struct station_config sta_conf;
	strncpy(sta_conf.ssid, r_ssid, sizeof(sta_conf.ssid));
	strncpy(sta_conf.password, r_passwd, sizeof(sta_conf.password));	

	wifi_set_opmode(STATION_MODE);
	wifi_station_set_config(&sta_conf);

	// wifi_station_connect();	/*this api should not be called from user_init*/
}

static void _update_motor_state(void) {
	__BIND_TANK_ *tank_link = get_tank_linkage();

	for(uint8_t i = 0; i < 5; i++) {
		if((tank_link[i].src_tank[0] != 0) && (tank_link[i].dst_tank[0] != 0)) {
			if(tank_link[i].motor_pin != 16) {
				GPIO_OUTPUT_SET(tank_link[i].motor_pin, tank_link[i].motor_state);
			} else {
				gpio16_output_set(tank_link[i].motor_state);
			}
		} else {
			break;
		}
	}
}

/*
 * ISR to handle interrupts from sensor pins
 * Is initialized only when controller is also acting as monitor
*/ 
void io_intr_handler(void) {
	uint32_t status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);          //READ STATUS OF INTERRUPT
	if ((status & BIT(water_low_pin)) || (status & BIT(water_spill_pin))) {
		xTaskCreate(mon_task, "Monitor Task", 512, NULL, 1, NULL);
		TRACE("Interrupt is working!");
	}

	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);       //CLEAR THE STATUS IN THE W1 INTERRUPT REGISTER
}

/*
 * Reads saved settings, chooses appropriate mode and updates previously known motor on/off state
 * Tasks:
 * 		Serves webpage for configuration
 * 		Continuously listens for broadcast message sent by nodes,
 * 		A task to register new nodes
 * 		A task to process data sent by nodes
*/
void user_init(void) {
	settings_init();

	_S_SETTINGS_INFO *all_setng = get_all_settings();

	switch(all_setng->esp_mode) {
		case STANDAlONE_MODE:
		_start_ap_mode(all_setng->ssid, all_setng->passwd);
		break;

		case ROUTER_MODE:
		case MQTT_MODE:
		_connect_to_wifi(all_setng->ssid, all_setng->passwd);
		break;

		default:
		_start_ap_mode(all_setng->ssid, all_setng->passwd);
		break;
	}

	gpio_init();
	_update_motor_state();

	/* 
	 * Check if controller is also used as monitor
	 * 
	 * Currently, a restart is required when monitor mode is enabled/disabled 
	 */
	if(all_setng->en_mon_mod) {
		mon_mode_gpio_init();
		gpio_intr_handler_register(io_intr_handler, NULL);
		ETS_GPIO_INTR_ENABLE();
	}

	xTaskCreate(bcast_lsn, "Bcast Listen", 512, NULL, 1, NULL);
	xTaskCreate(webpage_task, "Webpage_Task", 176, NULL, 1, NULL);
	xTaskCreate(reg_service, "Dev_Reg", 176, NULL, 1, NULL);
	xTaskCreate(prcs_sen_data, "Process Sensor", 176, NULL, 1, NULL);
}
