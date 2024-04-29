#ifndef __CIRCSTORE_H__
#define __CIRCSTORE_H__
#include "stdint.h"
/*
    作用：循环存储库，用于存储固定大小的数据块，当存储满时，会覆盖最早的数据块
    适用场景：以固定块的方式均匀存储数据，当存储满时，覆盖最早的数据块,适用于存储日志，存储基本配置文件等
    让flash的使用寿命更加均匀，减少擦写次数。
    实现极为高效，内部采用二分法查找，时间复杂度为O(logN)
    适用于单片机等平台，用户仅需配置flash操作接口，即可使用
    作者：惜枫醉
    版本：v1.0
*/

// flash操作接口
// flash基本配置，需要根据实际情况修改,影响所有circStoreHandler
#define FLASH_ERASE_SIZE 4096 // flash擦除块大小
#define SINGLE_STORE_SIZE 32 // 单个存储块的大小，需要为FLASH_ERASE_SIZE整除,需要注意的是每一块都包含了8byte头部信息，所以实际存储的数据会比这个值小





// 接口函数,由用户自行实现
//  读取数据块的回调函数,返回值为0时表示读取成功,否则失败
//  addressOffset:读取的地址偏移
//  buf:读取buf
//  bufLen:读取buf长度, 读取长度为FLASH_READ_SIZE的整数倍，最大SINGLE_BLOCK_SIZE
typedef unsigned char (*csi_read)(uint32_t address, uint8_t *buf, uint16_t bufLen);

// 写入数据块的回调函数,返回值为0时表示写入成功,否则失败
// addressOffset:写入的地址偏移
// buf:写入buf
// bufLen:写入buf长度, 写入长度为FLASH_WRITE_SIZE的整数倍，最大SINGLE_BLOCK_SIZE
typedef unsigned char (*csi_write)(uint32_t address, uint8_t *buf, uint16_t bufLen);

// 擦除数据块的回调函数,返回值为0时表示擦除成功,否则失败
// addressOffset:擦除的地址偏移,擦除地址传入为FLASH_ERASE_SIZE的整数倍
typedef unsigned char (*csi_erase)(uint32_t address);

typedef struct
{
    csi_read read;
    csi_write write;
    csi_erase erase;
} CircStoreInterface;

typedef struct
{
    uint32_t flashStart;           // flash起始地址,需要4字节对齐
    uint16_t flashBlocks;          // flash块数,需要为FLASH_ERASE_SIZE的整数倍,当只有一块时，会出现丢失全部历史记录的情况，所以建议至少2块
    const CircStoreInterface *csi; // flash操作接口,需要用户实现,不可为空
} CircStoreHandler_t;

// 初始化circStore库
// handler: circStore库句柄
// flashStart: flash起始地址,需要4字节对齐
// flashBlocks: flash块数,需要为FLASH_ERASE_SIZE的整数倍,当只有一块时，会出现丢失全部历史记录的情况，所以建议至少2块
// csi: flash操作接口,需要用户实现,不可为空
void circStoreInit(CircStoreHandler_t *handler, uint32_t flashStart, uint32_t flashBlocks, const CircStoreInterface *csi);
// 添加测试结果，单块数据大小不可超过logBlockSize
// data:数据
// len:数据长度,长度最长为SINGLE_STORE_SIZE-8
// 返回值为0时表示添加失败，其他:记录的logIndex
uint32_t circStoreAdd(CircStoreHandler_t *handler, uint8_t *data, uint16_t len);
// 读取最新的记录,返回值为0时表示读取成功,否则失败
// data:数据
// len:数据长度
// 返回值为0时表示读取成功,否则失败
int8_t circStoreReadLatest(CircStoreHandler_t *handler, uint8_t *data, uint16_t *len);
// 读取特定记录,返回值为0时表示读取成功,否则失败
// logIndex:记录索引
// data:数据
// len:数据长度
// 返回值为0时表示读取成功,否则失败
int8_t circStoreReadByLogIndex(CircStoreHandler_t *handler, uint32_t logIndex, uint8_t *data, uint16_t *len);
// 获取记录的数量,返回记录数量
// 返回值为0时表示读取成功,否则失败
uint16_t circStoreGetLogCount(CircStoreHandler_t *handler);
// 清除所有范围内的记录,返回值为0时表示清除成功,否则失败
int8_t circStoreClear(CircStoreHandler_t *handler);
// 获取最后一条记录的索引
uint32_t circStoreGetLastLogIndex(CircStoreHandler_t *handler); 

#endif
