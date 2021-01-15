#include "esp_common.h"
#include "esp_system.h"
#include "gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "settings/settings.h"

#include "debug.h"

/*
 * mon_task is created from ISR every time falling edge is detected in sensor pins and is deleted after execution
 * This task updates the tank status in the settings and takes appropriate action on motor
*/

extern void dst_low_action(char *dst);
extern void dst_full_action(char *dst);
extern void src_low_action(char *src);
extern void src_full_action(char *src);


extern uint8_t water_low_pin;
extern uint8_t water_spill_pin;

void mon_task(void *pvParameters) {
	uint8_t up = 0;
	uint8_t bottom = 0;

	up = (~(gpio_input_get()>>water_spill_pin)) & (BIT(0));
	bottom = (~(gpio_input_get()>>water_low_pin)) & (BIT(0));

	uint8_t new_state;

	switch((up<<1)|(bottom)) {
		case 0b10:
		new_state = 1;
		break;

		case 0b01:
		new_state = 0;
		break;

		default:
		vTaskDelete(NULL);
		break;
	}

	__TANK_SETTINGS_ *tank_details = get_tank_settings();
	_S_SETTINGS_INFO *all_setng = get_all_settings();

	char self_name[32];
	strncpy(self_name, all_setng->mon_mode_name, sizeof(self_name));

	for(uint8_t i = 0; i < 5; i++) {	/*We are assuming system can handle max. 5 tanks*/
		if(!strcmp(self_name, tank_details[i].tank_name)) {
			tank_details[i].tank_state = new_state;
			
			switch(tank_details[i].tank_oprn) {
				case Dst:
				switch(new_state) {
					case Low:
					dst_low_action(tank_details[i].tank_name);
					break;

					case Full:
					dst_full_action(tank_details[i].tank_name);
					break;
				}
				break;

				case Src:
				switch(new_state) {
					case Low:
					src_low_action(tank_details[i].tank_name);
					break;

					case Full:
					src_full_action(tank_details[i].tank_name);
					break;
				}
				break;

				case SrcDst:
				switch(new_state) {
					case Low:
					dst_low_action(tank_details[i].tank_name);
					src_low_action(tank_details[i].tank_name);
					break;

					case Full:
					src_full_action(tank_details[i].tank_name);
					dst_full_action(tank_details[i].tank_name);
					break;
				}
				break;

				default:
				break;
			}
			break;
		}
	}

	save_settings_to_flash();

	vTaskDelete(NULL);
}
