#include <string.h>

#include "esp_common.h"
#include "esp_system.h"

#include "lwip/api.h"

#include "webserver.h"
#include "settings/settings.h"

#include "debug.h"

#define GET_LINK				"GET /status HTTP"
#define POST_LINK				"POST /status HTTP"

/* Displays the current status of different tanks */

extern const char *html_header;
extern const char *html_header1;
extern const char *html_header2;
extern const char *htmlFooter;
extern const char *drop_list;
extern const char *tank_src_list;
extern const char *tank_stat;

static const char *_val_to_fun(uint8_t val) {
	switch(val) {
		case Src:
		return "Source";
		break;

		case SrcDst:
		return "Source + Destination";
		break;

		case Dst:
		return "Destination";
		break;

		default:
		return "Unknown";
		break;
	}
}

static const char *_val_to_level(uint8_t val) {
	switch(val) {
		case Low:
		return "Low";
		break;
		
		case Full:
		return "Full";
		break;

		default:
		return "Unknown";
		break;
	}
}

static uint32_t _get_status(struct netconn *conn, struct netbuf *recv) {
    char *temp_buffer;
    
	temp_buffer = pvPortMalloc(1024);
	if(temp_buffer == 0) {
		return 0;
    }


	webserver_webpage_chunked_data_new(temp_buffer, 1024, conn, html_header, "Configuration");
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header1);
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, html_header2, "Status");
	
	__TANK_SETTINGS_ *tank_list = get_tank_settings();
	uint8_t tanks_quan = get_tank_numbers();
	for(uint8_t i = 0; (i < tanks_quan) && (tank_list[i].tank_name != 0); i++) {
		webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, tank_stat, tank_list[i].tank_name, _val_to_fun(tank_list[i].tank_oprn), _val_to_level(tank_list[i].tank_state));
	}
	
	webserver_webpage_chunked_data_continuation(temp_buffer, 1024, conn, htmlFooter);
	webserver_webpage_chunked_data_end(conn);

	vPortFree(temp_buffer);

	return 1;
}

static uint32_t _post_status(struct netconn *conn, struct netbuf *recv) {
	_get_status(conn, NULL);
	return 1;
}

void status_init(void) {
    webserver_add_page(GET_LINK, _get_status);
	webserver_add_page(POST_LINK, _post_status);
}
