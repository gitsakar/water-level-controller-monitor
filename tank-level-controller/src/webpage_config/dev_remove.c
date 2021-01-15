#include <string.h>

#include "esp_common.h"
#include "esp_system.h"

#include "lwip/api.h"

#include "webserver.h"
#include "settings/settings.h"

#include "debug.h"

#define GET_LINK				"GET /rem HTTP"
#define POST_LINK				"POST /rem HTTP"

/*
 * Webpage to remove registered devices
*/

extern void urldecode2(uint8_t *dst , const uint8_t *src);

extern const char *html_header;
extern const char *html_header1;
extern const char *html_header2;
extern const char *htmlFooter;
extern const char *ind_list;

static uint32_t _get_rem(struct netconn *conn, struct netbuf *recv) {
	char *temp_buffer;
	
	temp_buffer = pvPortMalloc(1024);
	if(temp_buffer == 0) {
		return 0;
	}

	__TANK_SETTINGS_ *crnt_setng = get_tank_settings();

	webserver_webpage_chunked_data_new(temp_buffer, 1024, conn, html_header, "Configuration");
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header1);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header2, "Status");
	
	uint8_t tanks_quan = get_tank_numbers();
	for(int i = 0; i < tanks_quan; i++) {
		webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, ind_list, crnt_setng[i].tank_name, crnt_setng[i].tank_name);
	}

	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, htmlFooter);
	webserver_webpage_chunked_data_end(conn);

	vPortFree(temp_buffer);

	return 1;
}

static uint32_t _post_rem(struct netconn *conn, struct netbuf *rx_data) {
	
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

	urldecode2(data, data);

	/*
	 * Deletion of nodes is done by creating new list of nodes with selected nodes NOT on the list
	 * Both individual tank settings[__TANK__SETTINGS_] list and tank pair[__BIND_TANK_] list are updated
	 * 
	*/

	_S_SETTINGS_INFO *all_setng = get_all_settings();
	__TANK_SETTINGS_ *name_list = get_tank_settings();
	__TANK_SETTINGS_ new_list[5];
	__BIND_TANK_ *linkage_list = get_tank_linkage();
	__BIND_TANK_ new_linkage_list[5];
	memset(new_list, 0, sizeof(new_list));
	memset(new_linkage_list, 0, sizeof(new_linkage_list));
	uint8_t tank_num = get_tank_numbers();

	if(strstr(data, all_setng->mon_mode_name)) {
		all_setng->en_mon_mod = 0;
	}

	/* Run loop until blank entry is found */
	uint8_t i = 0;
	for(uint8_t j = 0; (j < tank_num) && (name_list[j].tank_name[0] != 0); j++) {
		if(!strstr(data, name_list[j].tank_name)) {
			new_list[i] = name_list[j];
			i++;
		}
	}

	/* Loop runs upto a point where there is entry on either dst_tank or src_tank */
	i = 0;
	for(uint8_t j = 0; (j < tank_num) && ((linkage_list[j].src_tank[0] != 0) || (linkage_list[j].dst_tank[0] != 0)); j++) {
		if(!((strstr(data, linkage_list[j].src_tank)) || (strstr(data, linkage_list[j].dst_tank)))) {
			new_linkage_list[i] = linkage_list[j];
			i++;
		}
	}

	char *rm_find = data;
	uint8_t count = 0;
	const char rm_tag[] = "rmdev";
	while(rm_find != NULL) {
		if((rm_find = strstr(rm_find, "rmdev")) != NULL) {
			rm_find += strlen(rm_tag);
			count++;
		}
	}

	all_setng->num_of_tanks -= count;

	memset(name_list, 0, sizeof(all_setng->tank_conf));
	memcpy(name_list, &new_list, sizeof(all_setng->tank_conf));

	memset(linkage_list, 0, sizeof(all_setng->tank_linkage));
	memcpy(linkage_list, &new_linkage_list, sizeof(all_setng->tank_linkage));

	save_settings_to_flash();

	_get_rem(conn, NULL);
	return 1;
}

void dev_rem_init(void) {
	webserver_add_page(GET_LINK, _get_rem);
	webserver_add_page(POST_LINK, _post_rem);
}
