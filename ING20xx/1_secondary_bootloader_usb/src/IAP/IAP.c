#include "IAP.h"
#include "eflash.h"
#include "bsp_usb_hid_iap.h"
#include "IAP_Transport.h"
#include "IAP_Application.h"
#include "stdbool.h"
#include "rom_tools.h"
#include "IAP_Flash_WP.h"
#include "IAP_UserDef.h"
#include "crc16.h"


#if USER_IAP_MAIN_ERROR_LOG_EN
#define IAP_ERROR(...)  printf(__VA_ARGS__)
#else
#define IAP_ERROR(...)      
#endif

#if USER_IAP_MAIN_DEBUG_LOG_EN
#define IAP_DEBUG(...)  printf(__VA_ARGS__)
#else
#define IAP_DEBUG(...)      
#endif


// =================================================================================================

/**
 * @brief IAP_Init
 * 
 */
void IAP_Init(void){
    IAP_DEBUG("\n===>IAP Init\n");

//    IAP_Flash_WP_Init();
    IAP_Application_Init();
    IAP_Transport_Init();
    // bsp_usb_disable();
    // bsp_usb_init();
}

