// author:xifengzui AKA BG5ESN
// version: v1.0
#include "circStore_zh.h"

#define CS_PRINT //printf

#if (FLASH_ERASE_SIZE % SINGLE_STORE_SIZE != 0)
#error "FLASH_ERASE_SIZE must be an integer multiple of SINGLE_STORE_SIZE"
#endif

#if (FLASH_ERASE_SIZE % 4 != 0)
#error "FLASH_ERASE_SIZE must be an integer multiple of 4"
#endif

#pragma pack(1)
typedef struct
{
    uint8_t usedMask;    // Used to mark whether each block is used: 0xAA: used, others: unused
    uint8_t reserved[1]; // Reserved
    uint32_t logIndex;   // Record ID
    uint16_t logLen;     // Record length
} CircStoreHeadler_t;
#pragma pack()
typedef struct
{
    CircStoreHeadler_t header;                                       // Record header
    uint8_t logData[SINGLE_STORE_SIZE - sizeof(CircStoreHeadler_t)]; // Record data
} CircStoreBlock_t;

// Return 0 for success, otherwise failure
int8_t circStoreGetFirstBlockAddress(CircStoreHandler_t *handler, uint32_t *address, uint32_t *logIndex)
{
    int blockCount = handler->flashBlocks;
    uint32_t minIndex = 0xFFFFFFFF;
    CircStoreHeadler_t header;
    int ii = 0;
    // Read the usedMask of each block and find the smallest one, return the address and logIndex
    for (ii = 0; ii < blockCount; ii++)
    {
        uint32_t addr = handler->flashStart + ii * FLASH_ERASE_SIZE;
        handler->csi->read(addr, (uint8_t *)&header, sizeof(CircStoreHeadler_t));
        if (header.usedMask != 0xAA)
        {
            continue;
        }

        if (header.logIndex < minIndex)
        {
            minIndex = header.logIndex;
            *address = addr;
            *logIndex = header.logIndex;
        }
    }

    if (minIndex == 0xFFFFFFFF)
    {
        CS_PRINT("no valid block found\n");
        *address = 0;
        *logIndex = 0;
        return -1;
    }

    return 0;
}

// Return 0 for success, otherwise failure
int8_t circStoreGetLastBlockAddress(CircStoreHandler_t *handler, uint32_t *address, uint32_t *logIndex)
{
    // Calculate the count
    CircStoreHeadler_t header;
    uint16_t leftStoreIndex = 0;
    uint16_t rightStoreIndex = 0;
    uint32_t addr = 0;
    uint16_t ii = 0;
    uint32_t maxIndex = 0;
    uint16_t foundIndex = 0;
    // Find the last block using binary search
    for (ii = 0; ii < handler->flashBlocks; ii++)
    {
        uint32_t baseAddr = handler->flashStart + ii * FLASH_ERASE_SIZE;
        leftStoreIndex = 0;
        rightStoreIndex = (FLASH_ERASE_SIZE / SINGLE_STORE_SIZE) - 1;
        // Find the last used block position
        while (leftStoreIndex < rightStoreIndex)
        {
            uint32_t midStoreIndex = (leftStoreIndex + rightStoreIndex) / 2;
            addr = baseAddr + midStoreIndex * SINGLE_STORE_SIZE;
            handler->csi->read(addr, (uint8_t *)&header, sizeof(CircStoreHeadler_t));
            if (header.usedMask == 0xAA)
            {
                foundIndex = midStoreIndex;
                leftStoreIndex = midStoreIndex + 1;
            }
            else
            {
                rightStoreIndex = midStoreIndex;
            }
        }

        // Find the last used block position
        if (leftStoreIndex == rightStoreIndex)
        {
            addr = baseAddr + leftStoreIndex * SINGLE_STORE_SIZE;
            handler->csi->read(addr, (uint8_t *)&header, sizeof(CircStoreHeadler_t));
            if (header.usedMask == 0xAA)
            {
                foundIndex = leftStoreIndex;
            }
        }

        addr = baseAddr + foundIndex * SINGLE_STORE_SIZE;
        // Get the found address
        handler->csi->read(addr, (uint8_t *)&header, sizeof(CircStoreHeadler_t));

        if (header.usedMask == 0xAA && header.logIndex > maxIndex)
        {
            maxIndex = header.logIndex;
            *address = addr;
            *logIndex = header.logIndex;
        }
    }

    if (maxIndex == 0)
    {
        CS_PRINT("no valid block found\n");
        *address = 0;
        *logIndex = 0;
        return -1;
    }
    return 0;
}

// Initialize circStore library
void circStoreTest(CircStoreHandler_t *handler);
void circStoreInit(CircStoreHandler_t *handler, uint32_t flashStart, uint32_t flashBlocks, const CircStoreInterface *csi)
{
    handler->csi = csi;
    handler->flashStart = flashStart;
    handler->flashBlocks = flashBlocks;
    CS_PRINT("circStoreInit:flashStart: 0x%lx, flashBlocks: %d\n", flashStart, flashBlocks);
#if 0
    circStoreTest(handler);
#endif
}

// Add test results, return result, the size of a single block of data cannot exceed logBlockSize
int8_t circStoreAdd(CircStoreHandler_t *handler, uint8_t *data, uint16_t len)
{
    // Write data, index starts from 1
    // Get the last block address
    int8_t ret = 0;
    uint32_t address = 0;
    uint32_t logIndex = 0;
    CircStoreBlock_t block;
    ret = circStoreGetLastBlockAddress(handler, &address, &logIndex);
    // Check if the next block of flash has been written with data and is the first block:
    if (ret != 0 || address == 0)
    {
        // No data written
        address = handler->flashStart;
        logIndex = 1;
    }
    else
    {
        // Data already written
        address += SINGLE_STORE_SIZE;
        logIndex++;
    }
    // Check if a page flip is required: when flipping, erasure is required
    if (((address - handler->flashStart) % FLASH_ERASE_SIZE) == 0)
    {

        // Check if it exceeds the range
        if (address >= (handler->flashStart + handler->flashBlocks * FLASH_ERASE_SIZE))
        {
            address = handler->flashStart; // Restart from the first block
            handler->csi->erase(address);
        }
        else
        {
            // Erase the next block
            handler->csi->erase(address);
        }
    }

    // Write data
    block.header.usedMask = 0xAA;
    block.header.logIndex = logIndex;
    block.header.logLen = len;
    memcpy(block.logData, data, len);

    if (len > SINGLE_STORE_SIZE)
    {
        CS_PRINT("data len is too long\n");
        return -1;
    }

    if (address < handler->flashStart || address > (handler->flashStart + handler->flashBlocks * FLASH_ERASE_SIZE))
    {
        CS_PRINT("address is out of range [0x%lx]\n", address);
        return -1;
    }

    handler->csi->write(address, (uint8_t *)&block, sizeof(CircStoreBlock_t));
    return 0;
}
int8_t circStoreReadByLogIndex(CircStoreHandler_t *handler, uint32_t logIndex, uint8_t *data, uint16_t *len)
{
    // Get the first block address and the last block address, check if logIndex is within the range
    uint32_t firstAddress;
    uint32_t firstLogIndex;
    uint32_t lastAddress;
    uint32_t lastLogIndex;
    int ret = 0;
    ret = circStoreGetFirstBlockAddress(handler, &firstAddress, &firstLogIndex);
    if (ret != 0 || firstAddress == 0)
    {
        CS_PRINT("no valid block found\n");
        return -1;
    }
    // Get the last block address, check if logIndex is within the range
    ret = circStoreGetLastBlockAddress(handler, &lastAddress, &lastLogIndex);
    if (ret != 0 || lastAddress == 0)
    {
        CS_PRINT("no valid block found\n");
        return -1;
    }

    if (logIndex < firstLogIndex || logIndex > lastLogIndex)
    {
        CS_PRINT("logIndex is out of range [%d,%d]\n", firstLogIndex, lastLogIndex);
        return -1;
    }

    // Within the address range, use binary search
    uint32_t leftIndex = 0;
    uint32_t rightIndex = (lastLogIndex - firstLogIndex);
    CircStoreBlock_t block;
    uint32_t midIndex = 0;
    uint32_t midAddress = 0;

    while (leftIndex <= rightIndex)
    {
        midIndex = (leftIndex + rightIndex) / 2;
        midAddress = firstAddress + midIndex * SINGLE_STORE_SIZE;
        // Address out of range, recalculate
        midAddress = midAddress % (handler->flashStart + handler->flashBlocks * FLASH_ERASE_SIZE);
        if (midAddress < handler->flashStart)
        {
            midAddress = handler->flashStart + midAddress;
        }
        handler->csi->read(midAddress, (uint8_t *)&block, sizeof(CircStoreBlock_t));
        if (block.header.logIndex == logIndex)
        {
            memcpy(data, block.logData, block.header.logLen);
            *len = block.header.logLen;
            return 0;
        }
        else if (block.header.logIndex < logIndex)
        {
            leftIndex = midIndex + 1;
        }
        else
        {
            rightIndex = midIndex - 1;
        }
    }
    CS_PRINT("no valid block found\n");
    *data = 0;
    *len = 0;
    return -1;
}

// Read the latest record
int8_t circStoreReadLatest(CircStoreHandler_t *handler, uint8_t *data, uint16_t *len)
{
    // Get the last block address
    uint32_t address;
    uint32_t logIndex;
    CircStoreBlock_t block;
    int ret = 0;
    ret = circStoreGetLastBlockAddress(handler, &address, &logIndex);
    if (ret != 0 || address == 0)
    {
        CS_PRINT("no valid block found\n");
        return -1;
    }
    handler->csi->read(address, (uint8_t *)&block, sizeof(CircStoreBlock_t));
    memcpy(data, block.logData, block.header.logLen);
    *len = block.header.logLen;
    return 0;
}

// Get the total count of records
uint16_t circStoreGetLogCount(CircStoreHandler_t *handler)
{
    // Get the last block address
    uint32_t address;
    uint32_t logIndex;
    int ret = 0;
    ret = circStoreGetLastBlockAddress(handler, &address, &logIndex);
    if (ret != 0 || address == 0)
    {
        CS_PRINT("no valid block found\n");
        return 0;
    }
    // Get the first block address
    uint32_t firstAddress;
    uint32_t firstLogIndex;
    ret = circStoreGetFirstBlockAddress(handler, &firstAddress, &firstLogIndex);
    if (ret != 0 || firstAddress == 0)
    {
        CS_PRINT("no valid block found\n");
        return 0;
    }
    return logIndex - firstLogIndex + 1;
}
// Clear all or records within a specific ID range
int8_t circStoreClear(CircStoreHandler_t *handler)
{
    // Erase all blocks according to ERASE_BLOCK
    int ii = 0;
    for (ii = 0; ii < handler->flashBlocks; ii++)
    {
        handler->csi->erase(handler->flashStart + ii * FLASH_ERASE_SIZE);
    }
    return 0;
}

#if 0
void circStoreTest(CircStoreHandler_t *handler)
{
    uint16_t d = {0};
    uint16_t len;
#if 1
    CS_PRINT("****************Add Test****************\n");
    for (int ii = 0; ii < ((FLASH_ERASE_SIZE / SINGLE_STORE_SIZE) * 2 + 10); ii++)
    {
        uint16_t w1;
        uint16_t wlen = 2;
        w1 = ii;
        circStoreAdd(handler, (uint8_t *)&w1, wlen);
    }
    CS_PRINT("****************Get Log Count Test****************\n");
    CS_PRINT("log count:%d\n", circStoreGetLogCount(handler));
#endif

#if 1
    CS_PRINT("****************Latest Log Test****************\n");
    d = 0;
    len = 0;
    circStoreReadLatest(handler, (uint8_t *)&d, &len);
    CS_PRINT("read latest data:%d\n", d);
#endif

#if 1
    CS_PRINT("****************Read Log by index Test 1****************\n");
    d = 0;
    len = 0;
    circStoreReadByLogIndex(handler, 1, (uint8_t *)&d, &len);
    CS_PRINT("read logIndex 1 data:%d\n", d);
#endif

#if 1
    CS_PRINT("****************Read Log by index Test 258****************\n");
    d = 0;
    len = 0;
    circStoreReadByLogIndex(handler, 258, (uint8_t *)&d, &len);
    CS_PRINT("read logIndex 258 data:%d\n", d);

    CS_PRINT("****************Read Log by index Test 300****************\n");
    circStoreReadByLogIndex(handler, 300, (uint8_t *)&d, &len);
    CS_PRINT("read logIndex 300 data:%d\n", d);
#endif
}
#endif
