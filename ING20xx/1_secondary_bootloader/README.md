# secondary_bootloader

ING20 系列芯片二级 bootloader 的实现，仅实现固件搬运的功能。


## 固件搬运

升级用的元数据我们照搬 ING918 里的做法， 从 0x02002000 往下依次存放 N 个 block_info，升级完成后擦除。block_info 的定义：

```c
typedef struct block_info
{
    uint32_t size;          // 固件大小
    uint32_t dst;           // 固件目标地址
    uint32_t src;           // 固件源地址
    uint32_t magic;         // 是否需要搬运固件的标志为，0x5A5A5A5A
} block_info_t;
```
