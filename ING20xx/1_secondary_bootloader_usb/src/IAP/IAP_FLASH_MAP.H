#ifndef _IAP_FLASH_MAP_H_
#define _IAP_FLASH_MAP_H_

#include "eflash.h"

// FLASH MAP: https://www.yuque.com/tanqian-c0ev9/czrc2l/sg3s9lapwmg06ggb?singleDoc

#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
#define EFLASH_UNIT_SIZE              (uint32_t)(EFLASH_PAGE_SIZE)
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
#define EFLASH_UNIT_SIZE              (uint32_t)(EFLASH_SECTOR_SIZE)
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_20)
#define EFLASH_UNIT_SIZE              (uint32_t)(EFLASH_SECTOR_SIZE)
#endif
#define ALIGN_TO(address, alignment)  (((address) + (alignment) - 1) & ~((alignment) - 1))
#define ALIGN_TO_EFLASH_UNIT(address) ALIGN_TO(address, EFLASH_UNIT_SIZE)

/*----------------------------------------------
0     ~ 2000     用来存放系统升级代码，芯片起始文件 8K
2000  ~ 3000     用来存放2级boot 4K
3000  ~ 27000    用来存放 platform 数据 144K
27000 ~ 6B000    用来存放 User 的代码以及 Flash空间
6B000 ~ 76000    用来存放 User 的 Flash 数据 基本信息以及功能区还有按键矩阵等默认数据 暂定44K
76000 ~ 78000    用来存储无线、配对地址、蓝牙名称数据 8K
78000 ~ 80000    用来存储 boot 升级代码 32K
----------------------------------------------*/

#define APP_ADDR                      (0x0200E000)
#define APP_SIZE                      (0x00019000)     // 163840 Byte   160KB
#define KV_PARAM_ADDR                 (0x0207E000)     // kv Param area: from KV_PARAM_ADDR to FLASH_TOP_ADDR
#define RSVD_START_ADDR               (uint32_t)(KV_PARAM_ADDR)
#define USER_PARAM_ADDR               (0x0207d000)     // user Param area: from USER_PARAM_ADDR to KV_PARAM_ADDR

#define SECOND_BOOT_START_ADDR        (uint32_t)(0x02002000)
#define PLATFORM_START_ADDR           (uint32_t)(0x02008000)
#define APP_START_ADDR                (uint32_t)(APP_ADDR)
#define APP_TOP_ADDR                  (uint32_t)(ALIGN_TO_EFLASH_UNIT(APP_ADDR + APP_SIZE))
#define APP_PARAM_START_ADDR          (uint32_t)(USER_PARAM_ADDR)
#define FLASH_TOP_ADDR                (uint32_t)(0x02080000)

#define SECOND_BOOT_CODE_SIZE         (PLATFORM_START_ADDR - SECOND_BOOT_START_ADDR)
#define PLATFORM_CODE_SIZE            (APP_START_ADDR      - PLATFORM_START_ADDR)
#define APP_CODE_SIZE                 (APP_TOP_ADDR        - APP_START_ADDR)
#define PLATFORM_APP_CODE_SIZE        (APP_TOP_ADDR        - PLATFORM_START_ADDR)
#define APP_PARAM_SIZE                (FLASH_TOP_ADDR      - APP_PARAM_START_ADDR)
#define USER_PARAM_SIZE               (KV_PARAM_ADDR       - USER_PARAM_ADDR)

#endif

