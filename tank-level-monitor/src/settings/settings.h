#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string.h>
#include "esp_common.h"

enum {
	WEB_MODE = 0b11,
	WATER_LOW = 0b01,
	WATER_HIGH = 0b10,
};

typedef struct {
	uint32_t magic;
	char ssid[32];
	char passwd[64];
	char cont_name[32];
	char node_name[32];
	uint8_t is_registered;
}__attribute__((aligned(4))) _S_SETTINGS_INFO;

void save_settings_to_flash();
_S_SETTINGS_INFO *get_all_settings(void);

void settings_init(void);

#endif
