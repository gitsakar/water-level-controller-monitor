#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string.h>
#include "esp_common.h"

enum {
	Src = 1,
	Dst,
	SrcDst,
};

enum {
	Low = 0,
	Full,
};

enum {
	STANDAlONE_MODE = 1,
	ROUTER_MODE,
	MQTT_MODE,
};

typedef struct {
	char tank_name[32];
	uint8_t tank_oprn;
	uint8_t tank_state;
}__attribute__((aligned(4))) __TANK_SETTINGS_;

typedef struct {
	char dst_tank[32];
	char src_tank[32];
	uint8_t motor_pin;
	uint8_t motor_state;
}__attribute__((aligned(4))) __BIND_TANK_;

typedef struct {
	uint32_t magic;
	uint8_t num_of_tanks;
	char controller_name[32];
	uint8_t en_mon_mod;
	char mon_mode_name[32];
	uint8_t esp_mode;
	char ssid[32];
	char passwd[64];
	__TANK_SETTINGS_ tank_conf[5];
	__BIND_TANK_ tank_linkage[5];
}__attribute__((aligned(4))) _S_SETTINGS_INFO;


void save_settings_to_flash();


_S_SETTINGS_INFO *get_all_settings(void);
__TANK_SETTINGS_ *get_tank_settings(void);
__BIND_TANK_ *get_tank_linkage(void);
uint8_t get_tank_numbers(void);
char *get_controller_name(void);

void settings_init(void);

#endif	/*ifndef SETTINGS_H_*/
