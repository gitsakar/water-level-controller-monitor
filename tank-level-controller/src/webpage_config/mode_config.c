#include <string.h>

#include "esp_common.h"
#include "esp_system.h"

#include "lwip/api.h"

#include "webserver.h"
#include "settings/settings.h"

#include "debug.h"

#define GET_LINK				"GET /mode_config HTTP"
#define POST_LINK				"POST /mode_config HTTP"

/*
 * This page provdies core confinguratio of device such as mode[mqtt/router/standalone],
 * controller name, ssid, password, enable monitor mode etc.
*/

extern const char *html_header;
extern const char *html_header1;
extern const char *html_header2;
extern const char *htmlFooter;
extern const char *esp_mode_drop_list;
extern const char *text_box;
extern const char *mon_mode;

extern void urldecode2(uint8_t *dst , const uint8_t *src);

static const char *_mode_text(uint8_t mode) {
	switch(mode) {
		case 1:
		return "Standalone Mode";

		case 2:
		return "Router Mode";

		case 3:
		return "MQTT Mode";

		default:
		return "Please Select One";
	}
}

static uint32_t _get_mode_config(struct netconn *conn, struct netbuf *recv) {
	char *temp_buffer;
	
	temp_buffer = pvPortMalloc(1024);
	if(temp_buffer == 0) {
		return 0;
	}

	_S_SETTINGS_INFO *crnt_setng = get_all_settings();

	const char *check_state = "unchecked";
	if(crnt_setng->en_mon_mod) {
		check_state = "checked";
	}

	webserver_webpage_chunked_data_new(temp_buffer, 1024, conn, html_header, "Configuration");
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header1);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header2, "Mode Config");
	
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, esp_mode_drop_list, _mode_text(crnt_setng->esp_mode));
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, text_box, "SSID:", "ssid", crnt_setng->ssid);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, text_box, "Password:", "passwd", crnt_setng->passwd);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, text_box, "Controller Name:", "cntname", crnt_setng->controller_name);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, mon_mode, check_state);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, text_box, "Name for Monitor Mode:", "monname", crnt_setng->mon_mode_name);
	
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, htmlFooter);
	webserver_webpage_chunked_data_end(conn);

	vPortFree(temp_buffer);

	return 1;
}

static uint32_t _post_mode_config(struct netconn *conn, struct netbuf *rx_data) {
	char *data, *ptr1;
	uint32_t temp = 0;

	struct netbuf *ptr_temp = 0;
	char *buffer = 0;

	TRACE ("Homepage POST called");
	conn->recv_timeout = 100;
	while((netconn_recv(conn, &ptr_temp)) == ERR_OK){
		if(ptr_temp) {
			netbuf_chain(rx_data, ptr_temp);
			ptr_temp = 0;
		} else {
			break;
		}
	}

	buffer = pvPortMalloc(rx_data->ptr->tot_len + 1);
	if(buffer == 0) {
		return 0;
	}
	netbuf_first(rx_data);
	do {
		memcpy(&buffer[temp], rx_data->ptr->payload, rx_data->ptr->len);
		temp += rx_data->ptr->len;
	} while(netbuf_next(rx_data) != -1);
	buffer[temp] = 0;
	TRACE("BUFFER: %s", buffer);
	data = strstr(buffer,"\r\n\r\n");
	if(data == 0) {
		vPortFree(buffer);
		return 0;
	}
	TRACE("DATA: %s", data);
	char *ptr;

	vPortFree(buffer);

	_S_SETTINGS_INFO *all_setng = get_all_settings();
	const char *tag_search = "opmode=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			all_setng->esp_mode = atoi(ptr);
		}
	}

	tag_search = "ssid=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(all_setng->ssid, ptr, sizeof(all_setng->ssid));
		}
	}

	tag_search = "passwd=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(all_setng->passwd, ptr, sizeof(all_setng->passwd));
		}
	}

	tag_search = "cntname=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(all_setng->controller_name, ptr, sizeof(all_setng->controller_name));
		}
	}

	uint8_t mon_mode_req = 0;
	tag_search = "monmode=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			mon_mode_req = 1;
		}
	} else {
		/*
		 * If monitor mode is disable, the monitor mode name must be removed from
		 * individual tank details and linked pair details
		 * 
		 * NOTE: Deletion from linked pair details is not tested.
		*/
		all_setng->en_mon_mod = 0;

		char data[32];
		strncpy(data, all_setng->mon_mode_name, sizeof(data));

		_S_SETTINGS_INFO *all_setng = get_all_settings();
		__TANK_SETTINGS_ *name_list = get_tank_settings();
		__TANK_SETTINGS_ new_list[5];
		__BIND_TANK_ *linkage_list = get_tank_linkage();
		__BIND_TANK_ new_linkage_list[5];
		memset(new_list, 0, sizeof(new_list));
		memset(new_linkage_list, 0, sizeof(new_linkage_list));
		uint8_t tank_num = get_tank_numbers();

		/* Deletion from individual list */
		uint8_t i = 0;
		for(uint8_t j = 0; (j < tank_num) && (name_list[j].tank_name[0] != 0); j++) {
			if(!strstr(data, name_list[j].tank_name)) {
				new_list[i] = name_list[j];
				i++;
			}
		}
		
		/* Deletion from linked list */
		i = 0;
		for(uint8_t j = 0; (j < tank_num) && ((linkage_list[j].src_tank[0] != 0) || (linkage_list[j].dst_tank[0] != 0)); j++) {
			if(!((strstr(data, linkage_list[j].src_tank)) || (strstr(data, linkage_list[j].dst_tank)))) {
				new_linkage_list[i] = linkage_list[j];
				i++;
			}
		}

		all_setng->num_of_tanks -= 1;

		memset(name_list, 0, sizeof(all_setng->tank_conf));
		memcpy(name_list, &new_list, sizeof(all_setng->tank_conf));

		memset(linkage_list, 0, sizeof(all_setng->tank_linkage));
		memcpy(linkage_list, &new_linkage_list, sizeof(all_setng->tank_linkage));
	}

	tag_search = "monname=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(all_setng->mon_mode_name, ptr, sizeof(all_setng->mon_mode_name));
		}
	}

	/* If monitor mode is requested, checks if there is name conflict and if there is space to write name
	 * before writing the name
	*/
	if(mon_mode_req) {
		_S_SETTINGS_INFO *temp_setng = get_all_settings();
		uint8_t tanks = temp_setng->num_of_tanks;
		uint8_t register_flag = 1;
		while(tanks) {
			if(!(strcmp(all_setng->mon_mode_name, temp_setng->tank_conf[tanks-1].tank_name))) {
				TRACE("Client Name Already Exists");
				register_flag = 0;
				break;
			}
			tanks--;
		}

		if(register_flag) {
			for(uint8_t i = 0; i < 5; i++) {    /*We are assuming system can handle max. 5 tanks*/
				if(temp_setng->tank_conf[i].tank_name[0] == 0) {
					strncpy(temp_setng->tank_conf[i].tank_name, all_setng->mon_mode_name, 32);
					TRACE("Name Written: %s", temp_setng->tank_conf[i].tank_name);
					temp_setng->num_of_tanks++;
					all_setng->en_mon_mod = 1;
					save_settings_to_flash();
					break;
				}
			}
		}
	}

	save_settings_to_flash();

	// save_tank_settings(tank_temp_setng);	//TODO: maye save both settings at once to reduce flash erase action



	_get_mode_config(conn, NULL);

	// webserver_webpage_chunked_data_new(temp_buffer, 1024, conn,htmlHeader_station_post);
	// webserver_webpage_chunked_data_end(conn);
	TRACE("after netconn write\n");

	// netconn_close(conn);

	// vTaskDelay(250);
	// system_restart();
	
	TRACE("At the end\n Return 1");
	return 1;

}

void mode_config_init(void) {
	webserver_add_page(GET_LINK, _get_mode_config);
	webserver_add_page(POST_LINK, _post_mode_config);
}
