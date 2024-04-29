#ifndef __CIRCSTORE_H__
#define __CIRCSTORE_H__
#include "stdint.h"
/*
    Purpose: Circular storage library for storing fixed-size data blocks. When the storage is full, it overwrites the oldest data block.
    Use case: Store data in a uniform manner using fixed blocks. When the storage is full, overwrite the oldest data block. 
    
    Suitable for storing logs, basic configuration files, etc.
    
    Provides even usage of flash memory and reduces erase cycles.
    Highly efficient implementation using binary search with a time complexity of O(logN).
    Suitable for platforms like microcontrollers. Users only need to configure the flash operation interface to use it.
    Author: xifengzui AKA BG5ESN
    Version: v1.0
*/

// Flash operation interface
// Basic flash configuration, needs to be modified according to the actual situation, affects all circStoreHandler
#define FLASH_ERASE_SIZE 4096 // Flash erase block size
#define SINGLE_STORE_SIZE 32 // Size of a single storage block, needs to be divisible by FLASH_ERASE_SIZE. Note that each block includes an 8-byte header, so the actual stored data will be smaller than this value



// Interface functions, to be implemented by the user
// Callback function for reading a data block. Returns 0 on success, otherwise failure.
// addressOffset: Address offset for reading
// buf: Buffer for reading
// bufLen: Length of the buffer, read length should be a multiple of FLASH_READ_SIZE, maximum SINGLE_BLOCK_SIZE
typedef unsigned char (*csi_read)(uint32_t address, uint8_t *buf, uint16_t bufLen);

// Callback function for writing a data block. Returns 0 on success, otherwise failure.
// addressOffset: Address offset for writing
// buf: Buffer for writing
// bufLen: Length of the buffer, write length should be a multiple of FLASH_WRITE_SIZE, maximum SINGLE_BLOCK_SIZE
typedef unsigned char (*csi_write)(uint32_t address, uint8_t *buf, uint16_t bufLen);

// Callback function for erasing a data block. Returns 0 on success, otherwise failure.
// addressOffset: Address offset for erasing, erase address should be a multiple of FLASH_ERASE_SIZE
typedef unsigned char (*csi_erase)(uint32_t address);

typedef struct
{
    csi_read read;
    csi_write write;
    csi_erase erase;
} CircStoreInterface;

typedef struct
{
    uint32_t flashStart;           // Flash start address, needs to be aligned to 4 bytes
    uint16_t flashBlocks;          // Number of flash blocks, needs to be a multiple of FLASH_ERASE_SIZE. When there is only one block, all historical records will be lost, so it is recommended to have at least 2 blocks.
    const CircStoreInterface *csi; // Flash operation interface, needs to be implemented by the user and cannot be empty
} CircStoreHandler_t;

// Initialize the circStore library
// handler: circStore library handler
// flashStart: Flash start address, needs to be aligned to 4 bytes
// flashBlocks: Number of flash blocks, needs to be a multiple of FLASH_ERASE_SIZE. When there is only one block, all historical records will be lost, so it is recommended to have at least 2 blocks.
// csi: Flash operation interface, needs to be implemented by the user and cannot be empty
void circStoreInit(CircStoreHandler_t *handler, uint32_t flashStart, uint32_t flashBlocks, const CircStoreInterface *csi);
// Add a test result, the size of a single block should not exceed logBlockSize
// data: Data
// len: Data length, maximum length is SINGLE_STORE_SIZE-8
// Returns 0 on success, otherwise failure
int8_t circStoreAdd(CircStoreHandler_t *handler, uint8_t *data, uint16_t len);
// Read the latest record, returns 0 on success, otherwise failure
// data: Data
// len: Data length
// Returns 0 on success, otherwise failure
int8_t circStoreReadLatest(CircStoreHandler_t *handler, uint8_t *data, uint16_t *len);
// Read a specific record, returns 0 on success, otherwise failure
// logIndex: Record index
// data: Data
// len: Data length
// Returns 0 on success, otherwise failure
int8_t circStoreReadByLogIndex(CircStoreHandler_t *handler, uint32_t logIndex, uint8_t *data, uint16_t *len);
// Get the number of records, returns the number of records
// Returns 0 on success, otherwise failure
uint16_t circStoreGetLogCount(CircStoreHandler_t *handler);
// Clear all records within the range, returns 0 on success, otherwise failure
int8_t circStoreClear(CircStoreHandler_t *handler);

#endif
