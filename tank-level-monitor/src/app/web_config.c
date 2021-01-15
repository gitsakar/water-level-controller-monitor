#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "webserver.h"

#include "debug.h"

/* 
 * Initializes webpage for configuration of device
 * Node goes to deep sleep in 5 minutes after initialization 
 */
extern void homepage_init(void);

void config_task(void *pvParameters) {
	webserver_init();
	homepage_init();

	vTaskDelay((5*60*1000)/portTICK_RATE_MS);
	system_deep_sleep(0);
	
	vTaskDelete(NULL);
}
