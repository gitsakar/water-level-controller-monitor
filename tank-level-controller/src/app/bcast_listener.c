#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/api.h"

#include "settings/settings.h"
#include "debug.h"

/*
 * bcast_lsn task continuously listens for broadcast messages sent by nodes
 * and replies if controller name is matched so as to let the node know
 * controller's ip address
*/
void bcast_lsn(void *pvParameters) {
	_S_SETTINGS_INFO *all_setng = get_all_settings();

	char dis_req[50];
	char dis_rsp[50];

	snprintf(dis_req, sizeof(dis_req), "ESP SEEK:%s", all_setng->controller_name);
	snprintf(dis_rsp, sizeof(dis_rsp), "ESP SHOW:%s", all_setng->controller_name);

	struct netconn *conn;
	struct netbuf *buf;
	char *data;
	u16_t len;

	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, 5555);

	while(1) {
		err_t err_rcv = netconn_recv(conn, &buf);
		if(err_rcv == ERR_OK) {
			netbuf_data(buf, (void **)&data, &len);

			if(!strcmp(data, dis_req)) {
				data = netbuf_alloc(buf, sizeof(dis_rsp));
				strncpy(data, dis_rsp, sizeof(dis_rsp));
				
				netconn_send(conn, buf);
			}
		}
	}
}
