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
 * Webpage to configure device like controller name, node name, ssid, password etc.
*/

extern const char *html_header;
extern const char *html_header1;
extern const char *html_header2;
extern const char *htmlFooter;
extern const char *config_boxes;
extern const char *check_box;

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

static uint32_t _get_homepage(struct netconn *conn, struct netbuf *recv) {
	char *temp_buffer;
    
	temp_buffer = pvPortMalloc(1024);
	if(temp_buffer == 0) {
		return 0;
    }

	_S_SETTINGS_INFO *setng = get_all_settings();

	webserver_webpage_chunked_data_new(temp_buffer, 1024, conn, html_header, "Configuration");
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header1);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn ,html_header2, "Configure Node");

	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, config_boxes, setng->ssid, setng->passwd, setng->cont_name, setng->node_name);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, check_box);

	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, htmlFooter);
	webserver_webpage_chunked_data_end(conn);

	vPortFree(temp_buffer);

	return 1;
}

static uint32_t _post_homepage(struct netconn *conn, struct netbuf *rx_data) {
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

	//Processing of data not done yet

	_S_SETTINGS_INFO *setng = get_all_settings();
	
	const char *tag_search = "ssid=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(setng->ssid, ptr, sizeof(setng->ssid));
		}
	}

	tag_search = "passwd=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(setng->passwd, ptr, sizeof(setng->passwd));
		}
	}

	tag_search = "conname=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(setng->cont_name, ptr, sizeof(setng->cont_name));
		}
	}

	tag_search = "nodname=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			strncpy(setng->node_name, ptr, sizeof(setng->node_name));
		}
	}

	tag_search = "unreg=";
	if((ptr = strstr(data, tag_search)) > 0) {
		ptr += strlen(tag_search);
		if((ptr1 = strchr(ptr,'&')) > 0) {
			*ptr1 = 0;
			data = ptr1 + 1;
			urldecode2(ptr, ptr);
			setng->is_registered = !atoi(ptr);
		}
	}

	save_settings_to_flash();

	_get_homepage(conn, NULL);

	// netconn_close(conn);
	system_restart();
	
	return 1;
}

void homepage_init(void) {
    webserver_add_page(GET_HOME, _get_homepage);
	webserver_add_page(POST_HOME, _post_homepage);
}
