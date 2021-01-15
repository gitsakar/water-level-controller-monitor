#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/api.h"

#include "settings/settings.h"
#include "debug.h"

/*
 * This task is started if there is change in water level, and sends the data to
 * the controller
*/

extern ip_addr_t cont_ip;

static uint8_t _receive_data_ack(struct netconn *conn) {
	uint8_t rc = 1;

	struct netbuf *inbuf;
	
	char *buf;
	u16_t len;
	
	err_t err = netconn_recv(conn, &inbuf);

	if(err == ERR_OK) {
		netbuf_data(inbuf, (void **)&buf, &len);
		if(!strcmp(buf, "APP_ACK")) {
			rc = 0;
		}
	}
	netbuf_delete(inbuf);

	return rc;
}

void send_data(void *pvParameters) {
	char *level = (char *) pvParameters;

	_S_SETTINGS_INFO *setng = get_all_settings();

	char sensor_msg[150];
	snprintf(sensor_msg, sizeof(sensor_msg), "Controller_Name=%s&Client_Name=%s&Level_State=%s&", setng->cont_name, setng->node_name, level);
	
	struct netconn *conn;
	
	uint8_t app_ack = 1;
	uint8_t retries = 30;
	while((app_ack) && (retries)) {
		--retries;

		conn = netconn_new(NETCONN_TCP);
	
		err_t rc1 = netconn_bind(conn, IP_ADDR_ANY, 0);
		conn->recv_timeout = 5000;
		conn->send_timeout = 1000;

		err_t rc2 = netconn_connect(conn, &cont_ip, 2222);

		if(rc1 != ERR_OK || rc2 != ERR_OK) {
			netconn_delete(conn);
			continue;
		}

		vTaskDelay(10/portTICK_RATE_MS);
		
		netconn_write(conn, sensor_msg, strlen(sensor_msg), NETCONN_COPY);
		app_ack = _receive_data_ack(conn);

		netconn_close(conn);
		netconn_delete(conn);
	}

	wifi_station_disconnect();
	system_deep_sleep(0);

	vTaskDelete(NULL);
}
