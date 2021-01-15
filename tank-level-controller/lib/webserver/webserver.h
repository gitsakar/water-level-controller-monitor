#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
#include "inttypes.h"
#if NO_SYS == 0
#include "lwip/tcpip.h"
#else
#include "lwip/tcp.h"
#include "rtsconn.h"
#endif

typedef uint32_t (*page_function)(struct netconn *conn, struct netbuf *rx_data);

void webserver_init(void);
void webserver_deinit(void);
uint32_t webserver_add_page(char *header, page_function func);
uint32_t webserver_http_unathorized(struct netconn *conn);
uint32_t webserver_parse_received_data(struct netconn *conn, struct netbuf *recv, char **data);
uint32_t webserver_page_not_found(struct netconn * conn);
uint32_t webserver_webpage_chunked_data_new(char * buffer, uint32_t buffer_size, struct netconn * , const char * , ...);
uint32_t webserver_webpage_chunked_data_continuation(char * buffer, uint32_t buffer_size, struct netconn * , const char * , ...);
uint32_t webserver_webpage_chunked_data_end(struct netconn *);
#endif
