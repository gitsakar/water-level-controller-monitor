#include "esp_common.h"
#include "gpio.h"

#include "settings/settings.h"

#include "debug.h"

/*
 * This is the main controller task called when nodes send their data
 * The task checks the tank type[dst/src/src+dst] that sent the data,
 * looks the state of corresponding tank pair and decides whether to 
 * turn on or off the corresponding motor
*/

extern char recv_node_name[32];
extern uint8_t new_node_state;

void dst_low_action(char *dst) {
	__BIND_TANK_ *tank_link = get_tank_linkage();
	__TANK_SETTINGS_ *tank_details = get_tank_settings();

	char linked_src_name[32];
	uint8_t linked_mot_pin;
	uint8_t linked_index_no;

	uint8_t src_state;
	for(uint8_t i = 0; i < 5; i++) {		/*We are assuming system can handle max. 5 tanks*/
		if(!strcmp(dst, tank_link[i].dst_tank)) {
			strncpy(linked_src_name, tank_link[i].src_tank, 32);
			linked_mot_pin = tank_link[i].motor_pin;
			linked_index_no = i;
			break;
		}
	}

	for(uint8_t i = 0; i < 5; i++) {
		if(!strcmp(linked_src_name, tank_details[i].tank_name)) {
			src_state = tank_details[i].tank_state;
			break;
		}
	}

	if(src_state) {
		//Start Corresponding Motor
		if(linked_mot_pin != 16) {
			GPIO_OUTPUT_SET(linked_mot_pin, false);
		} else {
			gpio16_output_set(false);
		}
		tank_link[linked_index_no].motor_state = false;
		TRACE("Destination is Low; Source is Full; Starting Motor");
	}
}

void dst_full_action(char *dst) {
	//Stop Corresponding Motor

	__BIND_TANK_ *tank_link = get_tank_linkage();
	uint8_t linked_mot_pin;

	for(uint8_t i = 0; i < 5; i++) {		/*We are assuming system can handle max. 5 tanks*/
		if(!strcmp(dst, tank_link[i].dst_tank)) {
			linked_mot_pin = tank_link[i].motor_pin;
			if(linked_mot_pin != 16) {
				GPIO_OUTPUT_SET(linked_mot_pin, true);
			} else {
				gpio16_output_set(true);
			}
			tank_link[i].motor_state = true;
			break;
		}
	}

	TRACE("Destination is now full");
}

void src_low_action(char *src) {
	//Stop Corresponding Motor

	__BIND_TANK_ *tank_link = get_tank_linkage();
	uint8_t linked_mot_pin;

	for(uint8_t i = 0; i < 5; i++) {		/*We are assuming system can handle max. 5 tanks*/
		if(!strcmp(src, tank_link[i].src_tank)) {
			linked_mot_pin = tank_link[i].motor_pin;
			if(linked_mot_pin != 16) {
				GPIO_OUTPUT_SET(linked_mot_pin, true);
			} else {
				gpio16_output_set(true);
			}
			tank_link[i].motor_state = true;
			break;
		}
	}

	TRACE("Source is now low");
}

void src_full_action(char *src) {
	__BIND_TANK_ *tank_link = get_tank_linkage();
	__TANK_SETTINGS_ *tank_details = get_tank_settings();

	char linked_dst_name[32];
	uint8_t linked_mot_pin;
	uint8_t linked_index_no;

	uint8_t dst_state;
	for(uint8_t i = 0; i < 5; i++) {		/*We are assuming system can handle max. 5 tanks*/
		if(!strcmp(src, tank_link[i].src_tank)) {
			strncpy(linked_dst_name, tank_link[i].dst_tank, 32);
			linked_mot_pin = tank_link[i].motor_pin;
			linked_index_no = i;
			break;
		}
	}

	for(uint8_t i = 0; i < 5; i++) {
		if(!strcmp(linked_dst_name, tank_details[i].tank_name)) {
			dst_state = tank_details[i].tank_state;
			break;
		}
	}

	if(!dst_state) {
		//Start Corresponding Motor
		if(linked_mot_pin != 16) {
			GPIO_OUTPUT_SET(linked_mot_pin, false);
		} else {
			gpio16_output_set(false);
		}
		tank_link[linked_index_no].motor_state = false;
		TRACE("Source is Full; Destination is Low");
	}
}

void controller(void *pvParameters) {
	__TANK_SETTINGS_ *tank_details = get_tank_settings();

	for(uint8_t i = 0; i < 5; i++) {	/*We are assuming system can handle max. 5 tanks*/
		if(!strcmp(recv_node_name, tank_details[i].tank_name)) {
			tank_details[i].tank_state = new_node_state;
			
			switch(tank_details[i].tank_oprn) {
				case Dst:
				switch(new_node_state) {
					case Low:
					dst_low_action(tank_details[i].tank_name);
					break;

					case Full:
					dst_full_action(tank_details[i].tank_name);
					break;
				}
				break;

				case Src:
				switch(new_node_state) {
					case Low:
					src_low_action(tank_details[i].tank_name);
					break;

					case Full:
					src_full_action(tank_details[i].tank_name);
					break;
				}
				break;

				case SrcDst:
				switch(new_node_state) {
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
