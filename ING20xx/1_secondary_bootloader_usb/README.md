# secondary_bootloader_usb

ING20 系列芯片二级 bootloader 的实现，该二级 bootloader 集成 USB 在线升级的功能。

# 宏
`DEBUG_PRINTF`  开关日志

# 结构体
## 固件搬运

升级用的元数据从 0x02002000 往下存放，首先是 boot_settings，
然后后面跟着 N 个 block_info，升级完成后擦除。
相关定义：

```c
typedef struct block_info
{
    uint32_t size;          // 固件大小
    uint32_t dst;           // 固件目标地址
    uint32_t src;           // 固件源地址
} block_info_t;

typedef struct 
{
    uint32_t enter_secondary_boot;   // 是否要进入二级 boot
    uint32_t entered_secondary_boot; // 是否进入过二级 boot
    uint32_t need_firmware_replace;  // 是否需要进行固件替换升级
	uint32_t block_num;              // 需要升级的固件个数
	uint32_t crc_value;              // block crc 校验码
} boot_settings_t;
```


