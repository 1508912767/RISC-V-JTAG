#include "jtag_function.h"

uint8_t cjtag_mode = 0;
uint8_t tms_in_val = 1;
uint8_t tdo_in_val = 0;

///////////////////////////////////////////
// jtag interface
static int jtag_read_sensor_data(mcu_jtag_t *self, uint8_t mode, uint8_t state, uint32_t addr, uint32_t* value)
{
    int error;    
    if(mode == SYS_BUS && state == MEMORY)
    {
        read_memory(addr, value, MEM_SIZE_32, SYS_BUS);
    }
    else if(mode == SYS_BUS && state == REGISTER)
    {
        read_dm_register(addr, value, &error);
    }
    else if(mode == ABS_CMD && state == MEMORY)
    {
        read_memory(addr, value, MEM_SIZE_32, ABS_CMD);
    }
    else if(mode == ABS_CMD && state == REGISTER)
    {
        read_register_using_abs_cmd(addr, value, REG_SIZE_32);
    }
    return error;
}

static int jtag_write_sensor_data(mcu_jtag_t *self, uint8_t mode, uint8_t state, uint32_t addr, uint32_t value)
{      
    int error;
    if(mode == SYS_BUS && state == MEMORY)
    {
        write_memory(addr, value, MEM_SIZE_32, SYS_BUS);
    }
    else if(mode == SYS_BUS && state == REGISTER)
    {
        write_dm_register(addr, value, &error);
    }
    else if(mode == ABS_CMD && state == MEMORY)
    {
        write_memory(addr, value, MEM_SIZE_32, ABS_CMD);
    }
    else if(mode == ABS_CMD && state == REGISTER)
    {
        write_register_using_abs_cmd(addr, value, REG_SIZE_32);
    }
    return error;
}

///////////////////////////////////////////
// mcu gpio driver
static gpio_i mcu_gpio_interface = 
{
	.init = (gpio_init_fn_t)mcu_gpio_init,
	.on = (gpio_on_fn_t)mcu_gpio_on,
	.off = (gpio_off_fn_t)mcu_gpio_off,
	.get_state = (gpio_get_state_fn_t)mcu_gpio_get_state,
	.toggle = (gpio_toggle_fn_t)mcu_gpio_toggle,
}; 
static mcu_gpio_t mcu_gpio_driver[] = 
{
	{
		.interface = &mcu_gpio_interface,
		.port = TCK_PORT,
		.pin = TCK_PIN,
	},
	{
    	.interface = &mcu_gpio_interface,
		.port = TMS_PORT,
		.pin = TMS_PIN,
	},
	{
		.interface = &mcu_gpio_interface,
		.port = RST_PORT,
		.pin = RST_PIN,
	},
    {
		.interface = &mcu_gpio_interface,
		.port = TDI_PORT,
		.pin = TDI_PIN,
	},
    {
		.interface = &mcu_gpio_interface,
		.port = TDO_PORT,
		.pin = TDO_PIN,
	},
};
static jtag_i jtag_interface = 
{
    .read = (jtag_read_fn_t)jtag_read_sensor_data,
    .write = (jtag_write_fn_t)jtag_write_sensor_data,
};

static mcu_jtag_t mcu_jtag_driver = 
{
    .interface = &jtag_interface,
    .gpio_interface = (gpio_i **)&mcu_gpio_driver,
    .delay_count = DELAY_CYCLE,    // 25 -> 800K, 10 -> 1.5M
    .jtag_mode = JTAG_MODE,
};

static void delay_cycle(void)
{   
    for (int i = 0; i < mcu_jtag_driver.delay_count; i++)
    {
        __NOP();
    }
}

///////////////////////////////////////////
// INIT
void Jtag_Function_Init(void)
{
    for (int i = 0; i < 5; i++)
    {
        gpio_off(&mcu_gpio_driver[i]);
    }

    gpio_init(&mcu_gpio_driver[JTAG_TCK], GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    gpio_init(&mcu_gpio_driver[JTAG_RST], GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);

    if(mcu_jtag_driver.jtag_mode == 4)
    {
        gpio_init(&mcu_gpio_driver[JTAG_TMS], GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        gpio_init(&mcu_gpio_driver[JTAG_TDI], GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
        gpio_init(&mcu_gpio_driver[JTAG_TDO], GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH);
    }
    else
    {
        gpio_init(&mcu_gpio_driver[JTAG_TMS], GPIO_MODE_OUTPUT_OD, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
    }
}

// i3c public function
void send_tms_tdi(bool tms_i, bool tdi_i)
{
    if(cjtag_mode)
    {
        if(tdi_i==0)
        {
            tms(1);
        }
        else
        {
            tms(0);
        }

        
        delay_cycle();
        tck(1);
        delay_cycle();
        tck(0);
        
        tms(tms_i);
        
        delay_cycle();
        tck(1);
        delay_cycle();
        tck(0);

        tms(0);

        // 这里好像也要加
        tms(1);
        delay_cycle();
        // 这里好像也要加
        tms(0);

        tck(1);
        delay_cycle();
        tck(0);
    }
    else{
        tms(tms_i);
        tdi(tdi_i);

        // 四线放到这里提前一个时钟去读
        // tdo_in_val = tdo_in;    
        tdo_in(tdo_in_val);    

        delay_cycle();
        tck(1);
        delay_cycle();
        tck(0);
    }
}

void send_tms(bool tms_i)
{
    tms(tms_i);
    delay_cycle();
    tck(1);
    delay_cycle();
    tck(0);
}

void jtag_tlr_to_idle(void)
{
    send_tms_tdi(0,0);
}

void jtag_idle_to_sld(void)
{
    send_tms_tdi(1,0);
}

void jtag_sld_to_cdr(void)
{
    send_tms_tdi(0,0);
}

void jtag_cdr_to_sdr(void)
{
    send_tms_tdi(0,0);
}

void jtag_sdr_to_sdr(void)
{
    send_tms_tdi(0,0);
}

void jtag_sdr_to_e1d(void)
{
    send_tms_tdi(1,0);
}

void jtag_e1d_to_pdr(void)
{
    send_tms_tdi(0,0);
}

void jtag_pdr_to_e2d(void)
{
    send_tms_tdi(1,0);
}

void jtag_e2d_to_sdr(void)
{
    send_tms_tdi(0,0);
}

void jtag_e2d_to_udr(void)
{
    send_tms_tdi(1,0);
}

void jtag_cdr_to_e1d(void)
{
    send_tms_tdi(1,0);
}

void jtag_e1d_to_udr(void)
{
    send_tms_tdi(1,0);
}

void jtag_udr_to_sld(void)
{
    send_tms_tdi(1,0);
}

void jtag_udr_to_idle(void)
{
    send_tms_tdi(0,0);
}

void jtag_sld_to_sli(void)
{
    send_tms_tdi(1,0);
}

void jtag_sli_to_cir(void)
{
    send_tms_tdi(0,0);
}

void jtag_cir_to_sir(void)
{
    send_tms_tdi(0,0);
}

void jtag_sir_to_sir(void)
{
    send_tms_tdi(0,0);
}

void jtag_sir_to_e1i(void)
{
    send_tms_tdi(1,0);
}

void jtag_e1i_to_uir(void)
{
    send_tms_tdi(1,0);
}

void jtag_uir_to_idle(void)
{
    send_tms_tdi(0,0);
}

void jtag_uir_to_sld(void)
{
    send_tms_tdi(1,0);
}

void jtag_e1i_to_pir(void)
{
    send_tms_tdi(0,0);
}

void jtag_pir_to_e2i(void)
{
    send_tms_tdi(1,0);
}

void jtag_eir_to_sir(void)
{
    send_tms_tdi(0,0);
}

void jtag_exit_to_idle(void)
{
    //send_tms_tdi(0);  // enter pause-dr
    //send_tms_tdi(1);  // enter exit2-dr
    send_tms_tdi(1,0);  // enter update-dr
    send_tms_tdi(0,0);  // enter ilde
}

void jtag_idle_to_shift_ir(void)
{
    send_tms_tdi(1,0);  // enter select-DR scan
    send_tms_tdi(1,0);  // enter select-IR scan
    send_tms_tdi(0,0);  // enter capture-ir 
    send_tms_tdi(0,0);  // enter shift-ir
}

void jtag_idle_to_shift_dr(void)
{
    send_tms_tdi(1,0);  // enter select-DR scan
    send_tms_tdi(0,0);  // enter capture-dr
    send_tms_tdi(0,0);  // enter shift-ir 
}

void send_ir(uint8_t ir_reg)
{
    for (int i = 0; i < 4; i++)
    {
        send_tms_tdi(0, (ir_reg >> i) & 0x01);
    }
    send_tms_tdi(1, (ir_reg >> 4) & 0x01);
}

void send_dr(JtagTestData* tdi_reg, JtagTestData* tdo_reg, int size)
{
    JtagTestData temp_tdo_reg;
    temp_tdo_reg.data_low = 0;
    temp_tdo_reg.data_mid = 0;
    temp_tdo_reg.data_high = 0;

    if(cjtag_mode)
    {        
        // current in shift-dr state
        for (int i = 0; i < size-1; i++)
        {
            // tms = ~tdi_reg[i];
            if(i>=0 && i<2)
            {
                if(((tdi_reg->data_low>>(i-0))&0x01)==1)
                {
                    tms(0);
                }
                else
                {
                    tms(1);
                }
            }
            else if (i>=2 && i<34)
            {
                if(((tdi_reg->data_mid>>(i-2))&0x01)==1)
                {
                    tms(0);
                }
                else
                {
                    tms(1);
                }
            }
            else if(i>=34 && i<41)
            {
                if(((tdi_reg->data_high>>(i-34)&0x01))==1)
                {
                    tms(0);
                }
                else
                {
                    tms(1);
                }
            }

            delay_cycle();
            tck(1);
            delay_cycle();
            tck(0);
            
            tms(0);

            delay_cycle();
            tck(1);
            delay_cycle();
            tck(0);

            tms(0);

            tms(1);
            // temp_tdo_reg = {tms_in, temp_tdo_reg[40:1]};
            temp_tdo_reg.data_low = (temp_tdo_reg.data_low >> 1) + ((temp_tdo_reg.data_mid & 1) << (2-1));
            temp_tdo_reg.data_mid = (temp_tdo_reg.data_mid >> 1) + ((temp_tdo_reg.data_high & 1) << (32-1)); // 所有的1都表示[40:1]中的1
            // tms_in_val = tms_in;
            tms_in(tms_in_val);
            temp_tdo_reg.data_high = (temp_tdo_reg.data_high >> 1) + (tms_in_val << (7-1));

            delay_cycle();
            // tms(0);
            tck(1);

            delay_cycle();
            tck(0);
        }

        // tms = ~tdi_reg[size-1];
        if((size-1)>=0 && (size-1)<2)
        {
            if(((tdi_reg->data_low>>((size-1)-0))&0x01)==1)
            {
                tms(0);
            }
            else
            {
                tms(1);
            }
        }
        else if ((size-1)>=2 && (size-1)<34)
        {
            if(((tdi_reg->data_mid>>((size-1)-2))&0x01)==1)
            {
                tms(0);
            }
            else
            {
                tms(1);
            }
        }
        else if((size-1)>=34 && (size-1)<41)
        {
            if(((tdi_reg->data_high>>((size-1)-34)&0x01))==1)
            {
                tms(0);
            }
            else
            {
                tms(1);
            }
        }
        
        delay_cycle();
        tck(1);
        delay_cycle();
        tck(0);

        tms(1);

        delay_cycle();
        tck(1);
        delay_cycle();
        tck(0);

        tms(0);

        tms(1);
        // temp_tdo_reg = {tms_in, temp_tdo_reg[40:1]};
        temp_tdo_reg.data_low = (temp_tdo_reg.data_low >> 1) + ((temp_tdo_reg.data_mid & 1) << (2-1));
        temp_tdo_reg.data_mid = (temp_tdo_reg.data_mid >> 1) + ((temp_tdo_reg.data_high & 1) << (32-1)); // 所有的1都表示[40:1]中的1
        // tms_in_val = tms_in;
        tms_in(tms_in_val);
        temp_tdo_reg.data_high = (temp_tdo_reg.data_high >> 1) + (tms_in_val << (7-1));
        
        delay_cycle();
        // tms(0);
        tck(1);

        delay_cycle();
        tck(0);
    }
    else
    {
        for(int i=0; i<size-1; i++)
        {
            // send_tms_tdi(0, tdi_reg[i]);
            if(i>=0 && i<2)
            {
                if(((tdi_reg->data_low>>(i-0))&0x01)==1)
                {
                    send_tms_tdi(0, 1);
                }
                else
                {
                    send_tms_tdi(0, 0);
                }
            }
            else if (i>=2 && i<34)
            {
                if(((tdi_reg->data_mid>>(i-2))&0x01)==1)
                {
                    send_tms_tdi(0, 1);
                }
                else
                {
                    send_tms_tdi(0, 0);
                }
            }
            else if(i>=34 && i<41)
            {
                if(((tdi_reg->data_high>>(i-34)&0x01))==1)
                {
                    send_tms_tdi(0, 1);
                }
                else
                {
                    send_tms_tdi(0, 0);
                }
            }
            // temp_tdo_reg = {jtag_tdo, temp_tdo_reg[40:1]};
            // 四线的放到这里读，数据会偏移一个bit, 所以要提前一个时钟去读，把读的放到send_tms_tdi里面就OK了
            // tdo_in_val = tdo_in;    
            temp_tdo_reg.data_low = (temp_tdo_reg.data_low >> 1) + ((temp_tdo_reg.data_mid & 1) << (2-1));
            temp_tdo_reg.data_mid = (temp_tdo_reg.data_mid >> 1) + ((temp_tdo_reg.data_high & 1) << (32-1)); // 所有的1都表示[40:1]中的1
            temp_tdo_reg.data_high = (temp_tdo_reg.data_high >> 1) + (tdo_in_val << (7-1)); 
        }
        // last send tms 1 

        // send_tms_tdi(1, tdi_reg[size-1]);
        if((size-1)>=0 && (size-1)<2)
        {
            if(((tdi_reg->data_low>>((size-1)-0))&0x01)==1)
            {
                send_tms_tdi(1, 1);
            }
            else
            {
                send_tms_tdi(1, 0);
            }
        }
        else if ((size-1)>=2 && (size-1)<34)
        {
            if(((tdi_reg->data_mid>>((size-1)-2))&0x01)==1)
            {
                send_tms_tdi(1, 1);
            }
            else
            {
                send_tms_tdi(1, 0);
            }
        }
        else if((size-1)>=34 && (size-1)<41)
        {
            if(((tdi_reg->data_high>>((size-1)-34)&0x01))==1)
            {
                send_tms_tdi(1, 1);
            }
            else
            {
                send_tms_tdi(1, 0);
            }
        }
        // temp_tdo_reg = {jtag_tdo, temp_tdo_reg[40:1]};
        // 四线的放到这里读，数据会偏移一个bit, 所以要提前一个时钟去读，把读的放到send_tms_tdi里面就OK了
        // tdo_in_val = tdo_in;    
        temp_tdo_reg.data_low = (temp_tdo_reg.data_low >> 1) + ((temp_tdo_reg.data_mid & 1) << (2-1));
        temp_tdo_reg.data_mid = (temp_tdo_reg.data_mid >> 1) + ((temp_tdo_reg.data_high & 1) << (32-1)); // 所有的1都表示[40:1]中的1
        temp_tdo_reg.data_high = (temp_tdo_reg.data_high >> 1) + (tdo_in_val << (7-1)); 
    }

    // tdo_reg = temp_tdo_reg
    if(size == 41)
    {
        tdo_reg->data_low = temp_tdo_reg.data_low;
        tdo_reg->data_mid = temp_tdo_reg.data_mid;
        tdo_reg->data_high = temp_tdo_reg.data_high;
    }
    // tdo_reg = temp_tdo_reg >> 9
    else if(size == 32)
    {
        tdo_reg->data_low = (temp_tdo_reg.data_mid >> (9-2)) & 0x03;
        tdo_reg->data_mid = (temp_tdo_reg.data_mid >> 9) + (temp_tdo_reg.data_high << (32-9));
        tdo_reg->data_high = 0;
    }
    else
    {
        printf("No condition");
    }
}

void jtag_keep_in_idle(int number)
{
    for (int i = 0; i < number; i++)
    {
        send_tms_tdi(0, 0);
    }
}

void jtag_send_ir(uint8_t ir_reg)
{
    send_ir(ir_reg);
}

void jtag_send_dr(JtagTestData* ir_reg, JtagTestData* tdo_reg, int size)
{
    send_dr(ir_reg, tdo_reg, size);
}

void jtag_reset(void)
{
    for (int i = 0; i < 5; i++)
    {
        send_tms_tdi(1,0);
    }
    if(mcu_jtag_driver.jtag_mode == 4)
    {
        send_tms_tdi(0,0);// 不能这样写，不然GPIO会多翻转一次
    }
    else
    {
        tms(0);
        delay_cycle();
        tck(1);
        delay_cycle();
    }
}

void switch_jtag2cjtag(void)
{
    JtagTestData tdi_reg;
    JtagTestData cjtag_data_o;
    cjtag_mode = 0;
    jtag_reset();
    tck(1);
    for (int i = 0; i < 8; i++)
    {
        delay_cycle();
        tms(1);
        delay_cycle();
        if(i!=7)    // 防止多翻转一次
        {
            tms(0);
        }
    }
    tck(0);
    for (int i = 0; i < 50; i++)
    {
        send_tms(1);
    }

    jtag_tlr_to_idle();

    // send zbs0
    jtag_idle_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_e1d();
    jtag_e1d_to_udr();

    // send zbs1
    jtag_udr_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_e1d();
    jtag_e1d_to_udr();

    // cmd2
    jtag_udr_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_sdr();
    jtag_sdr_to_e1d();
    jtag_e1d_to_udr();

    // cmd2-cp0-3
    jtag_udr_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_sdr();
    for (int i = 0; i < 2; i++)
    {
        jtag_sdr_to_sdr();
    }
    jtag_sdr_to_e1d();
    jtag_e1d_to_udr();
    
    // cmd2-cp0-3
    jtag_udr_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_sdr();
    for (int i = 0; i < 8; i++)
    {
        jtag_sdr_to_sdr();
    }
    jtag_sdr_to_e1d();
    jtag_e1d_to_udr();
    jtag_udr_to_idle();

    // check command
    for (int i = 0; i < 4; i++)
    {
        send_tms(0);
    }
    
    cjtag_mode = 1;

    // read rdback0 
    jtag_idle_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_sdr();
    for (int i = 0; i < 8; i++)
    {
        jtag_sdr_to_sdr();
    }   
    jtag_sdr_to_e1d();
    jtag_e1d_to_udr();
    jtag_udr_to_idle();

    jtag_idle_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_e1d();
    jtag_e1d_to_udr();
    jtag_udr_to_idle();

    jtag_idle_to_sld();
    jtag_sld_to_cdr();
    jtag_cdr_to_sdr();

    tdi_reg.data_low = 0;
    tdi_reg.data_mid = 0;
    tdi_reg.data_high = 0;
    struct JtagTest *p = &tdi_reg;
    struct JtagTest *q = &cjtag_data_o;
    send_dr(p, q, 32);
    // $display("rdback0: %h @%d", cjtag_data_o, $time());
    if((uint32_t)((q->data_mid<<2) + q->data_low) != 0xa0684009)
    {
        // 出问题的话，reset状态，SHUTDOWN引脚记得先拔掉
        // LED_Toggle(LED1);
    }

    jtag_e1d_to_udr();
    jtag_udr_to_idle();
    // check command
    for (int i = 0; i < 4; i++)
    {
        send_tms(0);
    }
    // quit command
    jtag_idle_to_sld();
    jtag_sld_to_sli();
    jtag_sli_to_cir();
    jtag_cir_to_sir();
    jtag_sir_to_e1i();
    jtag_e1i_to_uir();
    jtag_uir_to_idle();

    jtag_keep_in_idle(5);
}

void write_dm_register(uint32_t reg_num,  uint32_t reg_value, int* error)
{ 
    JtagTestData tdo_reg;
    JtagTestData tdi_reg;
    struct JtagTest *p = &tdi_reg;
    struct JtagTest *q = &tdo_reg;

    // tck is low, tms is low	
    uint8_t addr = reg_num & 0x7F; 
    uint32_t data = reg_value;

    // bit [40:0] tdi_reg = {addr, data, 2'b10};
    // bit [40:0] tdo_reg;
    p->data_low = 0x2;
    p->data_mid = data;
    p->data_high = addr;

    int timeout = 0;

    // write ir 0x11, mean access dmi register
    jtag_idle_to_shift_ir();

    // tdo_in_val = tdo_in;

    // send_ir(5'h11);
    send_ir(0x11);
    // back to idle
    jtag_exit_to_idle();
    // write dmi register, launch one dm register access
    jtag_idle_to_shift_dr();

    // send_dr(tdi_reg, tdo_reg, 41);

    send_dr(p, q, 41);

    // get the response
    // tdi_reg = {7'h0, 32'h0, 2'b0};
    p->data_low = 0;
    p->data_mid = 0;
    p->data_high = 0;

    // get the write response
    jtag_exit_to_idle();
    jtag_keep_in_idle(10);
    jtag_idle_to_shift_dr();

    // send_dr(tdi_reg, tdo_reg, 41);
    send_dr(p, q, 41);

    switch (q->data_low)
    {
        case 0x0:// write pass
            *error = 0;
            break;
        case 0x1:// reserve
            *error = 1;
            break;
        case 0x2:// write error
            *error = 2;
            break;
        case 0x3:// 2'b11, busy, need next read
            timeout += 1;
            if (timeout > 10) 
            {
                *error = 3;
                break;
            }
        default:
            *error = 4;
            break;
    }
    // back to idle
    jtag_exit_to_idle();
    jtag_keep_in_idle(10);
}

void read_dm_register(uint32_t reg_num,  uint32_t* reg_value, int* error)
{
    // bit [6:0] addr = reg_num[6:0];
    // bit [40:0] tdi_reg = {addr, 32'b0, 2'b01};
    // bit [40:0] tdo_reg;
    JtagTestData tdo_reg;
    JtagTestData tdi_reg;
    struct JtagTest *p = &tdi_reg;
    struct JtagTest *q = &tdo_reg;

    uint8_t addr = reg_num & 0x7F; 
    p->data_low = 0x1;
    p->data_mid = 0;
    p->data_high = addr;

    int timeout = 0;

    // write ir 0x11, mean access dmi register
    jtag_idle_to_shift_ir();
    send_ir(0x11);

    // back to idle
    jtag_exit_to_idle();
    // write dmi register, launch one dm register access
    jtag_idle_to_shift_dr();

    // send_dr(tdi_reg, tdo_reg, 41);
    send_dr(p, q, 41);
    
    // get the response
    // tdi_reg = {7'h0, 32'h0, 2'b0};
    p->data_low = 0;
    p->data_mid = 0;
    p->data_high = 0;
    
    // write dmi 0, get the response
    jtag_exit_to_idle();
    jtag_keep_in_idle(10);
    jtag_idle_to_shift_dr();

    // send_dr(tdi_reg, tdo_reg, 41);
    send_dr(p, q, 41);

    switch (q->data_low)
    {
        case 0x0://read pass
            // value = tdo_reg[33:2];
            *reg_value = q->data_mid;
            *error = 0;
            break;
        case 0x1:// reserve
            *error = 1;
            break;
        case 0x2:// read error
            *error = 2;
            break;
        case 0x3:// busy
            timeout += 1;
            if (timeout > 10) 
            {
                *error = 3;
                break;
            }
        default:
            *error = 4;
            break;
    }

    // enter idle
    jtag_exit_to_idle();
    jtag_keep_in_idle(10);
}

void access_dtm_register(uint8_t reg_num, uint32_t tdi_size, uint32_t tdi_data, JtagTestData * tdo_data)
{
    JtagTestData dr_data_i;
    JtagTestData dr_data_o;
    JtagTestData tmp_data;
    struct JtagTest *p = &dr_data_i;
    struct JtagTest *q = &dr_data_o;

    // bit[40:0] dr_data_i;
    // bit[40:0] dr_data_o;
    
    // dr_data_i = {9'b0, tdi_data};
    // dr_data_o = 41'b0;
    dr_data_i.data_low = tdi_data & 0x03;
    dr_data_i.data_mid = (tdi_data>>2); 
    dr_data_i.data_high = 0 ;

    dr_data_o.data_low = 0;
    dr_data_o.data_mid = 0;
    dr_data_o.data_high = 0;

    // write ir 0x11, mean access dmi register
    jtag_idle_to_shift_ir();
    send_ir(reg_num);
    // back to idle
    jtag_exit_to_idle();
    // write dmi register, launch one dm register access
    jtag_idle_to_shift_dr();

    // send_dr(dr_data_i, dr_data_o, tdi_size);

    send_dr(p, q, tdi_size);

    // enter idle
    jtag_exit_to_idle();
    jtag_keep_in_idle(10);

    // tdo_data = dr_data_o[40:9];
    tmp_data.data_low = (dr_data_o.data_mid >> (9-2)) & 0x03;
    tmp_data.data_mid = (dr_data_o.data_mid >> 9) + (dr_data_o.data_high << (32-9));
    tmp_data.data_high = 0;
    tdo_data = &tmp_data;
}

int get_jtag_wire_mode(void)
{
    if (cjtag_mode)
        return 2;
    else
        return 4;
}

int enable_dmi(void) 
{
    int error = 1;
    uint32_t value = 0;
    read_dm_register(DBG_DMCONTROL, &value, &error);  
	LOG_PRINT("enable_dmi read DBG_DMCONTROL error: %d\r\n",error);
	LOG_PRINT("enable_dmi read DBG_DMCONTROL value: 0x%x\r\n",value);

    value |= (1 << CTRL_DMACTIVE);
    write_dm_register(DBG_DMCONTROL, value, &error);

	LOG_PRINT("enable_dmi write DBG_DMCONTROL error: %d\r\n",error);
	LOG_PRINT("enable_dmi write DBG_DMCONTROL value: 0x%x\r\n",value);

    read_dm_register(DBG_DMCONTROL, &value, &error);  
	LOG_PRINT("enable_dmi read DBG_DMCONTROL error: %d\r\n",error);
	LOG_PRINT("enable_dmi read DBG_DMCONTROL value: 0x%x\r\n",value);

    if (error == 1 || value != 1)   
    {

    } 
    return error;
}

int halt_cpu(void) 
{
    int error;
    uint32_t value;
    read_dm_register(DBG_DMCONTROL, &value, &error); 
    LOG_PRINT("halt_cpu read error: %d\r\n",error);
    LOG_PRINT("halt_cpu read value: 0x%x\n",value);
    if (error)   
    {
        return error;   
    }      
    // set the haltreq bit
    value |= (1<<CTRL_HALTREQ);
	// clear the resumereq bit
    value &= ~(1<<CTRL_RESUMEREQ);
    write_dm_register(DBG_DMCONTROL, value, &error);
	LOG_PRINT("halt_cpu write error: %d\r\n",error);
	LOG_PRINT("halt_cpu write value: 0x%x\r\n",value);
    return error;
}

int resume_cpu(void)
{
    int error;
    uint32_t value;
    read_dm_register(DBG_DMCONTROL, &value, &error);
	LOG_PRINT("resume_cpu read value: 0x%x\r\n",value);    
    if (error)
    {
        return error;
    }
	value =1;
    // clear the haltreq bit
	value &= ~(1<<CTRL_HALTREQ);
    // set the resumereq bit
    value |= (1<<CTRL_RESUMEREQ);
    write_dm_register(DBG_DMCONTROL, value, &error);
    return error;
}

int clear_cmderr(void)
{
	int error;
	write_dm_register(DBG_ABSTRACTCS, (7 << ABSCS_CMDERR), &error);
	return error;
}

int read_register_using_abs_cmd(int reg_value, uint32_t *value, REG_ACCESS_SIZE size)
{
	int error;
	uint32_t r_value;

	// write commad
	uint32_t command = (size << CMD_AARSIZE) | (1 << CMD_TRANSFER) | reg_value;
	write_dm_register(DBG_COMMAND, command, &error);
	if (error)
		return error;

	// read register[31:0]
	read_dm_register(DBG_DATA0, &r_value, &error);
	if (error)
		return error;

	*value = (uint64_t)r_value;
	if (size == REG_SIZE_64) {
		// read register[63:32]
		read_dm_register(DBG_DATA1, &r_value, &error);
		*value = ((uint64_t)r_value << 32) | *value;
		if (error)
			return error;
	}
	return OK;
}
int write_register_using_abs_cmd(int reg_value, uint32_t value, REG_ACCESS_SIZE size)
{
	int error;
	uint32_t w_value;

	// write data[31:0]
	w_value = (uint32_t)(value & 0xffffffff);
	write_dm_register(DBG_DATA0, w_value, &error);
	if (error)
		return error;

	// if (size == REG_SIZE_64) {
	// 	// write data[63:32]
	// 	w_value = (uint32_t)((value>>32) & 0xffffffff);
	// 	write_dm_register(DBG_DATA1, w_value, &error);
	// 	if (error)
	// 		return error;
	// }

	// write commad
	uint32_t command = (size << CMD_AARSIZE) | (1 << CMD_TRANSFER) | (1<<CMD_WRITE) | reg_value;
	write_dm_register(DBG_COMMAND, command, &error);

	return error;
}
int read_memory_using_prg_buf(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size) {
	int error;
	uint32_t addr32;
	uint32_t opcode;

	switch(size) {
		case MEM_SIZE_8:
			opcode = 0x00040403;  // lb	s0,0(s0)
			break;
		case MEM_SIZE_16:
			opcode = 0x00041403;  // lh	s0,0(s0)
			break;
		case MEM_SIZE_32:
			opcode = 0x00042403;  // lw	s0,0(s0)
			break;
		case MEM_SIZE_64:
			opcode = 0x00043403;  // ld	s0,0(s0)
			break;
		default:
			opcode = 0x00100073;  // ebreak
	}

	// write progbuf0
	write_dm_register(DBG_PROGBUF0, opcode, &error);
	if (error)
		return error;
	// write prog buf1 ebreak
	write_dm_register(DBG_PROGBUF1, 0x00100073, &error);   // ebreak
	if (error)
		return error;
	//
	// write data0 address
	addr32 = (uint32_t)(addr & 0xffffffff);
	write_dm_register(DBG_DATA0, addr32, &error);
	if (error)
		return error;

	uint32_t command;
	// for rv64, the address is 64bit
	// if (riscv_xlen > 32) {
	// 	addr32 = (uint32_t)((addr >> 32) & 0xffffffff);
	// 	write_dm_register(DBG_DATA1, addr32, &error);
	// 	if (error)
	// 		return error;
	// 	command = (3u << CMD_AAMSIZE) | (1u << CMD_POSTEXEC) | (1u << CMD_TRANSFER) | (1u << CMD_WRITE) | 0x1008;
	// }
	// else
		command = (2u << CMD_AAMSIZE) | (1u << CMD_POSTEXEC) | (1u << CMD_TRANSFER) | (1u << CMD_WRITE) | 0x1008;

	// write command,  write, postexec, regno = 0x1008
	write_dm_register(DBG_COMMAND, command, &error);
	if (error)
		return error;

	// write command to read x8, regno = 0x1008
	if (size > MEM_SIZE_32)
		command = (REG_SIZE_64 << CMD_AARSIZE) | (1u << CMD_TRANSFER) | 0x1008;
	else
		command = (REG_SIZE_32 << CMD_AARSIZE) | (1u << CMD_TRANSFER) | 0x1008;
	write_dm_register(DBG_COMMAND, command, &error);
	if (error)
		return error;

	// read dbg_data
	uint32_t r_value;
	read_dm_register(DBG_DATA0, &r_value, &error);
	if (error)
		return error;
	*data = (uint64_t) r_value;
	// read more than 32 bit memory, using DBG_DATA1
	if (size > MEM_SIZE_32) {
		read_dm_register(DBG_DATA1, &r_value, &error);
		*data = ((uint64_t)r_value << 32u) | *data;
	}
	return error;
}
int read_memory_using_abs_cmd(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size) {
	int error;
	uint32_t addr32;
	// if (riscv_xlen > 32) {
	// 	// for rv64
	// 	addr32 = (uint32_t)addr & 0xffffffff;
	// 	write_dm_register(DBG_DATA2, addr32, &error);
	// 	if (error)
	// 		return error;

	// 	addr32 = (uint32_t)(addr >> 32u) & 0xffffffff;
	// 	write_dm_register(DBG_DATA3, addr32, &error);
	// 	if (error)
	// 		return error;
	// }
	// else {
		addr32 = (uint32_t)addr & 0xffffffff;
		write_dm_register(DBG_DATA1, addr32, &error);
		if (error)
			return error;
	// }

	// write command, to access memory
	write_dm_register(DBG_COMMAND, (2u << CMD_CMDTYPE) | (size << CMD_AARSIZE) | (1u<<CMD_TRANSFER), &error);
	if (error)
		return error;

	// read dbg_data
	uint32_t r_value;
	read_dm_register(DBG_DATA0, &r_value, &error);
	if (error)
		return error;
	*data = (uint64_t) r_value;
	// read more than 32 bit memory, using DBG_DATA1
	if (size > MEM_SIZE_32) {
		read_dm_register(DBG_DATA1, &r_value, &error);
		*data = ((uint64_t)r_value << 32u) | *data;
	}
	return error;
}
int read_memory_using_sys_bus(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size) {
	int error;
	uint32_t r_data;
	*data = 0;

	read_dm_register(DBG_SBCS, &r_data, &error);
	//printf("-Info : read_memory DBG_SBCS %0x\n", error);
	r_data |= (size << SBCS_SBACCESS) | (1u << SBCS_SBREADONADDR);

	// write SBCS , select the access size
	write_dm_register(DBG_SBCS, r_data, &error);
	//printf("-Info : write_memory DBG_SBCS error = %0x\n", error);

	// write address
	write_dm_register(DBG_SBADDRESS0, (addr & 0xffffffff), &error);
    // write_dm_register(DBG_SBDATA0, (addr & 0xffffffff), &error);
	//printf("-Info : write_memory DBG_SBADDRESS0 error = %0x\n", error);

	// wait the read finish
	int i;
	for(i=0; i<5; i++) {
		read_dm_register(DBG_SBCS, &r_data, &error);
		if (error)
        {
            return error;
        }
			
        //printf("-Info : read_memory DBG_SBCS error = %0x, i= %0x\n", error,i);
		// judge the sbbusy bit, if zero, mean access is finish
		if ( !((r_data >> SBCS_SBBUSY) & 0x1))
        {
            break;
        }	
	}
	if (i==5) {
		return TIMEOUT;
	}

	// read data
	// if (size > MEM_SIZE_32) {
	// 	read_dm_register(DBG_SBDATA1, &r_data, &error);
	//     //printf("-Info : read_memory DBG_SBDATA1 error = %0x\n", error);
	// 	if (error)
	// 		return error;
	// 	*data = (uint64_t)r_data << 32;
	// }

	read_dm_register(DBG_SBDATA0, &r_data, &error);
	//printf("-Info : read_memory DBG_SBDATA0 error = %0x\n", error);

	*data |= (uint32_t)r_data;

	// write sbcs 0, clear sbreadonaddr , sbaccess, sbautoincrement, abreadondata
	write_dm_register(DBG_SBCS, 0, &error);
	return error;
}
int write_memory_using_prg_buf(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size) {
	int error;
	int aarsize;
	uint32_t opcode;

	switch(size) {
		case MEM_SIZE_8:
			opcode = 0x00940023;    // sb	s1,0(s0)
			break;
		case MEM_SIZE_16:
			opcode = 0x00941023;    // sh	s1,0(s0)
			break;
		case MEM_SIZE_32:
			opcode = 0x00942023;    // sw	s1,0(s0)
			break;
		case MEM_SIZE_64:
			opcode = 0x00943023;    // sd	s1,0(s0)
			break;
		default:
			opcode = 0x00100073;    // ebreak
	}


	// write prog buffer 0  sw s1, 0(s0)
	write_dm_register(DBG_PROGBUF0, opcode, &error);   // sb/sh/sw/sd s1, 0(s0)
	if (error)
		return error;
	// write progbug1  ebreak
	write_dm_register(DBG_PROGBUF1, 0x00100073, &error);   // ebreak
	if (error)
		return error;
	//
	// write data0 address
	uint32_t addr32 = (uint32_t)(addr & 0xffffffff);
	write_dm_register(DBG_DATA0, addr32, &error);
	if (error)
		return error;

	uint32_t command;
	// if (riscv_xlen > 32) {
	// 	// for rv64 , the address is 64 bit
	// 	addr32 = (uint32_t)((addr >> 32) & 0xffffffff);
	// 	write_dm_register(DBG_DATA1, addr32, &error);
	// 	if (error)
	// 		return error;
	// 	command = (3u << CMD_AARSIZE) | (1u << CMD_WRITE) | (1u << CMD_TRANSFER) | 0x1008;
	// 	aarsize = 3;
	// }
	// else {
		command = (2u << CMD_AARSIZE) | (1u << CMD_WRITE) | (1u << CMD_TRANSFER) | 0x1008;
		aarsize = 2;
	// }

	// write command,  write, regno = 0x1008
	write_dm_register(DBG_COMMAND, command, &error);
	if (error)
		return error;

	// write data, value
	uint32_t data32;
	data32 = (uint32_t)(data & 0xffffffff);
	write_dm_register(DBG_DATA0, data32, &error);
	if (error)
		return error;

	// if (size > MEM_SIZE_32) {
	// 	// when write data exceed 32 bit, using dbg_data1 to parse high 32-bit value
	// 	data32 = (uint32_t)((data >> 32u) & 0xffffffff);
	// 	write_dm_register(DBG_DATA1, data32, &error);
	// 	if (error)
	// 		return error;
	// }

	// write command, write, postexec, regno = 0x1009
	command = (aarsize << CMD_AARSIZE) | (1u << CMD_POSTEXEC) | (1u << CMD_TRANSFER) | (1u << CMD_WRITE) | 0x1009;
	write_dm_register(DBG_COMMAND, command, &error);
	if (error)
		return error;
    return 0;
}
int write_memory_using_abs_cmd(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size) {
	int error ;
	uint32_t addr32;
	uint32_t data32;
	addr32 = (uint32_t)(addr & 0xffffffff);
	data32 = (uint32_t)(data & 0xffffffff);

	// if (riscv_xlen > 32) {
	// 	// for rv64
	// 	// write address
	// 	write_dm_register(DBG_DATA2, addr32, &error);
	// 	if (error)
	// 		return error;
	// 	addr32 = (uint32_t)((addr >> 32u) & 0xffffffff);
	// 	write_dm_register(DBG_DATA3, addr32, &error);
	// 	if (error)
	// 		return error;

	// 	// write data
	// 	write_dm_register(DBG_DATA0, data32, &error);
	// 	if (error)
	// 		return error;
	// 	data32 = (uint32_t)((data >> 32u) & 0xffffffff);
	// 	write_dm_register(DBG_DATA1, data32, &error);
	// 	if (error)
	// 		return error;
	// }
	// else {
		// for rv32
		// write data1 address
		write_dm_register(DBG_DATA1, addr32, &error);
		if (error)
			return error;
		// write data0 value
		write_dm_register(DBG_DATA0, data32, &error);
		if (error)
			return error;
	// }
	// write command, to access memory
	uint32_t command = (2u << CMD_CMDTYPE) | (size << CMD_AARSIZE) | (1u << CMD_TRANSFER) | (1u << CMD_WRITE);
                             //   24u                  20u                   17u                    16u
	write_dm_register(DBG_COMMAND, command, &error);
                          //0x17
	if (error)
		return error;
    return 0;
}
int write_memory_using_sys_bus(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size) {
	int error;
	uint32_t r_data;

	read_dm_register(DBG_SBCS, &r_data, &error);
	//printf("write_memory_using_sys_bus read DBG_SBCS r_data: 0x%x\n",r_data);  //1104
	
	if (error)
		return error;

	r_data &= ~((7u << SBCS_SBACCESS) | (1u << SBCS_SBREADONADDR));
	//r_data &= ~((7u << SBCS_SBACCESS));
	r_data |= (size << SBCS_SBACCESS);

	// write SBCS , select the access size
	write_dm_register(DBG_SBCS, r_data, &error);
	if (error)
		return error;

	// write address
	write_dm_register(DBG_SBADDRESS0, (addr & 0xffffffff), &error);
	if (error)
		return error;

	// if (riscv_xlen > 32) {
	// 	write_dm_register(DBG_SBADDRESS1, ((addr >> 32u) & 0xffffffff), &error);
	// 	if (error)
	// 		return error;
	// }

	// write data
	// if (size > MEM_SIZE_32) {
	// 	// for rv64, write the upper 32bit value
	// 	write_dm_register(DBG_SBDATA1, ((data >> 32u) & 0xffffffff), &error);
	// 	if (error)
	// 		return error;
	// }

	write_dm_register(DBG_SBDATA0, (data & 0xffffffff), &error);
	if (error)
		return error;

	// wait the write finish
	for(int i=0; i<5; i++) {
		read_dm_register(DBG_SBCS, &r_data, &error);
		//printf("write_sys_bus wait the write finish read DBG_SBCS r_data: 0x%x\n",r_data);
		if (error)
			return error;

		// judge the sbbusy bit, if zero, mean access is finish
		if ( !((r_data >> SBCS_SBBUSY) & 0x1)) {
			// write sbcs 0, clear sbreadonaddr , sbaccess, sbautoincrement, abreadondata
			write_dm_register(DBG_SBCS, 0, &error);
			return error;
		}
	}
	return 1;
}

int read_csr(int reg_num, uint32_t * value, AccessModeList mode) {
    return read_register_using_abs_cmd(reg_num, value, REG_SIZE_32);
}

int write_csr(int reg_num, uint32_t value, AccessModeList mode){
    return write_register_using_abs_cmd(reg_num, value, REG_SIZE_32);
}

int read_memory(uint32_t addr , uint32_t *data, MEM_ACCESS_SIZE size, AccessModeList mode) {
	if (mode == PRG_BUF)
		return read_memory_using_prg_buf(addr, data, size);
	else if (mode == ABS_CMD)  //abstract   ABS_CMD = 0
		return read_memory_using_abs_cmd(addr, data, size);
	else                       //sba        SYS_BUS = 2
		return read_memory_using_sys_bus(addr, data, size);
}

int write_memory(uint32_t addr , uint32_t data, MEM_ACCESS_SIZE size, AccessModeList mode) {
	if (mode == PRG_BUF)
		return write_memory_using_prg_buf(addr, data, size);
	else if (mode == ABS_CMD)
		return write_memory_using_abs_cmd(addr, data, size);
	else
		return write_memory_using_sys_bus(addr, data, size);
}

int write_memory_bulk_ABS_CMD(uint32_t base_addr, uint32_t *data_array, int len, MEM_ACCESS_SIZE size) {    //
	int error;
	// uint32_t r_data;
    int i;
	write_dm_register(DBG_DATA1, (base_addr & 0xffffffff), &error);
	if (error)
		return error;
    if(size > MEM_SIZE_32) {
	    write_dm_register(DBG_DATA1, (data_array[0] & 0xffffffff), &error); 
    if (error)
		return error;  

	}
   write_dm_register(DBG_DATA0, (data_array[0] & 0xffffffff), &error); //need modify
   if (error)
        return error;
   write_dm_register(DBG_COMMAND, ((2 << CMD_CMDTYPE) | (2 << CMD_AAMSIZE) | (1 << CMD_WRITE) | (1 << DMS_ALLHAVERESET)), &error); 	 
   if (error)
   return error;

   write_dm_register(DBG_ABSTRACTAUTO, 1, &error);
	if (error)
        return error;
	for(i = 1; i < len; i++) {
		// if (size > MEM_SIZE_32) {
		// 	write_dm_register(DBG_DATA1, ((data_array[i] >> 32u) & 0xffffffff), &error); //need modify
		// 	if (error)
        //         return error;
		// }
		write_dm_register(DBG_DATA0, (data_array[i] & 0xffffffff), &error);   
            	if (error)
                return error;
	}

	write_dm_register(DBG_ABSTRACTAUTO, 0, &error);
    if (error)
        return error;
    return 0;
}

void send_byte(uint8_t data)
{
    for(int i=0;i<8;i++)
    {
        send_tms((data>>(7-i))&0x01);
    }
}

void StartJtagTask(JtagStateList code)
{
    // 发bit 0 send_tms(0);
    // 发bit 1 send_tms(1);
    // 发送数据 128'h = 4DAD_1234_0FBF_A6C5_9E18_7170_FEDC_6963, 由于前面有11个clk，所以数据重新组合的一下
    tck(0);
    tms(0);
    tdi(0);
    rst(0);
    rst(0);
    delay_ms(1);
    
    send_byte(0x00);
    send_byte(0x18);
    send_byte(0xD2);
    send_byte(0xC7);
    send_byte(0x6F);
    send_byte(0xE1);
    send_byte(0xD1);
    send_byte(0xC3);
    send_byte(0x0F);
    send_byte(0x34);
    send_byte(0x6C);
    send_byte(0xBF);
    send_byte(0xBE);
    send_byte(0x05);
    send_byte(0x89);
    send_byte(0x16);
    if(code % 2 == 0)
    {
        send_byte(0xB6);
    }
    else
    {
        send_byte(0xB7);
    }

    // 0x4D>>5 = 010 ,实际发出来应该是010
    for(int i=0;i<3;i++)
    {
        send_tms(((code>>1)>>(i))&0x01);
    }

    delay_cycle();

    // 直接先拉高 w_shutdown_resetn
    rst(1);
    tdi(0);

    // flash 到 CPU 启动需要毫秒级延时
    delay_ms(2);
}

void dbg_test_code(void)
{
    LOG_PRINT("enter dbg test code\r\n");    
    uint32_t value;
	read_memory(TST_CTL_TST_KEY_OBSERVE_3780, &value, MEM_SIZE_32, SYS_BUS);
	if(value != 0x21) {
        LOG_PRINT("test_key_dft_en mismatch 0x%0x\r\n", value);
	}
    else
    {
        LOG_PRINT("read memory is 0x%x\r\n", value);
    }
}

void dbg_test_init(void)
{    
	int error;
    uint32_t value;
	LOG_PRINT("enter dbg_test_init\r\n");
    Jtag_Function_Init();
    
    if(mcu_jtag_driver.jtag_mode == 4)
    {
        jtag_reset();

        StartJtagTask(DftEnable_four);
        LOG_PRINT("print key mode\r\n");

        jtag_reset();
    }
    else
    {
        StartJtagTask(DftEnable_two);
        LOG_PRINT("print key mode\r\n");

        switch_jtag2cjtag();
        LOG_PRINT("switch to jtag mode\r\n");
    }

	enable_dmi();
	LOG_PRINT("dmi enable finish\r\n");

    read_dm_register(DBG_COMMAND, &value, &error);
	// read the s0 64-bit -> 32-bit
	write_dm_register(DBG_COMMAND, (3u << CMD_AARSIZE) | (1u << CMD_TRANSFER) | (0x1008), &error);
    read_dm_register(DBG_COMMAND, &value, &error);
	if (value != 0x321008) 
    {
		LOG_PRINT("initial can't access DBG_COMMAND\r\n");
        LOG_PRINT("value = %x\r\n", value);
	}
    LOG_PRINT("switch to 32-bit mode\r\n");

	clear_cmderr();
    LOG_PRINT("clear cmd err\r\n");

    dbg_test_code();
}


void flash_init(void)
{
    write_memory(FLASH_INTS_3775, 0xFFFFFFFF, MEM_SIZE_32, SYS_BUS);
}

int flash_erase(uint32_t start_addr, uint32_t end_addr)
{
	if(start_addr & 0xfff)
	{
		return FLASH_ERR_3775 | start_addr;
	}
	uint32_t rdata,wip;
    uint32_t sector_num = (end_addr - start_addr + FLASH_SECTOR_SIZE_3775 - 1) / FLASH_SECTOR_SIZE_3775;
    if(sector_num > (FLASH_MEM_SIZE_3775 + 1*1024*1024) / FLASH_SECTOR_SIZE_3775)
	{
		return FLASH_ERR_3775 | sector_num;
	}
	write_memory(FLASH_INTS_3775, 0xFFFFFFFF, MEM_SIZE_32, SYS_BUS);
    
    /* erase flash */
    write_memory(FLASH_CTRL_3775,0x00000000, MEM_SIZE_32, SYS_BUS);//disable cache
    
    for (int i = 0;i < sector_num;i++) 
	{
		write_memory(FLASH_COMD_3775,(0x80000000 | FLASH_CMD_WREN_3775), MEM_SIZE_32, SYS_BUS);//FLASH WREN
		write_memory(FLASH_ADDR_3775, start_addr + i*FLASH_SECTOR_SIZE_3775, MEM_SIZE_32, SYS_BUS);//整片擦除时无效
		write_memory(FLASH_COMD_3775,(0x80000000 | 0x20), MEM_SIZE_32, SYS_BUS);//60擦整片，20擦SPIFLASH_SECTOR_SIZE_3775
        read_memory(FLASH_COMD_3775, &rdata, MEM_SIZE_32, SYS_BUS);
		while(rdata & 0x80000000)
		{
            read_memory(FLASH_COMD_3775, &rdata, MEM_SIZE_32, SYS_BUS);
		}
		wip=0x1;
		write_memory(FLASH_INTS_3775,0xFFFFFFFF, MEM_SIZE_32, SYS_BUS);
		while(wip & 0x00000001)
		{
			write_memory(FLASH_COMD_3775,(0x80000000 | FLASH_CMD_RDSR_3775), MEM_SIZE_32, SYS_BUS);

            uint32_t temp;
            read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            while((temp & 0x4) == 0x0)
            {
                read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            }
			
            read_memory(FLASH_RDATA_3775, &wip, MEM_SIZE_32, SYS_BUS);
			write_memory(FLASH_INTS_3775,0xFFFFFFFF, MEM_SIZE_32, SYS_BUS);
		}
	}
    
    write_memory(FLASH_CTRL_3775,0x00000001, MEM_SIZE_32, SYS_BUS);//enable cache
    return 0;
}

int flash_write(uint8_t* buffer, uint32_t offset, uint32_t count)
{
	int write_count = 0;
	uint32_t i, rdata, wip;
    uint32_t cur_offset = offset % FLASH_PAGE_SIZE_3775;
    uint32_t cur_count = 0;
    /* write flash */
    write_memory(FLASH_CTRL_3775,0x00000000, MEM_SIZE_32, SYS_BUS);//disable cache
    
    while ((int)count > 0)
	{
        if ((cur_offset + count) >= FLASH_PAGE_SIZE_3775)
		{
            cur_count = FLASH_PAGE_SIZE_3775 - cur_offset;
        }
		else
		{
            cur_count = count;
			if(cur_count > FLASH_PAGE_SIZE_3775)
				cur_count = FLASH_PAGE_SIZE_3775;
        }
        cur_offset = 0;
		//--------------------------------------
		for(i=0;i<4 && i<cur_count/4;i++)
		{//write 4*4byte into wfifo
			write_memory(FLASH_WDATA_3775,((uint32_t*)buffer)[i], MEM_SIZE_32, SYS_BUS);
		}
		write_memory(FLASH_ADDR_3775,offset, MEM_SIZE_32, SYS_BUS);//program address      
		write_memory(FLASH_DATN_3775,cur_count-1, MEM_SIZE_32, SYS_BUS);//program length 256Byte
		write_memory(FLASH_COMD_3775,(0x80000000 | FLASH_CMD_WREN_3775), MEM_SIZE_32, SYS_BUS); //FLASH WREN , erase will reset WEL
		write_memory(FLASH_COMD_3775,(0x80000000 | FLASH_CMD_PP_3775), MEM_SIZE_32, SYS_BUS);   //FLASH PRO
		for(;i<cur_count/4;i++)
		{//check wfifo not almast full, write 1*4byte into wfifo
            uint32_t temp;
            read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            while((temp & 0x20) == 0x0)
            {
                read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            }

			write_memory(FLASH_WDATA_3775,((uint32_t*)buffer)[i], MEM_SIZE_32, SYS_BUS);
		}
        read_memory(FLASH_COMD_3775, &rdata, MEM_SIZE_32, SYS_BUS);
		while(rdata & 0x80000000)
		{
            read_memory(FLASH_COMD_3775, &rdata, MEM_SIZE_32, SYS_BUS);
		}
		wip=0x1;
		write_memory(FLASH_INTS_3775,0xFFFFFFFF, MEM_SIZE_32, SYS_BUS);
		while(wip & 0x00000001)
		{ //wip=0 means program finish
			write_memory(FLASH_COMD_3775,(0x80000000 | FLASH_CMD_RDSR_3775), MEM_SIZE_32, SYS_BUS);

            uint32_t temp;
            read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            while((temp & 0x4) == 0x0)
            {
                read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            }

            read_memory(FLASH_RDATA_3775, &wip, MEM_SIZE_32, SYS_BUS);
			write_memory(FLASH_INTS_3775,0xFFFFFFFF, MEM_SIZE_32, SYS_BUS);
		}
		//--------------------------------------
        buffer += cur_count;
        offset += cur_count;
        count -= cur_count;
        write_count += cur_count;
    }
    write_memory(FLASH_CTRL_3775,0x00000001, MEM_SIZE_32, SYS_BUS);//enable cache
    return 0;
}

int flash_read(uint8_t* buffer, uint32_t offset, uint32_t count)
{
	if((((uint32_t)buffer) & 3) | (offset & 3))
	{
		return FLASH_ERR_3775 | (uint32_t)buffer;
	}
	uint32_t i;
	uint32_t *buf_int;
	uint32_t cur_count;
	uint32_t cur_len;
    write_memory(FLASH_CTRL_3775, 0x00000000, MEM_SIZE_32, SYS_BUS);//disable cache
    
	while(count > 0)
	{
		if(count > FLASH_PAGE_SIZE_3775)//256
			cur_count = FLASH_PAGE_SIZE_3775;
		else
			cur_count = count;
		buf_int = (uint32_t *) buffer;
		cur_len = (cur_count + 3) / 4;

	    write_memory(FLASH_ADDR_3775, offset, MEM_SIZE_32, SYS_BUS);//read address
		write_memory(FLASH_DATN_3775, cur_len * 4 - 1, MEM_SIZE_32, SYS_BUS);//read length 256byte
		write_memory(FLASH_COMD_3775,(0x80000000 | FLASH_CMD_RD_3775), MEM_SIZE_32, SYS_BUS);//FLASH RD
		//write_memory(FLASH_COMD,(0x80000000 | 0x3b), MEM_SIZE_32, SYS_BUS);//dual output
		for( i=0;i<cur_len;i++)
		{
            uint32_t temp;
            read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            while((temp & 0x4) == 0x0)
            {
                read_memory(FLASH_INTS_3775, &temp, MEM_SIZE_32, SYS_BUS);
            }
            read_memory(FLASH_RDATA_3775, &buf_int[i], MEM_SIZE_32, SYS_BUS);
		}

		count -= cur_count;
		offset += cur_count;
		buffer += cur_count;
	}
	
    write_memory(FLASH_CTRL_3775,0x00000001, MEM_SIZE_32, SYS_BUS);//enable cache
    return 0;
}
