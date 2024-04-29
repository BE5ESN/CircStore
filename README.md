# CircStore
# [English](README_en.md) | 中文
## 一个非常简单flash模拟eeprom的存储库,适用于廉价mcu,能均匀磨损flash,提供更高的存储效率。

### 1. 介绍
CircStore是一个非常简单的MCU flash的存储库，它提供了一个简单的API，可以在MCU的flash中存储数据。CircStore的特点是：
- 1. 采用循环存储的方式，可以在均匀flash磨损的同时，提供更高的存储效率。
- 2. 采用了简单的数据结构，可以在MCU上快速实现。
- 3. 使用极其简单，仅需实现flash读写函数即可。
- 4. 单文件，无依赖，可以直接拷贝到工程中使用。
- 5. 内部寻块时采用了二分查找，提高了寻块效率。

### 2. 使用
CircStore的使用非常简单，只需要实现flash读写函数和修改2个宏定义即可。下面是一个简单的例子：
```c
#include "circstore.h"
unsigned char flashRead(uint32_t address, uint8_t *buf, uint16_t bufLen)
{
    // TODO: fix your flash read function
    return 0;
}

unsigned char flashWrite(uint32_t address, uint8_t *buf, uint16_t bufLen)
{
    // TODO: fix your flash write function
}

unsigned char flashErase(uint32_t address)
{
    // TODO: fix your flash erase function earse block(FLASH_ERASE_SIZE)
}

const CircStoreInterface csi = {
    .read = flashRead,
    .write = flashWrite,
    .erase = flashErase,
};

static CircStoreHandler_t handler;

#define YOUR_STORE_ADDRESS 0x08000000
#define YOUR_STORE_BLOCKS 2

void main(void)
{
    uint8_t readData[8];
    uint16_t len = 8;
    uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    circStoreInit(&handler, YOUR_STORE_ADDRESS, YOUR_STORE_BLOCKS, &csi);

    // Add a test result, the size of a single block should not exceed logBlockSize
    circStoreAdd(&handler, data, 8);
    // Read the latest record, returns 0 on success, otherwise failure
    circStoreReadLatest(&handler, readData, &len);
    // Read a specific record, returns 0 on success, otherwise failure
    circStoreReadByLogIndex(&handler, 0, readData, &len);
}
```

### 3. API
阅读源码中的注释，可以了解CircStore的API。

### 4. 注意
- 1. FLASH_ERASE_SIZE，所以在使用时，需要根据实际情况设置block的大小。
- 2. 最大的存储数据不可超过SINGLE_STORE_SIZE-8
- 3. 的存储数据是循环存储的，当超过最后大小后，会覆盖最早的数据。
- 4. 的存储可靠性由用户自行校验，只管存不管校，不保证数据的完整性，建议用户读取后校验数据的完整性。
- 5. 建议至少2块block。
- 6. 请根据实际情况修改flash读写函数。自行保证原子操作，防止在读写过程中被打断。
- 7. 作为配置文件记录时，请保证所有记录在一个SINGLE_STORE_SIZE内，仅使用最后一个记录作为配置文件，因为在写入时会覆盖之前的记录。


### 欢迎关注我的bilibili账户，[惜枫醉](https://space.bilibili.com/1922147080)
