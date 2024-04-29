#include "circStore.h"
// example.c
// author:xifengzui AKA BG5ESN
// version: v1.0
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