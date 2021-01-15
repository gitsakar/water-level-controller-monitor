#include <string.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "webserver.h"
#include "debug.h"

struct webserver_page_info {
	char *page_header;
	page_function func;
	struct webserver_page_info *next;
};

struct webserver_page_info *page_info = 0;
static xTaskHandle listen_task = 0;

#define HEADER_END		"\r\n\r\n"

static void
_receive(void *pv) {
	TRACE("inside recv\n");
	struct netconn *conn = pv;
	char *ptr = 0;
	struct netbuf *recv = 0;
	uint32_t len = 0;

	conn->recv_timeout = 5000;
	conn->send_timeout = 1000;

	do {
		struct netbuf * temp = 0;
		err_t err;

		err = netconn_recv(conn, &temp);
		if(err != ERR_OK || temp == 0) {
			TRACE("_receive, netconn_recv, temp:%x, err:%d", temp, err);
			break;
		}
		do {
			if((ptr = strstr(temp->ptr->payload, "\r\n\r\n")) > 0) {
				break;
			}
		} while(netbuf_next(temp) != -1);
		netbuf_first(temp);
		if(recv == 0) {
			recv = temp;
		} else {
			netbuf_chain(recv, temp);
		}
		conn->recv_timeout = 200/1;
	} while(ptr == 0);

	len = 0;
	if(ptr) {
		struct webserver_page_info *w = page_info;

		TRACE("Header end found");
		while(w) {
			netbuf_first(recv);
			do {
				if(strstr(recv->ptr->payload,w->page_header)) {
					TRACE("Match found:%s",w->page_header);
					netbuf_first(recv);
					w->func(conn,recv);
					len = 1;
					break;
				}
			} while(netbuf_next(recv) != -1);
			if(len == 1) {
				break;
			}
			w = w->next;
		}
		if(len == 0) {
			webserver_page_not_found(conn);
		}
	}
	netbuf_delete(recv);
	netconn_close(conn);
	netconn_delete(conn);
//	DISPLAY_HIGHWATER_MARK();
	vTaskDelete(NULL);
}

static void
_webserver_listen(void *pv) {
	err_t err;
	struct netconn *listen_conn;
	listen_conn = netconn_new(NETCONN_TCP);
	if(listen_conn == 0) {
		TRACE("Could not get new connection for netcon tcp");
		vTaskDelete(NULL);
	}
	netconn_bind(listen_conn,NULL,80);
	listen_conn->recv_timeout = 0;
	listen_conn->send_timeout = 5000/1;

	err = netconn_listen(listen_conn);
	if(err) {
		TRACE("Could not inititate listen for webserver:%d",err);
		netconn_delete(listen_conn);
		vTaskDelete(NULL);
	}
	do {
		struct netconn *conn = 0;
		err_t err = netconn_accept(listen_conn, &conn);
		TRACE("after accept\n");
		if(conn) {
			if(xTaskCreate(_receive,"W Receive",512,conn,1,NULL) != pdPASS) {
				TRACE("after recv task\n");
				TRACE("Could not create receive process for webserver");
				netconn_close(conn);
				netconn_delete(conn);
			}
		}
	} while(1);
}

void
webserver_init() {
	page_info = 0;
	listen_task = 0;
	if(xTaskCreate(_webserver_listen,"listen",200,NULL,2, &listen_task) != pdPASS) {
		TRACE("Could not create webserver listen task");
		vTaskDelay(20);
		return;
	}
}

void
webserver_deinit() {
	struct webserver_page_info * temp = page_info;

	while(temp) {
		struct webserver_page_info *next = temp->next;
		vPortFree(temp);
		temp = next;
	}
	if(listen_task) {
		vTaskDelete(listen_task);
		listen_task = 0;
	}

}

uint32_t
webserver_add_page(char *header, page_function func) {
	struct webserver_page_info * w = pvPortMalloc(sizeof(struct webserver_page_info));

	if(w == 0) {
		return 0;
	}
	w->page_header = header;
	w->func = func;
	w->next = 0;

	if(page_info == 0) {
		vTaskSuspendAll();
		page_info = w;
		xTaskResumeAll();
	} else {
		struct webserver_page_info *last = page_info;

		while(last->next) {
			last = last->next;
		}
		vTaskSuspendAll();
		last->next = w;
		xTaskResumeAll();
	}
	return 1;
}

uint32_t
webserver_http_unathorized(struct netconn *conn) {
	err_t err;
	const char unathorized_header_data[] = "HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nContent-Length: 0\r\n"
													 								"Content-Type: text/html\r\nWWW-Authenticate: Basic\r\n\r\n";

	err = netconn_write(conn,unathorized_header_data,sizeof(unathorized_header_data) - 1,NETCONN_COPY);
	if(err) {
		TRACE("Could not write page not found:%d",err);
	}
	return 1;
}

uint32_t
webserver_page_not_found(struct netconn * conn) {
	const char page_data[] = "<html><body bgcolor=\"white\"><center><h1>Page Not Available Matey!</h1></center></body></html>";
	const char header_data[] = "HTTP/1.1 404 Not Found\r\n""Content-Length: %d\r\n""Content-Type: text/html\r\n\r\n";
	err_t err;
	char * ptr = pvPortMalloc(sizeof(page_data) + sizeof(header_data) + 10);		
	if(ptr) {
		uint32_t len = sprintf(ptr,header_data,sizeof(page_data) - 1);
		len += sprintf(&ptr[len],page_data);
		err = netconn_write(conn,ptr,len,NETCONN_COPY);
		if(err) {
			TRACE("Could not write page not found:%d",err);
		}
		vPortFree(ptr);
	}
	return 1;
}

uint32_t
webserver_parse_received_data(struct netconn *conn, struct netbuf *recv, char **data_ptr) {
	char *header_end,*ptr;
	uint32_t content_length;
	uint32_t received_length;

	do {
		header_end = strstr(recv->ptr->payload,"\r\n\r\n");
		if(header_end) {
			break;
		}
	} while(netbuf_next(recv) != -1);
	if(header_end == 0) {
		return 0;
	}
	netbuf_first(recv);
	do {
		ptr = strstr(recv->ptr->payload,"Content-Length: ");
		if(ptr) {
			break;
		}
	} while(netbuf_next(recv) != -1);
	if(ptr == 0 || ptr > header_end) {
		return 0;
	}
	ptr += sizeof("Content-Length: ") - 1;
	content_length = strtoul(ptr,0,10);
	
	ptr = pvPortMalloc(content_length);
	if(ptr == 0) {
		return 0;
	}
	do {
		header_end = strstr(recv->ptr->payload,"\r\n\r\n");
		if(header_end) {
			header_end += 4;
			recv->ptr->len -= (header_end - (char *)recv->ptr->payload);
			recv->ptr->payload = header_end;
			break;
		}
	} while(netbuf_next(recv) != -1);
	received_length = 0;
	do {
		memcpy(&ptr[received_length],recv->ptr->payload,recv->ptr->len);
		received_length += recv->ptr->len;
	} while(netbuf_next(recv) != -1);
	if(received_length < content_length) {
		struct netbuf *temp = 0;
		err_t err = ERR_OK;
		
		do {
			temp = 0;
			err = netconn_recv(conn, &temp);
			if(temp > 0 && err == 0) {
				do {
					if(received_length + temp->ptr->len > content_length) {
						vPortFree(ptr);
						return 0;
					}
					memcpy(&ptr[received_length],temp->ptr->payload,temp->ptr->len);
					received_length += temp->ptr->len;
				} while(netbuf_next(temp) != -1);
				netbuf_delete(temp);
			} else {
				TRACE("webserver_parse_received_data, netconn_recv, err:%d", err);
				break;
			}
		} while(err == ERR_OK);
	}
	*data_ptr = ptr;
	return received_length;
}

uint32_t
webserver_webpage_chunked_data_new(char * buffer, uint32_t buffer_size, struct netconn * conn, const char * ptr, ...) {
	va_list args;
	err_t err;
	const char header_chunked[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\n\r\n";
	uint32_t data_len;

	err = netconn_write(conn, header_chunked, sizeof(header_chunked) - 1, NETCONN_COPY);
	if(err) {
		return 0;
	}

	va_start(args, ptr);
	data_len = vsnprintf(&buffer[6], buffer_size - 8, ptr, args);
	va_end(args);

	if(data_len > buffer_size - 8) {
//		ASSERT(0);
	}
	sprintf(buffer, "%04x", data_len);
	memcpy(&buffer[4], "\r\n", 2);
	memcpy(&buffer[data_len + 6], "\r\n", 2);
	err = netconn_write(conn, buffer, data_len + 6 + 2, NETCONN_COPY);
	if(err) {
		return 0;
	}
	return 1;
}

uint32_t
webserver_webpage_chunked_data_continuation(char * buffer, uint32_t buffer_size, struct netconn * conn, const char * ptr, ...) {
	va_list args;
	err_t err;
	uint32_t data_len;

	va_start(args, ptr);
	data_len = vsnprintf(&buffer[6], buffer_size - 8, ptr, args);
	TRACE("data_len:%d", data_len);
	va_end(args);

	if(data_len > buffer_size - 8) {
	}
	sprintf(buffer, "%04x", data_len);
	memcpy(&buffer[4], "\r\n", 2);
	memcpy(&buffer[data_len + 6], "\r\n", 2);
	err = netconn_write(conn, buffer, data_len + 6 + 2, NETCONN_COPY);
	if(err) {
		return 0;
	}
	return 1;
}

uint32_t
webserver_webpage_chunked_data_end(struct netconn * conn) {
	if(netconn_write(conn, "0000\r\n\r\n", 8, NETCONN_COPY) != ERR_OK) {
		return 0;
	}
	return 1;
}
