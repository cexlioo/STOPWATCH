#include "flash_index.h"
#include "string.h"
#include "stdio.h"
#include "w25qxx.h"

#define SECTOR_SIZE 4096        // 0x1000
// 索引数据部分
#define INDEX_SECTOR0_START 0x0000
#define INDEX_SECTOR1_START (INDEX_SECTOR0_START + 1 * SECTOR_SIZE) // 0x1000

// 数据项部分
#define TABLE0_START (INDEX_SECTOR1_START + 1 * SECTOR_SIZE)    // 0x2000
#define TABLE1_START (TABLE0_START + 31 * SECTOR_SIZE)      // 0x21000
#define TABLE2_START (TABLE1_START + 31 * SECTOR_SIZE)      // 0x40000
#define TABLE3_START (TABLE2_START + 32 * SECTOR_SIZE)      // 0x60000
#define TABLE4_START (TABLE3_START + 32 * SECTOR_SIZE)      // 0x80000
#define TABLE5_START (TABLE4_START + 32 * SECTOR_SIZE)      // 0xA0000
#define TABLE6_START (TABLE5_START + 32 * SECTOR_SIZE)      // 0xC0000
#define TABLE7_START (TABLE6_START + 32 * SECTOR_SIZE)      // 0xE0000
#define TABLE8_START (TABLE7_START + 32 * SECTOR_SIZE)      // 0x100000
#define TABLE9_START (TABLE8_START + 32 * SECTOR_SIZE)      // 0x120000
#define TABLE10_START (TABLE9_START + 32 * SECTOR_SIZE)     // 0x140000
#define TABLE11_START (TABLE10_START + 32 * SECTOR_SIZE)    // 0x160000
#define TABLE12_START (TABLE11_START + 32 * SECTOR_SIZE)    // 0x180000
#define TABLE13_START (TABLE12_START + 32 * SECTOR_SIZE)    // 0x1A0000
#define TABLE14_START (TABLE13_START + 32 * SECTOR_SIZE)    // 0x1C0000
#define TABLE15_START (TABLE14_START + 32 * SECTOR_SIZE)    // 0x1E0000

// 结束地址，暂未使用
#define FLASH_END (TABLE15_START + 32 * SECTOR_SIZE)            // 0x200000     2MB大小

#define W25QXX_PAGE_SIZE 256            /**< flash页大小 */
#define TABLE_CNT 16                    /**< index区域保存的表数目 */

const static uint32_t table_addr[TABLE_CNT] = {
	TABLE0_START ,
  TABLE1_START ,
  TABLE2_START ,
  TABLE3_START ,
  TABLE4_START ,
  TABLE5_START ,
  TABLE6_START ,
  TABLE7_START ,
  TABLE8_START ,
  TABLE9_START ,
  TABLE10_START,
  TABLE11_START,
  TABLE12_START,
  TABLE13_START,
  TABLE14_START,
  TABLE15_START
};

struct flash_index {                        /**< flash索引结构体，76字节 */
    uint16_t ntables;                       /**< 已写入表的数量，2字节，范围 0~16 */
    uint16_t next_table_i;                  /**< 即将写入的表的下标 2字节，范围 0~15 */
    uint32_t table_ts[TABLE_CNT];           /**< 表的产生时间，也就是骑行开始时间，共16个，每个4字节 */
    uint32_t next_version;                  /**< 即将写入的版本号,4字节，每次累加1 */
    uint8_t  reserved[2];                   /**< 保留2字节 */
    uint8_t  next_no;                       /**< 即将写入的索引数据编号，范围 0~1 */
    uint8_t  write_flag;                    /**< 已写入标记位 0xA5 */
};

static struct flash_index index;

//初始化
static void initialize_index(void)
{
	memset(&index, 0, sizeof(struct flash_index));
	index.write_flag = FLASH_WRITE_FLAG;
	index.next_no = 0;
}

//擦除index需要的扇区
void erase_index(uint32_t sector_addr)
{
	w25qxx_sector_erase(sector_addr);
}

//打印索引里面的内容
void print_index(const char* desc, struct flash_index* index)
{
	int i;
  Log_d("======================= [%s] =========================\n", desc);
    
  Log_d("Index No: %u\n", index->next_no);
  Log_d("Version: %u\n", index->next_version);
  Log_d("Reserved: 0x%02X 0x%02X\n", index->reserved[0], index->reserved[1]);
  Log_d("Write Flag: 0x%02X\n", index->write_flag);
  Log_d("Total Tables: %u\n", index->ntables);
  Log_d("Table Index: %u\n", index->next_table_i);

  for (i = 0; i < TABLE_CNT; i++) 
	{
      Log_d("Table Timestamp[%d]: %u\n", i, index->table_ts[i]);
  }
}

static void set_next_index(void)
{
	//更新索引，设置下一次写入的信息
	index.next_version += 1;
	index.next_table_i = (index.next_table_i + 1) % TABLE_CNT;	//循环更新
	index.next_no = (index.next_no == 0) ? 1 : 0;
	/*
	struct flash_index index0, index1;
	
	memset(&index0, 0, sizeof(struct flash_index));
	memset(&index1, 0, sizeof(struct flash_index));
	
	w25qxx_read_bytes(INDEX_SECTOR0_START, (uint8_t*)&index0, sizeof(struct flash_index));
	w25qxx_read_bytes(INDEX_SECTOR1_START, (uint8_t*)&index1, sizeof(struct flash_index));
	
	print_index("index 0", &index0);
	print_index("index 1", &index1);
	*/
	print_index("next index", &index);
	
}

//索引数据写入flash
static void write_index_to_flash(uint32_t page_addr)
{
	uint8_t buffer[W25QXX_PAGE_SIZE] = {0};
	
	Log_d("write index to 0x%x\n", page_addr);
	
	//print_index("write index", &index);
	memcpy(buffer, &index, sizeof(struct flash_index));
	w25qxx_page_write_bytes(page_addr, buffer, W25QXX_PAGE_SIZE);
	
}

//从flash读取出索引数据
static void read_index_from_flash(void)
{
	struct flash_index index0, index1;
	struct flash_index* new;
	
	memset(&index0, 0, sizeof(struct flash_index));
	memset(&index1, 0, sizeof(struct flash_index));
	
	w25qxx_read_bytes(INDEX_SECTOR0_START, (uint8_t*)&index0, sizeof(struct flash_index));
	w25qxx_read_bytes(INDEX_SECTOR1_START, (uint8_t*)&index1, sizeof(struct flash_index));
	
	print_index("index 0", &index0);
	print_index("index 1", &index1);
	
	if(index0.write_flag == FLASH_WRITE_FLAG && index1.write_flag == FLASH_WRITE_FLAG)//两个索引内容都被写入
	{
		if(index0.next_version == 0xFFFFFFFF)// index 0 的版本被擦除，但flag未擦除，极少发生的情况
		{
			new = &index1;
		}
		else if(index1.next_version == 0xFFFFFFFF)// index 1 的版本被擦除，但flag未擦除，极少发生的情况
		{
			new = &index0;
		}
		else
		{
			new = (index0.next_version > index1.next_version) ? &index0 : &index1;
		}
		memcpy(&index, new, sizeof(struct flash_index));
		set_next_index();
	}
	else if(index0.write_flag == FLASH_WRITE_FLAG)// index 0 被写入了，index 1 未被写入或刚擦掉时断电了，使用 index 0
	{
		memcpy(&index, &index0, sizeof(struct flash_index));
		set_next_index();
		Log_d("use index 0\n");
	}
	else if(index1.write_flag == FLASH_WRITE_FLAG)// index 1 被写入了，index 0 刚擦掉时断电了，使用 index 1
	{
		memcpy(&index, &index1, sizeof(struct flash_index));
		set_next_index();
		Log_d("use index 1\n");
	}
	else // index 0 、index 1 均未被写入，出厂状态，第1次骑行
	{
		initialize_index();
		Log_d("initialize index data\n");
	}
}

void write_index(void)
{
	uint32_t sector_addr;
	sector_addr = (index.next_no == 0) ? INDEX_SECTOR0_START : INDEX_SECTOR1_START;
	
	if(index.ntables < TABLE_CNT)
		index.ntables += 1;
	
	Log_d("erase index: 0x%x\n", sector_addr);
	
	erase_index(sector_addr);
	write_index_to_flash(sector_addr);
	set_next_index();
}

void set_table_created_timestamp(uint32_t ts)
{
	Log_d("create the [%d] table, timestamp:%u\n", index.next_table_i, ts);
	index.table_ts[index.next_table_i] = ts;
}

//n: 1~16
uint32_t get_table_n_created_timestamp(int n)
{
	int idx;
	if(index.ntables == TABLE_CNT)//循环保存了
	{
		idx = (index.next_table_i + n - 1) % TABLE_CNT;
	}
	else//未循环保存
	{
		idx = n - 1;
	}
	return index.table_ts[idx];
}

//n: 1~16
uint32_t get_table_n_addr(int n)
{
	int idx;
	if(index.ntables == TABLE_CNT)//循环保存了
	{
		idx = (index.next_table_i + n - 1) % TABLE_CNT;
	}
	else//未循环保存
	{
		idx = n - 1;
	}
	Log_d("get table [%d] start addr, idx:%d, addr:0x%x\n", n, idx, table_addr[idx]);
	return table_addr[idx];
}

uint32_t get_next_table_addr(void)
{
	return table_addr[index.next_table_i];
}

uint16_t get_table_cnt(void)
{
	return index.ntables;
}

void init_index(void)
{
	read_index_from_flash();
}
