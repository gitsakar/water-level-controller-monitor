#include "esp_common.h"
#include "esp_system.h"

#include "gpio.h"

#include "settings/settings.h"
#include "debug.h"

/*
 * Initializes GPIO and reads their status
*/
uint8_t gpio_status(void) {
	uint8_t water_spill_pin = 5;
	uint8_t water_low_pin = 4;
	uint8_t webpage_configure_pin = 14;

	uint8_t up = 0;
	uint8_t bottom = 0;
	uint8_t web_page = 0;

	GPIO_ConfigTypeDef pin_config = {
		.GPIO_Pin = (BIT(water_spill_pin)),
		.GPIO_Mode = GPIO_Mode_Input,
		.GPIO_Pullup = GPIO_PullUp_DIS,
		.GPIO_IntrType = GPIO_PIN_INTR_DISABLE,
	};

	gpio_config(&pin_config);
	up = (~(gpio_input_get()>>water_spill_pin)) & (BIT(0));

	pin_config.GPIO_Pin = (BIT(water_low_pin));
	gpio_config(&pin_config);
	bottom = (~(gpio_input_get()>>water_low_pin)) & (BIT(0));
	
	pin_config.GPIO_Pin = (BIT(webpage_configure_pin));
	gpio_config(&pin_config);
	web_page = (~(gpio_input_get()>>webpage_configure_pin)) & (BIT(0));

	if(web_page) {
		TRACE("return webpage mode 0b11");
		return WEB_MODE;
	} else {
		TRACE("return up:%d bottom:%d", up, bottom);
		return ((up<<1)|(bottom));
	}
}
