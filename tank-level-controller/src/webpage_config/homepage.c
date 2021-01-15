#include <string.h>

#include "esp_common.h"
#include "esp_system.h"

#include "lwip/api.h"

#include "webserver.h"
#include "settings/settings.h"

#include "debug.h"

#define GET_HOME				"GET /home HTTP"
#define POST_HOME				"POST /home HTTP"

/*
 * Homepage provides prompt to choose tank mode [src/dst/src+dst]
 * 
 * Function _update_dst_lst
 * Also after each save, destination tanks list are updated in tank linkage list [__BIND_TANKS_]
 * and remaining locations are emptied
*/

extern const char *html_header;
extern const char *html_header1;
extern const char *html_header2;
extern const char *htmlFooter;
extern const char *drop_list;
extern const char *htmlHeader_station_post;

static const char *_tank_status_name(__TANK_SETTINGS_ *stgn_ptr, uint8_t id) {
	switch(stgn_ptr[id].tank_oprn) {
		case Src:
		return "Source";

		case Dst:
		return "Destination";

		case SrcDst:
		return "Source + Destination";

		default:
		return "Please Select One";
	}
}

static void _update_dst_lst(void) {
	_S_SETTINGS_INFO *temp_setng = get_all_settings();
	uint8_t tanks_no = temp_setng->num_of_tanks;
	uint8_t j = 0;
	for(uint8_t i = 0; i < tanks_no; i++) {
		switch(temp_setng->tank_conf[i].tank_oprn) {
			case Dst:
			case SrcDst:
			strncpy(temp_setng->tank_linkage[j].dst_tank, temp_setng->tank_conf[i].tank_name, sizeof(temp_setng->tank_linkage[j].dst_tank));
			j++;
			break;

			default:
			break;
		}
	}
	while(j != tanks_no) {
		memset(temp_setng->tank_linkage[j].dst_tank, 0, sizeof(temp_setng->tank_linkage[j].dst_tank));
		memset(temp_setng->tank_linkage[j].src_tank, 0, sizeof(temp_setng->tank_linkage[j].src_tank));
		j++;
	}
	// save_bind_settings(temp_setng->tank_linkage);
	save_settings_to_flash();
}

static uint32_t _get_homepage(struct netconn *conn, struct netbuf *recv) {
	char *temp_buffer;
    
	temp_buffer = pvPortMalloc(1024);
	if(temp_buffer == 0) {
		return 0;
    }

	__TANK_SETTINGS_ *crnt_setng = get_tank_settings();

	webserver_webpage_chunked_data_new(temp_buffer, 1024, conn, html_header, "Configuration");
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header1);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header2, "Tank Mode Selection");
	
	uint8_t tanks_quan = get_tank_numbers();
	for(int i = 0; i < tanks_quan; i++) {
		char label[26];
		char tag[10];
		snprintf(label, sizeof(label), "Select Function of %s", crnt_setng[i].tank_name);
		snprintf(tag, sizeof(tag), "tank_fun%d", i);
		webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, drop_list, label, tag, _tank_status_name(crnt_setng, i));
	}
	
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, htmlFooter);
	webserver_webpage_chunked_data_end(conn);

	vPortFree(temp_buffer);

	return 1;
}

static uint32_t _post_homepage (struct netconn *conn, struct netbuf *rx_data) {

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

	__TANK_SETTINGS_ *tank_temp_setng = get_tank_settings();
	uint8_t tanks_quan = get_tank_numbers();
	for(uint8_t i = 0; i < tanks_quan; i++) {
		char tag_search[11];
		snprintf(tag_search, sizeof(tag_search), "tank_fun%d=", i);

		if((ptr = strstr(data, tag_search)) > 0) {
			ptr += strlen(tag_search);
			if((ptr1 = strchr(ptr,'&')) > 0) {
				*ptr1 = 0;
				data = ptr1 + 1;
				tank_temp_setng[i].tank_oprn = (uint8_t) atoi(ptr);
			}
		}
	}

	/* Settings are saved from _update_dst_lst */
	_update_dst_lst();


	_get_homepage(conn, NULL);

	// webserver_webpage_chunked_data_new(temp_buffer, 1024, conn,htmlHeader_station_post);
	// webserver_webpage_chunked_data_end(conn);
	TRACE("after netconn write\n");

	// netconn_close(conn);

	// vTaskDelay(250);
	// system_restart();
	
	TRACE("At the end\n Return 1");
	return 1;
}

void homepage_init(void) {
    webserver_add_page(GET_HOME, _get_homepage);
	webserver_add_page(POST_HOME, _post_homepage);
}
