#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/api.h"

#include "settings/settings.h"

#include "debug.h"

/*
 * prcs_sen_data receives the data sent by node, sends ack on reception of proper data
 * and starts controller task to decide upon the action required
*/

extern void controller(void *pvParameters);

char recv_node_name[32];
uint8_t new_node_state;

static void _process(struct netconn *new_conn) {
	new_conn->recv_timeout = 5000;
	new_conn->send_timeout = 1000;

	struct netbuf *inbuf = NULL;
	char *ptr, *ptr_end;

	char *buf;
	u16_t len;

	err_t err = netconn_recv(new_conn, &inbuf);

	char recv_cont_name[32];

	if(err == ERR_OK) {
		netbuf_data(inbuf, (void **)&buf, &len);

		if((ptr = strstr(buf, "Controller_Name="))) {
			ptr += 16;
			if((ptr_end = strchr(buf, '&'))) {
				*ptr_end = 0;
				buf = ptr_end + 1;
				TRACE("Controller Name: %s", ptr);
				strncpy(recv_cont_name, ptr, sizeof(recv_cont_name));
			}

			if((ptr = strstr(buf, "Client_Name="))) {
				ptr += 12;
				if((ptr_end = strchr(buf, '&'))) {
					*ptr_end = 0;
					buf = ptr_end + 1;
					TRACE("Client Name: %s", ptr);
					strncpy(recv_node_name, ptr, sizeof(recv_node_name));
				}
				
				if((ptr = strstr(buf, "Level_State="))) {
					ptr += 12;
					if((ptr_end = strchr(buf, '&'))) {
						*ptr_end = 0;
						buf = ptr_end + 1;
						new_node_state = (uint8_t) atoi(ptr);
					}

					vTaskDelay(10/portTICK_RATE_MS);
					netconn_write(new_conn, "APP_ACK", 7, NETCONN_NOCOPY);

				}
			}
		}
		
		/*Start Task only after suitable condition*/
		if(!(strcmp(recv_cont_name, get_controller_name()))) {
			xTaskCreate(controller, "Motor Controller", 176, NULL, 1, NULL);
		}
	}
	netbuf_delete(inbuf);
}

void prcs_sen_data(void *pvParameters) {
	struct netconn *prcs_conn = netconn_new(NETCONN_TCP);
	netconn_bind(prcs_conn, IP_ADDR_ANY, 2222);
	
	prcs_conn->recv_timeout = 0;
	prcs_conn->send_timeout = 5000/1;

	netconn_listen(prcs_conn);

	while (1) {
		struct netconn *new_conn = NULL;
		netconn_accept(prcs_conn, &new_conn);

		if(new_conn) {
			TRACE("accepted");
			_process(new_conn);
		}

		netconn_close(new_conn);
		netconn_delete(new_conn);

		vTaskDelay(20/portTICK_RATE_MS);
	}
}
