/*
 * Functions used to Save and Retrieve data from Flash
 * 
 */
#include "esp_common.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "settings.h"
#include "debug.h"

#define SHIFT_TO_SECTOR			12			/*For Flash of sector size 4KB*/

static const uint32_t settings_address = 0x00070000UL;
static const uint32_t settings_magic = 0xaaaaaaaaUL;

static _S_SETTINGS_INFO settings_info;

static void _save_settings_info(_S_SETTINGS_INFO *saveInfo, uint32_t restart) {
	SpiFlashOpResult flash_result;
	flash_result = spi_flash_erase_sector(settings_address >> SHIFT_TO_SECTOR);
	if(flash_result == SPI_FLASH_RESULT_OK) {
		TRACE("erased");
	} else {
		TRACE("erase failed!");
	}
	// vTaskDelay(100);
	TRACE("SPI flash write status :%d", spi_flash_write(settings_address, (uint32_t *)saveInfo, sizeof(*saveInfo)));

	if(restart) {
		TRACE("Device Restarting");
		vTaskDelay(50);
		system_restart();
	}
}

void save_settings_to_flash() {
	_save_settings_info(&settings_info, 0);
}

static void _get_settings_info(_S_SETTINGS_INFO *readInfo) {
	SpiFlashOpResult status;

	status = spi_flash_read(settings_address, (uint32_t *)readInfo, sizeof(*readInfo));
	TRACE("_Settings read, SPI_STATUS: %d", status);
	TRACE("TANK 1: %d", settings_info.tank_conf[0].tank_oprn);
	TRACE("TANK 2: %d", settings_info.tank_conf[1].tank_oprn);
	TRACE("TANK 3: %d", settings_info.tank_conf[2].tank_oprn);
	TRACE("TANK 4: %d", settings_info.tank_conf[3].tank_oprn);
	TRACE("TANK 5: %d", settings_info.tank_conf[4].tank_oprn);

	TRACE("=====================");
	TRACE("Destination Tanks");
	TRACE("Dst 1: %s", settings_info.tank_linkage[0].dst_tank);
	TRACE("Dst 2: %s", settings_info.tank_linkage[1].dst_tank);
	TRACE("Dst 3: %s", settings_info.tank_linkage[2].dst_tank);
	TRACE("Dst 4: %s", settings_info.tank_linkage[3].dst_tank);
	TRACE("Dst 5: %s", settings_info.tank_linkage[4].dst_tank);

	TRACE("=====================");
	TRACE("Linked Source Tanks");
	TRACE("Src 1: %s", settings_info.tank_linkage[0].src_tank);
	TRACE("Src 2: %s", settings_info.tank_linkage[1].src_tank);
	TRACE("Src 3: %s", settings_info.tank_linkage[2].src_tank);
	TRACE("Src 4: %s", settings_info.tank_linkage[3].src_tank);
	TRACE("Src 5: %s", settings_info.tank_linkage[4].src_tank);

	TRACE("no. of tanks: %d", settings_info.num_of_tanks);

}

_S_SETTINGS_INFO *get_all_settings(void) {
	return &settings_info;
}

__TANK_SETTINGS_ *get_tank_settings(void) {
	return settings_info.tank_conf;
}

__BIND_TANK_ *get_tank_linkage(void) {
	return settings_info.tank_linkage;
}

uint8_t get_tank_numbers(void) {
	return settings_info.num_of_tanks;
}

char *get_controller_name(void) {
	return settings_info.controller_name;
}

void settings_init(void) {
	_get_settings_info(&settings_info);
	TRACE("after read");
	if(settings_info.magic == settings_magic){
		TRACE("magic found");
	} else {
		memset(&settings_info, 0, sizeof(_S_SETTINGS_INFO));

		settings_info.magic = settings_magic;
		settings_info.esp_mode = STANDAlONE_MODE;
		strncpy(settings_info.ssid, "ESP_STANDALONE", sizeof(settings_info.ssid));
		strncpy(settings_info.passwd, "aaaaaaaa", sizeof(settings_info.passwd));
		
		settings_info.num_of_tanks = 5;

		strncpy(settings_info.controller_name, "MController", 32);

		snprintf(settings_info.tank_conf[0].tank_name, 32, "First");
		snprintf(settings_info.tank_conf[1].tank_name, 32, "Second");
		snprintf(settings_info.tank_conf[2].tank_name, 32, "Third");
		snprintf(settings_info.tank_conf[3].tank_name, 32, "Fourth");
		snprintf(settings_info.tank_conf[4].tank_name, 32, "Fifth");

		TRACE("Default settings saved!");
		TRACE("magic: %d", settings_info.magic);

		_save_settings_info(&settings_info, 0);
		_get_settings_info(&settings_info);
	}
}
