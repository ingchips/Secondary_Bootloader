#ifndef _IAP_BOOTLOADER_H_
#define _IAP_BOOTLOADER_H_

#include <stdint.h>
#include "eflash.h"

#define DEF_FLAG 0x5A5A5A5A

typedef struct block_info
{
    uint32_t size;
    uint32_t dst;
    uint32_t src;
} block_info_t;

typedef struct 
{
    uint32_t enter_secondary_boot;   // 是否要进入二级 boot
    uint32_t entered_secondary_boot; // 是否进入过二级 boot
    uint32_t need_firmware_replace;  // 是否需要进行固件替换升级
	uint32_t block_num;              // 需要升级的固件个数
	uint32_t crc_value;              // block crc 校验码
} boot_settings_t;

/**
 * @brief Program FOTA metadata.
 *
 * @param[in] block_num         number of blocks
 * @param[in] blocks            an array of `fota_update_block_t`
 * @return                      0 if successful else non-0
 */
int program_fota_metadata(const int block_num, const fota_update_block_t *blocks);


int set_entered_secondary_boot_flag(void);

#endif