#include <string.h>

#include "esp_common.h"
#include "esp_system.h"

#include "lwip/api.h"

#include "webserver.h"
#include "settings/settings.h"

#include "debug.h"

#define GET_LINK				"GET /link_tank HTTP"
#define POST_LINK				"POST /link_tank HTTP"

extern const char *html_header;
extern const char *html_header1;
extern const char *html_header2;
extern const char *htmlFooter;
extern const char *drop_list;
extern const char *tank_src_list;
extern const char *tank_src_list_dyn_1;
extern const char *tank_src_list_dyn_2;
extern const char *tank_src_list_dyn_3;
extern const char *pin_assignment;

/*
 * This page provides prompt to link dst tanks with src tanks along with motor pins associated
*/

void urldecode2(uint8_t *dst , const uint8_t *src) {
    uint8_t a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')   {
                a -= 'a'-'A';
            }
            if (a >= 'A')   {
                a -= ('A' - 10);
            }   else    {
                a -= '0';
            }
            if (b >= 'a')   {
                b -= 'a'-'A';
            }
            if (b >= 'A')   {
                b -= ('A' - 10);
            }   else    {
                b -= '0';
            }
            *dst++ = 16*a+b;
            src+=3;
        }   else if (*src == '+')   {
            *dst++ = ' ';
            src++;
        }   else    {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
    return;
}

static uint32_t _get_link_tank(struct netconn *conn, struct netbuf *recv) {
	char *temp_buffer;
    
	temp_buffer = pvPortMalloc(1024);
	if(temp_buffer == 0) {
		return 0;
    }

    __TANK_SETTINGS_ *crnt_setng = get_tank_settings();

	webserver_webpage_chunked_data_new(temp_buffer, 1024, conn, html_header, "Configuration");
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header1);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header2, "Linking Tank");
	
	__BIND_TANK_ *temp_bind = get_tank_linkage();
	uint8_t tanks_quan = get_tank_numbers();
	for(uint8_t i = 0; (i < tanks_quan) && (temp_bind[i].dst_tank[0] != 0); i++) {
		char label[60];
		char tag[10];
		char pin_tag[10];
		char cur_pin_name[10];
		snprintf(label, sizeof(label), "Select Source Tank for %s", temp_bind[i].dst_tank);
		snprintf(tag, sizeof(tag), "link_fun%d", i);
		snprintf(pin_tag, sizeof(pin_tag), "pin_tag%d", i);
		snprintf(cur_pin_name, sizeof(cur_pin_name), "Pin %d", temp_bind[i].motor_pin);
		
		webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, tank_src_list_dyn_1, label, tag, temp_bind[i].src_tank);
		for(uint8_t j = 0; j < tanks_quan; j++) {
			switch(crnt_setng[j].tank_oprn) {
				case Src:
				case SrcDst:
				webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, tank_src_list_dyn_2, crnt_setng[j].tank_name, crnt_setng[j].tank_name);
				break;

				default:
				break;
			}
		}
		webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, tank_src_list_dyn_3);
		webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, pin_assignment, pin_tag, cur_pin_name);
	}
	
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, htmlFooter);
	webserver_webpage_chunked_data_end(conn);

	vPortFree(temp_buffer);

	return 1;
}

static uint32_t _post_link_tank(struct netconn *conn, struct netbuf *rx_data) {
	
	char *data, *ptr1;
	uint32_t temp = 0;

	struct netbuf *ptr_temp = 0;
	char *buffer = 0;


	TRACE ("Link Tank POST called");
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

	_S_SETTINGS_INFO *temp_setng = get_all_settings();
	uint8_t tanks_quan = temp_setng->num_of_tanks;
	for(uint8_t i = 0; (i < tanks_quan) && (temp_setng->tank_linkage[i].dst_tank[0] != 0) ; i++) {
		char tag_search[11];
		snprintf(tag_search, sizeof(tag_search), "link_fun%d=", i);

		char pin_tag_search[10];
		snprintf(pin_tag_search, sizeof(pin_tag_search), "pin_tag%d=", i);

		if((ptr = strstr(data,tag_search)) > 0) {
			ptr += strlen(tag_search);
			if((ptr1 = strchr(ptr,'&')) > 0) {
				*ptr1 = 0;
				data = ptr1 + 1;
				urldecode2(ptr, ptr);
				strncpy(temp_setng->tank_linkage[i].src_tank, ptr, 32);
			}
		}

		if((ptr = strstr(data,pin_tag_search)) > 0) {
			ptr += strlen(pin_tag_search);
			if((ptr1 = strchr(ptr,'&')) > 0) {
				*ptr1 = 0;
				data = ptr1 + 1;
				temp_setng->tank_linkage[i].motor_pin = atoi(ptr);
			}
		}

		temp_setng->tank_linkage[i].motor_state = true;
	}

	save_settings_to_flash();


	vTaskDelay(10);

	_get_link_tank(conn, NULL);

	// webserver_webpage_chunked_data_new(temp_buffer, 1024, conn,htmlHeader_station_post);
	// webserver_webpage_chunked_data_end(conn);
	TRACE("after netconn write\n");

	// netconn_close(conn);

	// vTaskDelay(250);
	// system_restart();
	
	TRACE("At the end\n Return 1");
	return 1;
}

void link_tank_init(void) {
    webserver_add_page(GET_LINK, _get_link_tank);
	webserver_add_page(POST_LINK, _post_link_tank);
}
