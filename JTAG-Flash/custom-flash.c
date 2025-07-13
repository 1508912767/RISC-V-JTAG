#include "stdarg.h"
#include "flash.h"

#define	LOG_ABLE			0

#define	FLASH_PAGE_SIZE		256
#define	FLASH_SECTOR_SIZE	4096
#define	FLASH_MEM_SIZE		(128*1024)
#define	FLASH_ERR			(1<<31)
#define	MCYCLE_FREQ			80000000

#define APB1_BASE			0x60000000
#define APB1_SLAVE6			0x40000
#define APB1_SLAVE6_BASE	(APB1_BASE + APB1_SLAVE6)
#define FLASH_BASE			(APB1_SLAVE6_BASE)
#define FLASH_COMD			(FLASH_BASE + 0x0)
#define FLASH_ADDR			(FLASH_BASE + 0x4)
#define FLASH_DATN			(FLASH_BASE + 0x8)
#define FLASH_WDATA			(FLASH_BASE + 0xc)
#define FLASH_RDATA			(FLASH_BASE + 0xc)
#define FLASH_CTRL			(FLASH_BASE + 0x10)
#define FLASH_INTE			(FLASH_BASE + 0x14)
#define FLASH_INTS 			(FLASH_BASE + 0x18)
#define FLASH_STAT 			(FLASH_BASE + 0x1c)

#define FLASH_CMD_PP 		0x2
#define FLASH_CMD_WREN 		0x6
#define FLASH_CMD_WRDI 		0x4
#define FLASH_CMD_RDSR 		0x5
#define FLASH_CMD_WRSR 		0x1
#define FLASH_CMD_RD 		0x3
#define FLASH_CMD_SE 		0x20
#define FLASH_CMD_REMS 		0x90

#ifndef	reg_read
#define	reg_read(addr)		(* ((volatile unsigned int *) (addr)))
#endif
#ifndef	reg_write
#define	reg_write(addr,val)	(* ((volatile unsigned int *) (addr))) = (val)
#endif
#ifndef read_csr
#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })
#endif

//------------------------------------------------------------------------
#if LOG_ABLE
static void UartOut(int val)
{
	if(val)
		reg_write(0x3000005C,0x00000001);
	else
		reg_write(0x3000005C,0x00000000);
}

static void UartWait()
{
	unsigned int ts = read_csr(mcycle);
	asm("nop");
	unsigned int dset = MCYCLE_FREQ / 115200 - 38;
	unsigned int dnow;
	do
		dnow = read_csr(mcycle) - ts;
	while(dnow <= dset);
}

static void LogInit()
{
	UartOut(1);
	int i;
	for(i=0;i<16;i++)
		UartWait();
}

static void Myputchar(char a)
{//这个时序可能被中断打断，导致输出乱码
	int i = 8;
	UartOut(0);//发送启始位
	UartWait();
	while(i--)//发送8位数据位
	{
		UartOut(a & 0x01);//先传低位
		UartWait();
		a = a >> 1;
	}
	UartOut(1);//发送结束位
	UartWait();
}

static void LogInt(int a)
{
	int i, t;
	for(i=0;i<8;i++)
	{
		t = (a>>(28-i*4)) & 0xf;
		if(t < 10)
			Myputchar(t + '0');
		else
			Myputchar(t + 'A' - 10);
	}
}

static void LogInt2(int flag, int a, int b)
{
	Myputchar((flag>>0) & 0xff);
	Myputchar((flag>>8) & 0xff);
	LogInt(a);
	Myputchar(',');
	LogInt(b);
	Myputchar('\n');
}
#else
static void LogInit()
{
}
static void LogInt(int a)
{
}
static void LogInt2(int flag, int a, int b)
{
}
static void Myputchar(char a)
{
}
#endif
//--------------------------------------------------------------------------------
static inline void DelayCmd()
{
	asm("nop");asm("nop");asm("nop");asm("nop");
	asm("nop");asm("nop");asm("nop");asm("nop");
	asm("nop");asm("nop");asm("nop");asm("nop");
	asm("nop");asm("nop");asm("nop");asm("nop");
}

int flash_init(uint32_t *spi_base)
{//Flash初始化，入口参数是指定SPI的地址，3775用不上
//    asm("csrw 0x7ca, 0x1");
//    asm("fence.i");
	LogInit();
	reg_write(FLASH_INTS,0xFFFFFFFF);
//	GD_LOGD("flash init");
    return 0;//返回chipid
}

int flash_erase(uint32_t *spi_base, uint32_t start_addr, uint32_t end_addr)
{
//	GD_LOGD("flash earse %08x %08x", start_addr, end_addr);
	LogInt2('S' + (':'<<8), start_addr, end_addr);
	if(start_addr & 0xfff)
	{//简化问题，只做sector对齐的
//		GD_LOGD("ERR erase addr");
		return FLASH_ERR | start_addr;
	}
	uint32_t rdata,wip;
    uint32_t sector_num = (end_addr - start_addr + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE;
    if(sector_num > (FLASH_MEM_SIZE + 1*1024*1024) / FLASH_SECTOR_SIZE)
	{//设一个最大值
		return FLASH_ERR | sector_num;
	}
	reg_write(FLASH_INTS,0xFFFFFFFF);
    DelayCmd();
    /* erase flash */
    reg_write(FLASH_CTRL,0x00000000);//disable cache
    DelayCmd();
    for (int i = 0;i < sector_num;i++) 
	{
		reg_write(FLASH_COMD,(0x80000000 | FLASH_CMD_WREN));//FLASH WREN
		asm("nop");
		//erase need 100-400ms
		reg_write(FLASH_ADDR, start_addr + i*FLASH_SECTOR_SIZE);//整片擦除时无效
		reg_write(FLASH_COMD,(0x80000000 | 0x20));//60擦整片，20擦SPIFLASH_SECTOR_SIZE
		asm("nop");
		rdata = reg_read(FLASH_COMD);
		while(rdata & 0x80000000)
		{
			rdata = reg_read(FLASH_COMD);
		}
		wip=0x1;
		reg_write(FLASH_INTS,0xFFFFFFFF);
		while(wip & 0x00000001)
		{ //wip=0 means erase finish
			reg_write(FLASH_COMD,(0x80000000 | FLASH_CMD_RDSR));
			asm("nop");
			while((reg_read(FLASH_INTS) & 0x4) == 0x0);
			wip = reg_read(FLASH_RDATA);
			reg_write(FLASH_INTS,0xFFFFFFFF);
		}
	}
    DelayCmd();
    reg_write(FLASH_CTRL,0x00000001);//enable cache
//	GD_LOGD("flash earse return");
    return 0;
}

void flash_erase_end()
{
}

int flash_write(uint32_t *spi_base, uint8_t* buffer, uint32_t offset, uint32_t count)
{
//	LogInt2('W' + (':'<<8), offset, count);
//	//LogInt2('D' + ('T'<<8), ((uint32_t*)buffer)[0x14/4], ((uint32_t*)buffer)[0x18/4]);
//	if((((uint32_t)buffer) & 3))
//	{//先简化问题，只做4字节对其的
//		LogInt2('E' + (':'<<8), offset, (int)buffer);
//		return FLASH_ERR | (uint32_t)buffer;
//	}
	int write_count = 0;
	unsigned int i, rdata, wip;
    uint32_t cur_offset = offset % FLASH_PAGE_SIZE;
    uint32_t cur_count = 0;
    /* write flash */
    reg_write(FLASH_CTRL,0x00000000);//disable cache
    DelayCmd();
    DelayCmd();
    while ((int)count > 0)
	{
        if ((cur_offset + count) >= FLASH_PAGE_SIZE)
		{
            cur_count = FLASH_PAGE_SIZE - cur_offset;
        }
		else
		{
            cur_count = count;
			if(cur_count > FLASH_PAGE_SIZE)
				cur_count = FLASH_PAGE_SIZE;
        }
        cur_offset = 0;
		//--------------------------------------
		for(i=0;i<4 && i<cur_count/4;i++)
		{//write 4*4byte into wfifo
			reg_write(FLASH_WDATA,((uint32_t*)buffer)[i]);
		}
		reg_write(FLASH_ADDR,offset);//program address      
		reg_write(FLASH_DATN,cur_count-1);//program length 256Byte
		reg_write(FLASH_COMD,(0x80000000 | FLASH_CMD_WREN)); //FLASH WREN , erase will reset WEL
		asm("nop");
		reg_write(FLASH_COMD,(0x80000000 | FLASH_CMD_PP));   //FLASH PRO
		for(;i<cur_count/4;i++)
		{//check wfifo not almast full, write 1*4byte into wfifo
			while((reg_read(FLASH_INTS) & 0x20) == 0x0);
			reg_write(FLASH_WDATA,((uint32_t*)buffer)[i]);
		}
		asm("nop");
		rdata = reg_read(FLASH_COMD);
		while(rdata & 0x80000000)
		{
			rdata = reg_read(FLASH_COMD);
		}
		wip=0x1;
		reg_write(FLASH_INTS,0xFFFFFFFF);
		while(wip & 0x00000001)
		{ //wip=0 means program finish
			reg_write(FLASH_COMD,(0x80000000 | FLASH_CMD_RDSR));
			while((reg_read(FLASH_INTS) & 0x4) == 0x0);
			wip = reg_read(FLASH_RDATA);
			reg_write(FLASH_INTS,0xFFFFFFFF);
		}
		//--------------------------------------
        buffer += cur_count;
        offset += cur_count;
        count -= cur_count;
        write_count += cur_count;
    }
    DelayCmd();
    reg_write(FLASH_CTRL,0x00000001);//enable cache
//	LogInt2('W' + ('n'<<8), write_count, (int)buffer);
    return 0;
}

void flash_write_end()
{
}

int flash_read(uint32_t *spi_base, uint8_t* buffer, uint32_t offset, uint32_t count)
{
//	GD_LOGD("flash read %08x %08x", offset, count);
	LogInt2('R' + (':'<<8), offset, count);
	if((((uint32_t)buffer) & 3) | (offset & 3))
	{//先简化问题，只做4字节对其的
		LogInt2('E' + (':'<<8), offset, (int)buffer);
		return FLASH_ERR | (uint32_t)buffer;
	}
	uint32_t i;	//, rdata;
	uint32_t *buf_int;
	uint32_t cur_count;
	uint32_t cur_len;
    reg_write(FLASH_CTRL, 0x00000000);//disable cache
    DelayCmd();
	while(count > 0)
	{
		if(count > FLASH_PAGE_SIZE)//256
			cur_count = FLASH_PAGE_SIZE;
		else
			cur_count = count;
		buf_int = (uint32_t *) buffer;
		cur_len = (cur_count + 3) / 4;

	    reg_write(FLASH_ADDR, offset );//read address
		reg_write(FLASH_DATN, cur_len * 4 - 1);//read length 256byte
		reg_write(FLASH_COMD,(0x80000000 | FLASH_CMD_RD));//FLASH RD
		//reg_write(FLASH_COMD,(0x80000000 | 0x3b));//dual output
		for( i=0;i<cur_len;i++)
		{
			while((reg_read(FLASH_INTS) & 0x4) == 0x0);
			buf_int[i] = reg_read(FLASH_RDATA);
		}

		count -= cur_count;
		offset += cur_count;
		buffer += cur_count;
	}
	DelayCmd();
    reg_write(FLASH_CTRL,0x00000001);//enable cache
//	GD_LOGD("flash read return");
    return 0;
}
