#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "webserver.h"

#include "debug.h"

extern void homepage_init(void);
extern void link_tank_init(void);
extern void status_init(void);
extern void mode_config_init();

/*
 * Webpages for configuration are initialized from this task
*/

void webpage_task(void *pvParameters) {
    webserver_init();
    homepage_init();
    link_tank_init();
    mode_config_init();
    status_init();
    dev_rem_init();

	do {
		vTaskDelay((5*60*1000)/portTICK_RATE_MS);
		// system_restart();
	} while (1);
}
