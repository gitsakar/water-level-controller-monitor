#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/api.h"

#include "settings/settings.h"
#include "debug.h"

/*
 * Node sends broadcast message at the beginning expecting a reply from controller
 * If node receives a reply from controller within retries, either registration
 * or data sending task is initiated else node goes to deep sleep
*/

extern void send_data(void *pvParameters);
extern void reg(void *pvParameters);

extern uint8_t sensor_state;

ip_addr_t cont_ip;

static uint8_t _is_wifi_connected(void) {
	uint8_t retries = 30;

	while (retries) {
		STATION_STATUS status = wifi_station_get_connect_status();
		TRACE("%s: status = %d\n\r", __func__, status);

		switch(status) {
			case STATION_WRONG_PASSWORD:
			TRACE("WiFi: wrong password");
			break;

			case STATION_NO_AP_FOUND:
			TRACE("WiFi: AP not found");
			break;

			case STATION_CONNECT_FAIL:
			TRACE("WiFi: connection failed");
			break;

			case STATION_GOT_IP:
			TRACE("WiFi: Connected");
			return 1;
		}
		vTaskDelay(1000/portTICK_RATE_MS);
		--retries;
	}

	wifi_station_disconnect();
	system_deep_sleep(0);
	vTaskDelete(NULL);
}

void sensor_core_function(uint8_t sensor_state) {
	_S_SETTINGS_INFO *setng = get_all_settings();

	if(setng->is_registered) {
		switch(sensor_state) {
			case WATER_LOW:
			xTaskCreate(send_data, "Send Data Low", 512, "0", 1, NULL);
			break;

			case WATER_HIGH:
			xTaskCreate(send_data, "Send Data Full", 512, "1", 1, NULL);
			break;

			default:
			system_deep_sleep(0);
			break;
		}
	} else {
		//start registration task
		xTaskCreate(reg, "Register", 512, NULL, 1, NULL);
	}
}

void bcast_snd(void *pvParameters) {
	_is_wifi_connected();
	struct ip_info ipconfig;

	_S_SETTINGS_INFO *all_setng = get_all_settings();

	char dis_req[50];
	char dis_rsp[50];

	snprintf(dis_req, sizeof(dis_req), "ESP SEEK:%s", all_setng->cont_name);
	snprintf(dis_rsp, sizeof(dis_rsp), "ESP SHOW:%s", all_setng->cont_name);

	struct netconn *conn;
	struct netbuf *buf;
	char *data;

	u16_t len;

	conn = netconn_new(NETCONN_UDP);
	conn->recv_timeout = 5000;

	netconn_bind(conn, IP_ADDR_ANY, 5555);

	uint8_t retries = 30;
	while(retries) {
		--retries;
		
		wifi_get_ip_info(STATION_IF, &ipconfig);

		ip_addr_t bcast_ip = {
			.addr = (~(ipconfig.netmask.addr) | (ipconfig.ip.addr)),
		};

		buf = netbuf_new();

		data = netbuf_alloc(buf, sizeof(dis_req));
		strncpy(data, dis_req, sizeof(dis_req));
		buf->addr.ip4 = bcast_ip;
		buf->port = 5555;

		netconn_send(conn, buf);
		
		err_t err_rcv = netconn_recv(conn, &buf);
		if(err_rcv == ERR_OK) {
			netbuf_data(buf, (void **)&data, &len);

			if(!strcmp(data, dis_rsp)) {
				cont_ip = buf->addr.ip4;
				netbuf_delete(buf);
				netconn_delete(conn);
				break;
			}
		}

		netbuf_delete(buf);
		vTaskDelay(5000/portTICK_RATE_MS);
	}

	if(!retries) {
		wifi_station_disconnect();
		system_deep_sleep(0);
	}

	vTaskDelete(NULL);
	sensor_core_function(sensor_state);
}
