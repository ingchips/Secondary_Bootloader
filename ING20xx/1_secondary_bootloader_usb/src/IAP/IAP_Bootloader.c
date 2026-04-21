#include "IAP_Bootloader.h"
#include "rom_tools.h"
#include <string.h>
#include "interface.h"

#define START               (FLASH_BASE + EFLASH_ERASABLE_SIZE)

int program_fota_metadata(const int block_num, const fota_update_block_t *blocks)
{
    int i = 0;
	int r = 0;
    const boot_settings_t *boot_settings = (const boot_settings_t *)(START + EFLASH_SECTOR_SIZE - sizeof(boot_settings_t));
	block_info_t block_info;
	
	// 判断与升级固件相关的 Flash 地址是否可以写入数据
	if(boot_settings->need_firmware_replace != 0xFFFFFFFF || boot_settings->block_num != 0xFFFFFFFF
		|| boot_settings->crc_value != 0xFFFFFFFF)
	{
		erase_flash_sector(START);
	}
	
	uint32_t addr = (uint32_t)boot_settings - sizeof(block_info_t) * block_num;
	for( ; addr < (uint32_t)boot_settings; addr += 4)
	{
		if(*(uint32_t *)addr != 0xFFFFFFFF)
		{
			erase_flash_sector(START);
			break;
		}
	}
	
	// 开始写入新固件相关信息
	boot_settings_t new_settings;
	memcpy(&new_settings, boot_settings, sizeof(boot_settings_t));
	new_settings.block_num = block_num;
	new_settings.need_firmware_replace = DEF_FLAG; // 需要升级固件

	addr = (uint32_t)boot_settings;
	// block info
    for (i = 0; i < block_num; i++)
    {
		addr -= sizeof(block_info_t);
        block_info.size = blocks[i].size;
        block_info.dst  = blocks[i].dest;
        block_info.src  = blocks[i].src;
		r = write_flash(addr, (uint8_t *)&block_info, sizeof(block_info_t));
		if (r) return r;
    }
	
	// block CRC 
	uint16_t crc_compute = crc((uint8_t *)addr, sizeof(block_info_t) * new_settings.block_num);
	new_settings.crc_value = crc_compute;
    const block_info_t *bloc_info = (const block_info_t *)((uint32_t)boot_settings - sizeof(block_info_t));
	debug_printf("dst : 0x%x, src : 0x%x, size : 0x%x.\n", bloc_info->dst, bloc_info->src, bloc_info->size);
	debug_printf("addr : 0x%x, block_num : %d, crc_compute : 0x%x.\n", addr, new_settings.block_num, crc_compute);
	r = write_flash(START + EFLASH_ERASABLE_SIZE - sizeof(boot_settings_t), (uint8_t *)&new_settings, sizeof(boot_settings_t));
	if (r) return r;

    return 0;
}

int set_entered_secondary_boot_flag(void)
{
    const boot_settings_t *boot_settings = (const boot_settings_t *)(START + EFLASH_SECTOR_SIZE - sizeof(boot_settings_t));
	
	uint32_t entered_secondary_boot = DEF_FLAG; // boot_settings_t->entered_secondary_boot
	
	 // 如果已存在标记，擦除后再写
	if(boot_settings->entered_secondary_boot != 0xFFFFFFFF)
	{
		erase_flash_sector(START);
	}
	
	// 存放 boot_settings_t->enter_secondary_boot 的起始地址，0x02002000 - 16 = 0x02001FEC
	int r = write_flash((uint32_t)boot_settings + 4, (uint8_t *)&entered_secondary_boot, sizeof(entered_secondary_boot));
	// 如果标记写入失败，代表 flash 损坏
	if(r)
	{
		erase_flash_sector(START);
	}
	return r;
}
