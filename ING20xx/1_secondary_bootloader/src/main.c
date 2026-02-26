//#include "platform_api.h"
#include <stdint.h>
#include <string.h>
#include "eflash.h"

#define DEF_UPDATE_FLAG     0x5A5A5A5A

typedef struct block_info
{
    uint32_t size;
    uint32_t dst;
    uint32_t src;
    uint32_t magic;
} block_info_t;

# define __PLATFORM_ADDR    0x02004000

typedef struct boot_ver_s
{
    unsigned short major;
    char  minor;
    char  patch;
} boot_ver_t;

const boot_ver_t *boot_get_version(void)
{
    extern uint32_t __BOOT_VER;
    return (const boot_ver_t *)&__BOOT_VER;
}

//static void set_reg_bit(volatile uint32_t *reg, uint8_t v, uint8_t bit_offset)
//{
//    if (v)
//        *reg |= 1 << bit_offset;
//    else
//        *reg &= ~(1 << bit_offset);
//}

//static void SYSCTRL_DisableSlowRC(void)
//{
//    #define AON1_PMU3       ((volatile uint32_t *)(AON1_CTRL_BASE + 0x3c))

//    set_reg_bit(AON1_PMU3, 0, 19);
//}

/* Note: If 24M OSC is used as source of slow clock, then
 * this simplified `SYSCTRL_GetSlowClk` can be used:
 *
 * uint32_t SYSCTRL_GetSlowClk(void)
 * {
 *     return OSC_CLK_FREQ;
 * }
 */

// Jump to application at specified address [keil compiler version 5]
void jump_to_app(uint32_t app_addr)
{
    typedef void (*pFunction)(void);
    uint32_t jumpAddress;
    pFunction JumpToApplication;

    // Get the application stack pointer (first 4 bytes at app_addr)
    __set_MSP(*(volatile uint32_t *)app_addr);

    // Get the application entry point (second 4 bytes at app_addr + 4)
    jumpAddress = *(volatile uint32_t *)(app_addr + 4);
    JumpToApplication = (pFunction)jumpAddress;

    // Jump to application
    JumpToApplication();
}

// 把数据从 Flash 拷贝到 Flash 的辅助函数：逐扇区读入内存再写入
static int flash_to_flash(uint32_t src, uint32_t dst, uint8_t *buffer, uint32_t size)
{
    while (size > 0)
    {
        int r;
        uint32_t block = size;
        if (block >= EFLASH_ERASABLE_SIZE) block = EFLASH_ERASABLE_SIZE;

        memcpy(buffer, (const void *)src, block);
        r = program_flash(dst, buffer, block);
        if (r) return r;

        dst += block;
        src += block;
        size -= block;
    }
    return 0;
}

// 可以在这里做升级前的检查，可酌情设计检查方法
// 这里检查了目标地址是否按扇区对齐
static int check_fota_blocks(const block_info_t *p)
{
    while (p->magic == DEF_UPDATE_FLAG)
    {
        if (p->dst & (EFLASH_SECTOR_SIZE - 1))
            return -1;
        p--;
    }
    return 0;
}

int main(void)
{
    SYSCTRL_ConfigPLLClk(5, 80, 1);
    SYSCTRL_EnablePLL(1);
    SYSCTRL_SelectFlashClk(SYSCTRL_CLK_PLL_DIV_5); // 76.8MHZ
    SYSCTRL_SelectHClk(SYSCTRL_CLK_PLL_DIV_3); //128MHZ

//    SYSCTRL_DisableSlowRC();
	
    const block_info_t *p = (const block_info_t *)(FLASH_BASE +
        EFLASH_SECTOR_SIZE * 2 - sizeof(block_info_t));
	if(p->magic == DEF_UPDATE_FLAG)
	{
		if (check_fota_blocks(p) == 0)
		{
			// 检查通过，逐项拷贝
			while (p->magic == DEF_UPDATE_FLAG)
			{
				// 注意：内存直接使用，不定义变量
				flash_to_flash(p->src, p->dst, (void *)0x20002000, p->size);
				p--;
			}
			// 擦除元信息
			// 只有在这个动作执行期间电源失效，才有可能导致固件损坏
			erase_flash_sector(FLASH_BASE + EFLASH_SECTOR_SIZE);

			// 重启
			NVIC_SystemReset();
		}
	}

    jump_to_app(__PLATFORM_ADDR);

    return 0;
}
