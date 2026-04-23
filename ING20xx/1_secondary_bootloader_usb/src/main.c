#include <stdint.h>
#include <string.h>
#include "eflash.h"
#include "ingsoc.h"
#include "bsp_usb_hid_iap.h"
#include "IAP.h"
#include "interface.h"
#include "rom_tools.h"

static uint8_t buff[EFLASH_ERASABLE_SIZE];

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

#if (DEBUG_PRINTF == 1)

#define PRINT_PORT    APB_UART0

uint32_t cb_putc(char *c, void *dummy)
{
    while (apUART_Check_TXFIFO_FULL(PRINT_PORT) == 1);
    UART_SendData(PRINT_PORT, (uint8_t)*c);
    return 0;
}

int fputc(int ch, FILE *f)
{
    cb_putc((char *)&ch, NULL);
    return ch;
}

void config_uart(uint32_t freq, uint32_t baud)
{
    UART_sStateStruct config;

    config.word_length       = UART_WLEN_8_BITS;
    config.parity            = UART_PARITY_NOT_CHECK;
    config.fifo_enable       = 1;
    config.two_stop_bits     = 0;
    config.receive_en        = 1;
    config.transmit_en       = 1;
    config.UART_en           = 1;
    config.cts_en            = 0;
    config.rts_en            = 0;
    config.rxfifo_waterlevel = 1;
    config.txfifo_waterlevel = 1;
    config.ClockFrequency    = freq;
    config.BaudRate          = baud;

    apUART_Initialize(PRINT_PORT, &config, 0);
}

void setup_peripherals(void)
{
    SYSCTRL_ClearClkGateMulti(1 << SYSCTRL_ClkGate_APB_UART0 | 1 << SYSCTRL_ClkGate_APB_TMR1);
    config_uart(OSC_CLK_FREQ, 460800);
}

#else

void setup_peripherals(void)
{

}

#endif


/* Note: If 24M OSC is used as source of slow clock, then
 * this simplified `SYSCTRL_GetSlowClk` can be used:
 *
 * uint32_t SYSCTRL_GetSlowClk(void)
 * {
 *     return OSC_CLK_FREQ;
 * }
 */

#define __PLATFORM_ADDR    0x02008000
//// Jump to application at specified address [keil compiler version 5]
//void jump_to_app(uint32_t app_addr)
//{
//    typedef void (*pFunction)(void);
//    uint32_t jumpAddress;
//    pFunction JumpToApplication;

//    // Get the application stack pointer (first 4 bytes at app_addr)
//    __set_MSP(*(volatile uint32_t *)app_addr);

//    // Get the application entry point (second 4 bytes at app_addr + 4)
//    jumpAddress = *(volatile uint32_t *)(app_addr + 4);
//    JumpToApplication = (pFunction)jumpAddress;

//    // Jump to application
//    JumpToApplication();
//}

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

void check_blocks_and_update(void)
{	
#if (DEBUG_PRINTF == 1)
	setup_peripherals();
#endif
    const boot_settings_t *boot_settings = (const boot_settings_t *)(0x2002000 - sizeof(boot_settings_t));
	
    const block_info_t *block_info = (const block_info_t *)((uint32_t)boot_settings - sizeof(block_info_t));
	
	// CRC check
	uint16_t crc_compute = crc((uint8_t *)((uint32_t)boot_settings - boot_settings->block_num * sizeof(block_info_t)), 
						        sizeof(block_info_t) * boot_settings->block_num);

	if(crc_compute != boot_settings->crc_value)
	{
		debug_printf("CRC check error, crc_compute : 0x%x, crc_value : 0x%x.\n", crc_compute, boot_settings->crc_value);
		// 升级相关信息错误，直接重启
		goto need_reset;
	}

	debug_printf("Start firmware replacement.\n");
	const block_info_t *p = block_info;
	if(boot_settings->block_num)
	{
		// 可以在这里做升级前的检查，可酌情设计检查方法
		// 这里检查了目标地址是否按扇区对齐
		for(uint32_t i = 0; i < boot_settings->block_num; i++)
		{
			if (p->dst & (EFLASH_SECTOR_SIZE - 1))
				goto need_reset;
			p--;
		}
		
		// 检查通过，逐项拷贝
		p = block_info;
		for(uint32_t i = 0; i < boot_settings->block_num; i++)
		{
			// 注意：内存直接使用，不定义变量
			int r = flash_to_flash(p->src, p->dst, buff, p->size);
			p--;
		}
	}
	
need_reset:
	// 重启
	wdt_reset();
}

int main(void)
{
    SYSCTRL_ConfigPLLClk(5, 80, 1);
    SYSCTRL_EnablePLL(1);
    SYSCTRL_SelectFlashClk(SYSCTRL_CLK_PLL_DIV_5); // 76.8MHZ, When the voltage is lower than 2.7V, the flash operates at a frequency of less than 40 MHz.
    SYSCTRL_SelectHClk(SYSCTRL_CLK_PLL_DIV_3); // 128MHZ
	
	// 将 VTOR 指定到当前向量表下，执行完后，应修改回默认值 0x00000000 或重启
	__disable_irq();
	SCB->VTOR = 0x2002000;
	__enable_irq();
	
	// 设置已经进入二级 boot 的标志位
	set_entered_secondary_boot_flag();
	
	// 初始化要用到的外设，比如 uart
	setup_peripherals();
	
	bsp_usb_init();
	IAP_Init();
	
	app_timer_init();
	app_timer_start(wdt_reset, 10000);
	debug_printf("start secondary boot\n");
	
	while(1)
	{
		if(app_loop() == 1)
		{
			check_blocks_and_update();
			
			// 升级失败后重启，一般不会跑到这里
			wdt_reset();
		}
	}

    return 0;
}


//int main(void)
//{
//    SYSCTRL_ConfigPLLClk(5, 80, 1);
//    SYSCTRL_EnablePLL(1);
//    SYSCTRL_SelectFlashClk(SYSCTRL_CLK_PLL_DIV_5); // 76.8MHZ
//    SYSCTRL_SelectHClk(SYSCTRL_CLK_PLL_DIV_3); // 128MHZ
////    SYSCTRL_DisableSlowRC();
//	
//    const boot_settings_t *boot_settings = (const boot_settings_t *)(FLASH_BASE +
//        EFLASH_SECTOR_SIZE * 2 - sizeof(boot_settings_t));
//	
//	if(boot_settings->need_firmware_replace == DEF_FLAG) // 需要搬运固件，通常为接收固件完毕后，掉电重启
//	{
//		check_blocks_and_update();
//	}
//	else if(boot_settings->entered_secondary_boot == DEF_FLAG) // 进入 USB boot 过后，掉电重启一般到这里，直接进入跳过进入 APP
//	{
//		//TODO
//	}
//	else if(boot_settings->enter_secondary_boot == DEF_FLAG) // 需要停留在 USB boot
//	{
//		// 将 VTOR 指定到当前向量表下，执行完后，应修改回默认值 0x00000000 或重启
//		__disable_irq();
//		SCB->VTOR = 0x2002000;
//		__enable_irq();
//		
//		// 初始化要用到的外设，比如 uart
//		setup_peripherals();
//		
//		bsp_usb_init();
//		IAP_Init();
//		
//		app_timer_init();
//	    app_timer_start(wdt_reset, 10000);
//		debug_printf("start\n");
//		
//		while(1)
//		{
//			if(app_timer_loop() == 1)
//			{
//				check_blocks_and_update();
//				
//				// 升级失败后重启，一般不会跑到这里
//				wdt_reset();
//			}
//		}
//	}
//	
//    jump_to_app(__PLATFORM_ADDR);

//    return 0;
//}
