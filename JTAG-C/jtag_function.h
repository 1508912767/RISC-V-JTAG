#ifndef __JTAG_FUNCTION_H
#define __JTAG_FUNCTION_H

#include "config.h"

// ALL IO Level must be 1v8
#define TCK_PORT	GPIOB		// 该款LEVEL SHIFT 需要 单MCU侧上拉1K
#define TCK_PIN		GPIO_PIN_6	
#define TMS_PORT	GPIOA		// 该款LEVEL SHIFT 需要 双侧都上拉1K
#define TMS_PIN		GPIO_PIN_4	
#define RST_PORT	GPIOC		// RST
#define RST_PIN		GPIO_PIN_4
#define TDI_PORT	GPIOA		// MISO
#define TDI_PIN		GPIO_PIN_6
// #define TDO_PORT	GPIOA		// P0_7 
// #define TDO_PIN		GPIO_PIN_15
#define TDO_PORT	GPIOC		// IRQ
#define TDO_PIN		GPIO_PIN_5

#define JTAG_MODE		2
#define DELAY_CYCLE		10

#define tck(n)		(n?gpio_on(&mcu_gpio_driver[JTAG_TCK]):gpio_off(&mcu_gpio_driver[JTAG_TCK]))
#define tms(n)		(n?gpio_on(&mcu_gpio_driver[JTAG_TMS]):gpio_off(&mcu_gpio_driver[JTAG_TMS]))
#define rst(n)		(n?gpio_on(&mcu_gpio_driver[JTAG_RST]):gpio_off(&mcu_gpio_driver[JTAG_RST]))
#define tdi(n)		(n?gpio_on(&mcu_gpio_driver[JTAG_TDI]):gpio_off(&mcu_gpio_driver[JTAG_TDI]))
#define tms_in(n)	gpio_get_state(&mcu_gpio_driver[JTAG_TMS], &n)
#define tdo_in(n)	gpio_get_state(&mcu_gpio_driver[JTAG_TDO], &n)

#define DBG_DATA0 0x04
#define DBG_DATA1 0x05
#define DBG_DATA2 0x06
#define DBG_DATA3 0x07
#define DBG_DMCONTROL 0x10
#define DBG_DMSTATUS 0x11
#define DBG_HARTINFO 0x12
#define DBG_HALTSUM1 0x13
#define DBG_HAWINDOWSEL 0x14
#define DBG_HAWINDOW 0x15
#define DBG_ABSTRACTCS 0x16
#define DBG_COMMAND 0x17
#define DBG_ABSTRACTAUTO 0x18
#define DBG_CONFSTRPTR0 0x19
#define DBG_CONFSTRPTR1 0x1a
#define DBG_CONFSTRPTR2 0x1b
#define DBG_CONFSTRPTR3 0x1c
#define DBG_NEXTDM 0x1d
#define DBG_PROGBUF0 0x20
#define DBG_PROGBUF1 0x21
#define DBG_AUTHDATA 0x30
#define DBG_HALTSUM2 0x34
#define DBG_HALTSUM3 0x35
#define DBG_SBADDRESS3 0x37
#define DBG_SBCS 0x38
#define DBG_SBADDRESS0 0x39
#define DBG_SBADDRESS1 0x3a
#define DBG_SBADDRESS2 0x3b
#define DBG_SBDATA0 0x3c
#define DBG_SBDATA1 0x3d
#define DBG_SBDATA2 0x3e
#define DBG_SBDATA3 0x3f
#define DBG_HALTSUM0 0x40

#define CMD_CMDTYPE 24u
#define CMD_AARSIZE 20u
#define CMD_AAMSIZE 20u
#define CMD_POSTEXEC 18u
#define CMD_TRANSFER 17u
#define CMD_WRITE 16u

#define CTRL_HALTREQ 31u
#define CTRL_RESUMEREQ 30u
#define CTRL_HARTRESET 29u
#define CTRL_ACKHAVERESET 28u
#define CTRL_SETRESETHALTREQ 3u
#define CTRL_CLRRESETHALTREQ 2u
#define CTRL_NDMRESET 1u
#define CTRL_DMACTIVE 0u

#define DMS_IMPEBREAK 22u
#define DMS_ALLHAVERESET 19u
#define DMS_ANYHAVERESET 18u
#define DMS_ALLRESUMEACK 17u
#define DMS_ANYRESUMEACK 16u
#define DMS_ALLNONEXISTENT 15u
#define DMS_ANYNONEXISTENT 14u
#define DMS_ALLUNAVAIL 13u
#define DMS_ANYUNAVAIL 12u
#define DMS_ALLRUNNING 11u
#define DMS_ANYRUNNING 10u
#define DMS_ALLHALTED 9u
#define DMS_ANYHALTED 8u
#define DMS_AUTHENTICATED 7u
#define DMS_AUTHBUSY 6u
#define DMS_HASRESETHALTREQ 5u
#define DMS_CONFSTRPTRVALID 4u
#define DMS_VERSION 0u

#define ABSCS_PROGBUFSIZE 24u
#define ABSCS_BUSY 12u
#define ABSCS_CMDERR 8u
#define ABSCS_DATACOUNT 0u

#define SBCS_SBBUSYERROR 22u
#define SBCS_SBBUSY 21u
#define SBCS_SBREADONADDR 20u
#define SBCS_SBACCESS 17u
#define SBCS_SBAUTOINCREMENT 16u
#define SBCS_SBREADONDATA 15u
#define SBCS_SBERROR 12u
#define SBCS_SBASIZE 5u
#define SBCS_SBACCESS128 4u
#define SBCS_SBACCESS64 3u
#define SBCS_SBACCESS32 2u
#define SBCS_SBACCESS16 1u
#define SBCS_SBACCESS8 0u

#define OK 0
#define TIMEOUT 2

#define TST_CTL_TST_KEY_ENTRY_3775       0x60004000
#define TST_CTL_TST_KEY_OBSERVE_3775     0x60004004
#define TST_CTL_TST_KEY_ENTRY_3780       0xA0004000
#define TST_CTL_TST_KEY_OBSERVE_3780     0xA0004004

#define	FLASH_PAGE_SIZE_3775		256
#define	FLASH_SECTOR_SIZE_3775	    4096
#define	FLASH_MEM_SIZE_3775		    (128*1024)
#define	FLASH_ERR_3775              (1<<31)
#define	MCYCLE_FREQ_3775			80000000

#define APB1_BASE_3775			0x60000000
#define APB1_SLAVE6_3775	    0x40000
#define APB1_BASE_3780			0xA0000000
#define APB1_SLAVE6_3780	    0x50000
#define APB1_SLAVE6_BASE_3775   (APB1_BASE_3780 + APB1_SLAVE6_3780)
#define FLASH_BASE_3775			(APB1_SLAVE6_BASE_3775)
#define FLASH_COMD_3775			(FLASH_BASE_3775 + 0x0)
#define FLASH_ADDR_3775			(FLASH_BASE_3775 + 0x4)
#define FLASH_DATN_3775			(FLASH_BASE_3775 + 0x8)
#define FLASH_WDATA_3775		(FLASH_BASE_3775 + 0xc)
#define FLASH_RDATA_3775		(FLASH_BASE_3775 + 0xc)
#define FLASH_CTRL_3775			(FLASH_BASE_3775 + 0x10)
#define FLASH_INTE_3775			(FLASH_BASE_3775 + 0x14)
#define FLASH_INTS_3775			(FLASH_BASE_3775 + 0x18)
#define FLASH_STAT_3775 		(FLASH_BASE_3775 + 0x1c)

#define FLASH_CMD_PP_3775       0x2
#define FLASH_CMD_WREN_3775     0x6
#define FLASH_CMD_WRDI_3775     0x4
#define FLASH_CMD_RDSR_3775     0x5
#define FLASH_CMD_WRSR_3775     0x1
#define FLASH_CMD_RD_3775       0x3
#define FLASH_CMD_SE_3775 		0x20
#define FLASH_CMD_REMS_3775     0x90

typedef enum {
	MEM_SIZE_8 = 0u,
	MEM_SIZE_16,
	MEM_SIZE_32,
	MEM_SIZE_64,
	MEM_SIZE_128
}MEM_ACCESS_SIZE;

typedef enum {
	REG_SIZE_32 = 2u,
	REG_SIZE_64
}REG_ACCESS_SIZE;

typedef enum {
	CPU_NONEXISTENT,
	CPU_UNAVAIL,
	CPU_RUNNING,
	CPU_HALTED
}CPU_STATE;

typedef enum {
	CMDERR_NONE,
	CMDERR_BUSY,
	CMDERR_NOT_SUPPORT,
	CMDERR_EXCEPTION,
	CMDERR_HALT_RESUMT,
	CMDERR_BUS,
	CMDERR_OTHER
}CMD_ERROR_STATE;

typedef enum
{
	INIT,
	TEST,
	GET_MODE,
	SET_MODE,
	CPU_RESUME,
	CPU_HALT,
	ENABLE_DMI,
	CLEAR_CMD_ERROR,
}JtagModeList;

typedef enum
{
    ScanNoregu_two  = 0x2,
    ScanNoregu_four,
    DftEnable_two,
    DftEnable_four,
    ScanDdie_two,
    ScanDdie_four,
    DftSafepwr_two,
    DftSafepwr_four,
    DftSafepwrclk_two,
    DftSafepwrclk_four,
    DftPor_two,
    DftPor_four,
    ScanAdie_two,
    ScanAdie_four,
}JtagStateList;

typedef enum {
	ABS_CMD = 0,
	PRG_BUF = 1,
	SYS_BUS = 2
}AccessModeList;

typedef enum {
	MEMORY,
	REGISTER,
}AccessStateList;

typedef enum 
{
	JTAG_TCK,
	JTAG_TMS,
	JTAG_RST,
	JTAG_TDI,
	JTAG_TDO,
}JtagPinList;

#pragma pack (1)
typedef struct JtagTest
{
    unsigned data_low   :2;
    unsigned data_mid   :32;
    unsigned data_high  :7;
}JtagTestData;
#pragma pack ()

///////////////////////////////////////////
// jtag interface
typedef int (*jtag_read_fn_t)(void*, uint8_t, uint8_t, uint32_t, uint32_t*);
typedef int (*jtag_write_fn_t)(void*, uint8_t, uint8_t, uint32_t, uint32_t);
typedef struct 
{
    jtag_read_fn_t read;
    jtag_write_fn_t write;
}jtag_i;

static inline int jtag_read_data(void *self, uint8_t mode, uint8_t state, uint32_t addr, uint32_t* value)
{
    return (*(jtag_i **)self)->read(self, mode, state, addr, value);
}

static inline int jtag_write_data(void *self, uint8_t mode, uint8_t state, uint32_t addr, uint32_t value)
{
    return (*(jtag_i **)self)->write(self, mode, state, addr, value);
}

///////////////////////////////////////////
// jtag device interface
typedef struct 
{
    jtag_i *interface;
	gpio_i **gpio_interface;
	uint8_t delay_count;
	uint8_t jtag_mode;
}mcu_jtag_t;

///////////////////////////////////////////
// INIT
void Jtag_Function_Init(void);
// jtag send tms tdi
void send_tms_tdi(bool tms_i, bool tdi_i);
void send_tms(bool tms_i);
// State Machine
void jtag_tlr_to_idle(void);
void jtag_idle_to_sld(void);
void jtag_sld_to_cdr(void);
void jtag_cdr_to_sdr(void);
void jtag_sdr_to_sdr(void);
void jtag_sdr_to_e1d(void);
void jtag_e1d_to_pdr(void);
void jtag_pdr_to_e2d(void);
void jtag_e2d_to_sdr(void);
void jtag_e2d_to_udr(void);
void jtag_cdr_to_e1d(void);
void jtag_e1d_to_udr(void);
void jtag_udr_to_sld(void);
void jtag_udr_to_idle(void);
void jtag_sld_to_sli(void);
void jtag_sli_to_cir(void);
void jtag_cir_to_sir(void);
void jtag_sir_to_sir(void);
void jtag_sir_to_e1i(void);
void jtag_e1i_to_uir(void);
void jtag_uir_to_idle(void);
void jtag_uir_to_sld(void);
void jtag_e1i_to_pir(void);
void jtag_pir_to_e2i(void);
void jtag_eir_to_sir(void);
void jtag_exit_to_idle(void);
void jtag_idle_to_shift_ir(void);
void jtag_idle_to_shift_dr(void);
// jtag send dr and ir
void send_ir(uint8_t ir_reg);
void send_dr(JtagTestData* tdi_reg, JtagTestData* tdo_reg, int size);
void jtag_keep_in_idle(int number);
void jtag_send_ir(uint8_t ir_reg);
void jtag_send_dr(JtagTestData* tdi_reg, JtagTestData* tdo_reg, int size);
// Print Key
void StartJtagTask(JtagStateList code);
// jtag function
void jtag_reset(void);
void switch_jtag2cjtag(void);
int get_jtag_wire_mode(void);
// dm register function
void write_dm_register(uint32_t reg_num,  uint32_t reg_value, int* error);
void read_dm_register(uint32_t reg_num,  uint32_t* reg_value, int* error);
void access_dtm_register(uint8_t reg_num, uint32_t tdi_size, uint32_t tdi_data, JtagTestData * tdo_data);
// cpu function
int enable_dmi();
int halt_cpu();
int resume_cpu(); 
// csr function
int read_csr(int reg_num, uint32_t * value, AccessModeList mode); 
int write_csr(int reg_num, uint32_t value, AccessModeList mode);  
// memory function
int read_memory(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size, AccessModeList mode); 
int write_memory(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size, AccessModeList mode); 
int write_memory_bulk_ABS_CMD(uint32_t base_addr, uint32_t *data_array, int len, MEM_ACCESS_SIZE size); 
// debug test function
void dbg_test_code(void);
int clear_cmderr(void);
void dbg_test_init(void);
// flash function
void flash_init(void);
int flash_erase(uint32_t start_addr, uint32_t end_addr);
int flash_write(uint8_t* buffer, uint32_t offset, uint32_t count);
int flash_read(uint8_t* buffer, uint32_t offset, uint32_t count);
// abs prg sys function
int read_register_using_abs_cmd(int reg_value, uint32_t *value, REG_ACCESS_SIZE size);
int write_register_using_abs_cmd(int reg_value, uint32_t value, REG_ACCESS_SIZE size);
int read_memory_using_prg_buf(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size);
int read_memory_using_abs_cmd(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size);
int read_memory_using_sys_bus(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size);
int write_memory_using_prg_buf(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size);
int write_memory_using_abs_cmd(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size);
int write_memory_using_sys_bus(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size);

#endif
