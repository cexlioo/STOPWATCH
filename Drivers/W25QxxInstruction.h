#ifndef __W25QxxINSTRUCTION_H
#define __W25QxxINSTRUCTION_H

#define W25Qxx_WRITE_ENABLE							0x06    /* 写使能，必须在任何编程/擦除之前设置 */
#define W25Qxx_WRITE_DISABLE						0x04    /* 禁止写入，上电后状态 */
#define W25Qxx_READ_STATUS_REGISTER_1				0x05    /* 读取1号状态寄存器 */
#define W25Qxx_READ_STATUS_REGISTER_2				0x35    /* 读取2号状态寄存器 */
#define W25Qxx_WRITE_STATUS_REGISTER				0x01    /* 写入1号状态寄存器 */
#define W25Qxx_PAGE_PROGRAM							0x02    /* 通过1根SPI线，按页写入 */
#define W25Qxx_QUAD_PAGE_PROGRAM					0x32    /* 通过4根SPI线，按页写入 */
#define W25Qxx_BLOCK_ERASE_64KB						0xD8    /* 将所有 64Kbyte 块设置为 0xFF */
#define W25Qxx_BLOCK_ERASE_32KB						0x52    /* 将所有 32Kbyte 块设置为 0xFF */
#define W25Qxx_SECTOR_ERASE_4KB						0x20    /* 将所有 4Kbyte 扇区设置为 0xFF（擦除它） */
#define W25Qxx_CHIP_ERASE							0xC7    /* 将芯片所有空间填充为0xFF */
#define W25Qxx_ERASE_SUSPEND						0x75    /* 暂停擦除/编程操作（仅当 SUS=0、BYSY=1 时才适用） */
#define W25Qxx_ERASE_RESUME							0x7A    /* 恢复擦除/编程操作（如果 SUS=1，BUSY=0）*/
#define W25Qxx_POWER_DOWN							0xB9    /* 芯片断电（通过读取 ID 上电） */
#define W25Qxx_HIGH_PERFORMANCE_MODE				0xA3    /* 高性能模式 */
#define W25Qxx_CONTINUOUS_READ_MODE_RESET			0xFF    /* 读模式复位 */
#define W25Qxx_RELEASE_POWER_DOWN_HPM_DEVICE_ID		0xAB    /* 上电，读取设备 ID */
#define W25Qxx_MANUFACTURER_DEVICE_ID				0x90    /* 读取制造商 ID 和设备 ID */
#define W25Qxx_READ_UNIQUE_ID						0x4B    /* 读取唯一芯片64位ID */
#define W25Qxx_JEDEC_ID								0x9F    /* 读取 JEDEC 标准 ID */
#define W25Qxx_READ_DATA							0x03    /* 通过标准SPI读取数据 */
#define W25Qxx_FAST_READ							0x0B    /* 最高 FR 速度 (8.2.12) */
#define W25Qxx_FAST_READ_DUAL_OUTPUT				0x3B    /* 快速读取双 SPI 输出 (8.2.14) */
#define W25Qxx_FAST_READ_DUAL_IO					0xBB    /* 双 SPI I/O 快速读取（地址通过两条线传输） */
#define W25Qxx_FAST_READ_QUAD_OUTPUT				0x6B    /* 快速读取四路 SPI 输出 (8.2.16) */
#define W25Qxx_FAST_READ_QUAD_IO					0xEB    /* 快速读取四路 SPI I/O（地址通过四线传输） */

#define W25Qxx_REGISTER_1_BUSY_BIT                  0x01    /* 1号寄存器的busy位是最低位 */

#define W25Qxx_DUMMY_BYTE							0xFF    /* 任意数据 */

#endif
