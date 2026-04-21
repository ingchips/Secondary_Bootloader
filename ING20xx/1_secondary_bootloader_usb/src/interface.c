#include "interface.h"
#include "peripheral_usb.h"
#include "eflash.h"
#include "IAP_Bootloader.h"

static uint8_t app_timer_flag = 0;
static app_timer_callback timer_callback;
static uint8_t quite_loop_flag = 0;

uint32_t n20_IRQHandler(void *user_data)
{
	return USB_IrqHandler(user_data);
}

uint32_t TIMER1_IRQHandler(void *user_data)
{
    TMR_IntClr(APB_TMR1, 0, 0xf);
    TMR_Enable(APB_TMR1, 0, 0);
    TMR_IntEnable(APB_TMR1, 0, 0);
	app_timer_flag = 1;
	return 0;
}

uint32_t n31_IRQHandler(void *user_data)
{
	return TIMER1_IRQHandler(user_data);
}

void app_quite_loop(void)
{
	quite_loop_flag = 1;
}

/* 通过看门狗复位，所有寄存器均恢复到默认状态，USB 会复位掉线。*/
void wdt_reset(void)
{
	// 擦除元信息
	// 只有在这个动作执行期间电源失效，才有可能导致固件损坏
	erase_flash_sector(FLASH_BASE + EFLASH_SECTOR_SIZE);
	
    SYSCTRL_ClearClkGate(SYSCTRL_ITEM_APB_WDT);
    TMR_WatchDogEnable3(WDT_INTTIME_INTERVAL_2MS, WDT_RSTTIME_INTERVAL_4MS, 0);
    while(1);
}

// app 与 usb boot 使用同一片内存，防止跳转后变量所在的内存被 app 赋值
void app_timer_init(void)
{
    app_timer_flag = 0;
    timer_callback = NULL;
	quite_loop_flag = 0;
}

uint32_t app_timer_start(app_timer_callback callback, uint16_t ms)
{
	if(callback == NULL)
	{
		return 1;
	}
	timer_callback = callback;
    // setup timer1
    TMR_SetOpMode(APB_TMR1, 0, TMR_CTL_OP_MODE_32BIT_TIMER_x1, TMR_CLK_MODE_APB, 0);
    TMR_SetReload(APB_TMR1, 0, TMR_GetClk(APB_TMR1, 0) / 1000 * ms);
    TMR_Enable(APB_TMR1, 0, 0xf);
    TMR_IntEnable(APB_TMR1, 0, 0xf);

//	NVIC_SetPriority(IRQn_ITMER1, 1);
	__NVIC_ClearPendingIRQ(IRQn_ITMER1);
	__NVIC_EnableIRQ(IRQn_ITMER1);
	return 0;
}

void app_timer_stop(void)
{
    TMR_Enable(APB_TMR1, 0, 0);
    TMR_IntEnable(APB_TMR1, 0, 0);
	__NVIC_DisableIRQ(IRQn_ITMER1);
}

uint32_t app_loop(void)
{
	if(app_timer_flag)
	{
		timer_callback();
		app_timer_flag = 0;
	}
	
	if(quite_loop_flag)
	{
		quite_loop_flag = 0;
		return 1;
	}
	
	return 0;
}