#include "esp_common.h"
#include "esp_system.h"

#include "settings.h"
#include "debug.h"

#define SHIFT_TO_SECTOR			12			/*For Flash of sector size 4KB*/

static const uint32_t settings_address = 0x00070000UL;
static const uint32_t settings_magic = 0xddddddddUL;

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

_S_SETTINGS_INFO *get_all_settings(void) {
	return &settings_info;
}

static void _get_settings_info(_S_SETTINGS_INFO *readInfo) {
	SpiFlashOpResult status;

	status = spi_flash_read(settings_address, (uint32_t *)readInfo, sizeof(*readInfo));
	TRACE("_Settings read, SPI_STATUS: %d", status);
}

void settings_init(void) {
    _get_settings_info(&settings_info);
	TRACE("after read");
	if(settings_info.magic == settings_magic) {
		TRACE("magic found");
	} else {
        memset(&settings_info, 0, sizeof(settings_info));

		settings_info.magic = settings_magic;
		
		strncpy(settings_info.ssid, "ESP_STANDALONE", 32);
        strncpy(settings_info.passwd, "aaaaaaaa", 64);
        strncpy(settings_info.cont_name, "MController", 32);
        strncpy(settings_info.node_name, "This_Node", 32);
        settings_info.is_registered = 0;

		_save_settings_info(&settings_info, 0);
		_get_settings_info(&settings_info);
	}
}
