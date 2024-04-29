# CircStore
# [中文](README.md) | English
## A very simple flash simulation EEPROM storage library, suitable for cheap MCUs, can evenly wear flash and provide higher storage efficiency.


### 1. Introduction
CircStore is a very simple storage library for MCU flash, which provides a simple API for storing data in the flash of the MCU. The features of CircStore are:
- 1. It uses circular storage to evenly wear flash and provide higher storage efficiency.
- 2. It uses a simple data structure that can be quickly implemented on an MCU.
- 3. It is extremely easy to use, only requiring the implementation of flash read and write functions.
- 4. It is a single file with no dependencies, and can be directly copied into a project for use.
- 5. It uses binary search for block searching, improving block search efficiency.

### 2. Usage
CircStore is very easy to use, you only need to implement the flash read and write functions and modify 2 macro definitions. Here is a simple example:
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
Read the comments in the source code to understand the API of CircStore.

### 4. Notes
- 1. FLASH_ERASE_SIZE, so when using it, you need to set the size of the block according to the actual situation.
- 2. The maximum stored data cannot exceed SINGLE_STORE_SIZE-8.
- 3. The stored data is circular, and when it exceeds the maximum size, it will overwrite the earliest data.
- 4. The reliability of the stored data is verified by the user, only responsible for storage, not for verification. The integrity of the data is recommended to be verified by the user after reading.
- 5. It is recommended to have at least 2 blocks.
- 6. Please modify the flash read and write functions according to the actual situation. Ensure atomic operations to prevent interruption during the read and write process.
- 7. Notice if you use as configuration storage, please ensure the data is store in one SINGLE_STORE_SIZE, and the data is not exceed the SINGLE_STORE_SIZE-8,you should read the lastest as current config,DO NOT use history data as config, it may erase by new data.


### Follow me [bilibili](https://space.bilibili.com/1922147080)