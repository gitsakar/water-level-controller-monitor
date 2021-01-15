#include "esp_common.h"
#include "esp_system.h"

#include "gpio.h"

#include "debug.h"

/*
 * Function gpio_init is used to set certain pins as output where motors are connected
 * Function mon_mode_gpio_init is used if monitor mode is enabled which initializes pins for sensor
*/

/*
 * NOTE: If following output pins (for motor) are changed, the webpage should also be updated
 * pin names are written in "const char *pin_assignment"
*/ 
uint8_t water_low_pin = 5;
uint8_t water_spill_pin = 3;

void gpio_init(void) {
	uint8_t m_pin = 13;

	GPIO_ConfigTypeDef pin_config = {
		.GPIO_Pin = (BIT(m_pin)),
		.GPIO_Mode = GPIO_Mode_Output,
		.GPIO_Pullup = GPIO_PullUp_DIS,
		.GPIO_IntrType = GPIO_PIN_INTR_DISABLE,
	};

	gpio_config(&pin_config);
	GPIO_OUTPUT_SET(m_pin, true);

	m_pin = 14;
	pin_config.GPIO_Pin = (BIT(m_pin));
	gpio_config(&pin_config);
	GPIO_OUTPUT_SET(m_pin, true);

	m_pin = 4;
	pin_config.GPIO_Pin = (BIT(m_pin));
	gpio_config(&pin_config);
	GPIO_OUTPUT_SET(m_pin, true);

	m_pin = 0;
	pin_config.GPIO_Pin = (BIT(m_pin));
	gpio_config(&pin_config);
	GPIO_OUTPUT_SET(m_pin, true);

	gpio16_output_conf();
	gpio16_output_set(true);
}

void mon_mode_gpio_init(void) {

	GPIO_ConfigTypeDef pin_config = {
		.GPIO_Pin = (BIT(water_low_pin)),
		.GPIO_Mode = GPIO_Mode_Input,
		.GPIO_Pullup = GPIO_PullUp_EN,
		.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE,
	};

	gpio_config(&pin_config);

	pin_config.GPIO_Pin = (BIT(water_spill_pin));
	gpio_config(&pin_config);
}
