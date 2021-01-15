#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/api.h"

#include "settings/settings.h"

#include "debug.h"

/* 
 * reg_service task registers new nodes
 * 
 * nodes names must be unique in order to be registered
 * 
 * Currently cannot server multiple thread at once,
 * serves one thread at a time
*/
static void _serve(struct netconn *srv_conn) {
	srv_conn->recv_timeout = 5000;
	srv_conn->send_timeout = 1000;

	struct netbuf *inbuf = NULL;
	char *buf;
	u16_t len;

	err_t err = netconn_recv(srv_conn, &inbuf);

	char *ptr, *ptr_end;

	char recv_cont_name[32];
	char recv_client_name[32];

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
					strncpy(recv_client_name, ptr, sizeof(recv_client_name));
				}

				vTaskDelay(10/portTICK_RATE_MS);
				netconn_write(srv_conn, "APP_ACK", 7, NETCONN_NOCOPY);

			}
		}
		
		if(!(strcmp(recv_cont_name, get_controller_name()))) {
			TRACE("Controller name matched\n");
			_S_SETTINGS_INFO *temp_setng = get_all_settings();
			uint8_t tanks = temp_setng->num_of_tanks;
			uint8_t register_flag = 1;
			while(tanks) {
				if(!(strcmp(recv_client_name, temp_setng->tank_conf[tanks-1].tank_name))) {
					TRACE("Client Name Already Exists");
					register_flag = 0;
					break;
				}
				tanks--;
			}

			if(register_flag) {
				for(uint8_t i = 0; i < 5; i++) {    /*We are assuming system can handle max. 5 tanks*/
					if(temp_setng->tank_conf[i].tank_name[0] == 0) {
						strncpy(temp_setng->tank_conf[i].tank_name, recv_client_name, 32);
						TRACE("Name Written: %s", temp_setng->tank_conf[i].tank_name);
						temp_setng->num_of_tanks++;
						save_settings_to_flash();
						break;
					}
				}
			}
		}
	}
	netbuf_delete(inbuf);
	TRACE("leaving register service");
}

void reg_service(void *pvParameters) {
	struct netconn *reg_conn = netconn_new(NETCONN_TCP);
	netconn_bind(reg_conn, IP_ADDR_ANY, 4444);
	
	reg_conn->recv_timeout = 0;
	reg_conn->send_timeout = 5000/1;

	netconn_listen(reg_conn);

	while (1) {
		struct netconn *new_conn = NULL;
		netconn_accept(reg_conn, &new_conn);

		if(new_conn) {
			TRACE("accepted");
			_serve(new_conn);
		}

		netconn_close(new_conn);
		netconn_delete(new_conn);

		vTaskDelay(200/portTICK_RATE_MS);
	}
}
