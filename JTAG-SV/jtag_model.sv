/**
 * @file: jtag_dpi.sv
 * @description:
 * @author:  lujun
 * @version: v2.0
		v1.0 support jtag debug 
		v2.0 support 2-wire debug
 * @create_time: Nov 2, 2019 8:56:33 AM
 * @update_time:
 */
`timescale 1ns/1ps

typedef   longint    unsigned  uint64_t;
typedef   int        unsigned  uint32_t;
typedef   shortint   unsigned  uint16_t;
typedef   byte       unsigned  uint8_t;

typedef   longint    signed  int64_t;
typedef   int        signed  int32_t;
typedef   shortint   signed  int16_t;
typedef   byte       signed  int8_t;

module jtag_model
	#(
		parameter DELAY=50
	)
	(
		input  clk,
		input  rst_n,

		output jtag_tck,
		output jtag_tms,
        input  jtag_tms_in,
        input  jtag_tms_in_en,
		output jtag_tdi,
		input  jtag_tdo,

        output    reg VPP0 ,
        output    reg VPP1 ,
        output    reg TM0  

	);

	reg tck;
	reg tms;
	wire tms_in;
	reg tdi;
	reg cjtag_mode;
	assign tms_in = jtag_tms_in_en ? jtag_tms_in : 1'b1;

    import "DPI-C" context task dbg_test_init();


	export "DPI-C" task write_dm_register;
	export "DPI-C" task read_dm_register;
	export "DPI-C" task debug_error;
	export "DPI-C" task debug_finish;
	export "DPI-C" task delay_us;

	initial begin
		tck = 1'b0;
		tms = 1'b0;
		tdi = 1'b0;
		cjtag_mode = 1'b0;
		// wait the reset is release
		@(posedge rst_n);
		#1300us;
		jtag_reset();		
        $display("jtag_model use 2wire");
        switch_jtag2cjtag();
		
		dbg_test_init();
	end
	
    task delay_us();
         #16us;
    endtask: delay_us

	// reset and enter idle state
	task jtag_reset();
		repeat(5)
			send_tms_tdi(1);
		    send_tms_tdi(0);  // enter idle
	endtask: jtag_reset

    task send_tms(input tms_i);
        tms = tms_i;
        #DELAY tck = 1'b1;
        #DELAY tck = 1'b0;
    endtask: send_tms
	
	task send_tms_tdi(input tms_i, input tdi_i=1'b0);
		if (cjtag_mode) begin
            tms = ~tdi_i;
            #DELAY tck = 1'b1;
            #DELAY tck = 1'b0;
            tms = tms_i;
            #DELAY tck = 1'b1;
            #DELAY tck = 1'b0;
            tms = 1'b0; 
            #DELAY tck = 1'b1;
            #DELAY tck = 1'b0;
        end
        else begin
            tms = tms_i;
            tdi = tdi_i;
            #DELAY tck = 1'b1;
            #DELAY tck = 1'b0;
        end
	endtask
	
	task jtag_tlr_to_idle();
        send_tms_tdi(1'b0);
    endtask

    task jtag_idle_to_sld();
        send_tms_tdi(1'b1);
    endtask

    task jtag_sld_to_cdr();
        send_tms_tdi(1'b0);
    endtask

    task jtag_cdr_to_sdr();
        send_tms_tdi(1'b0);
    endtask

    task jtag_sdr_to_sdr();
        send_tms_tdi(1'b0);
    endtask

    task jtag_sdr_to_e1d();
        send_tms_tdi(1'b1);
    endtask

    task jtag_e1d_to_pdr();
        send_tms_tdi(1'b0);
    endtask

    task jtag_pdr_to_e2d();
        send_tms_tdi(1'b1);
    endtask

    task jtag_e2d_to_sdr();
        send_tms_tdi(1'b0);
    endtask

    task jtag_e2d_to_udr();
        send_tms_tdi(1'b1);
    endtask

    task jtag_cdr_to_e1d();
        send_tms_tdi(1'b1);
    endtask

    task jtag_e1d_to_udr();
        send_tms_tdi(1'b1);
    endtask

    task jtag_udr_to_sld();
        send_tms_tdi(1'b1);
    endtask

    task jtag_udr_to_idle();
        send_tms_tdi(1'b0);
    endtask

    task jtag_sld_to_sli();
        send_tms_tdi(1'b1);
    endtask

    task jtag_sli_to_cir();
        send_tms_tdi(1'b0);
    endtask
    
    task jtag_cir_to_sir();
        send_tms_tdi(1'b0);
    endtask

    task jtag_sir_to_sir();
        send_tms_tdi(1'b0);
    endtask
    
    task jtag_sir_to_e1i();
        send_tms_tdi(1'b1);
    endtask

    task jtag_e1i_to_uir();
        send_tms_tdi(1'b1);
    endtask

    task jtag_uir_to_idle();
        send_tms_tdi(1'b0);
    endtask

    task jtag_uir_to_sld();
        send_tms_tdi(1'b1);
    endtask

    task jtag_e1i_to_pir();
        send_tms_tdi(1'b0);
    endtask

    task jtag_pir_to_e2i();
        send_tms_tdi(1'b1);
    endtask

    task jtag_eir_to_sir();
        send_tms_tdi(1'b0);
    endtask
	
	
	task jtag_exit_to_idle();
		//send_tms_tdi(0);  // enter pause-dr
		//send_tms_tdi(1);  // enter exit2-dr
		send_tms_tdi(1);  // enter update-dr
		send_tms_tdi(0);  // enter ilde
	endtask: jtag_exit_to_idle
	
	// from idle state to shift ir
	task jtag_idle_to_shift_ir();
		send_tms_tdi(1);  // enter select-DR scan
		send_tms_tdi(1);  // enter select-IR scan
		send_tms_tdi(0);  // enter capture-ir 
		send_tms_tdi(0);  // enter shift-ir
	endtask: jtag_idle_to_shift_ir

	// from idle state to shift-dr 
	task jtag_idle_to_shift_dr();
		send_tms_tdi(1);  // enter select-DR scan
		send_tms_tdi(0);  // enter capture-dr
		send_tms_tdi(0);  // enter shift-ir
	endtask: jtag_idle_to_shift_dr
	
	task send_ir(bit[4:0] ir_reg);
		// current in shift-ir state
		for(int i=0; i<4; i++) begin
			send_tms_tdi(0, ir_reg[i]);
		end
		// the last, tms send 1
		send_tms_tdi(1, ir_reg[4]);
	endtask
	
	bit[40:0] temp_tdo_reg;	
	task send_dr(bit[40:0] tdi_reg, output bit[40:0] tdo_reg, input int size);
		int i;
        temp_tdo_reg = 41'b0;
		// current in shift-dr state
        if (cjtag_mode) begin
            for(i=0; i<size-1; i++) begin
                tms = ~tdi_reg[i];
                #DELAY tck = 1'b1;
                #DELAY tck = 1'b0;
                tms = 1'b0;
                #DELAY tck = 1'b1;
                #DELAY tck = 1'b0;
                tms = 1'b0; 
                #DELAY;
                temp_tdo_reg = {tms_in, temp_tdo_reg[40:1]};
                tck = 1'b1;
                #DELAY tck = 1'b0;
            end
            // last send tms 1 
            tms = ~tdi_reg[size-1];
            #DELAY tck = 1'b1;
            #DELAY tck = 1'b0;
            tms = 1'b1;
            #DELAY tck = 1'b1;
            #DELAY tck = 1'b0;
            tms = 1'b0; 
            #DELAY;
            temp_tdo_reg = {tms_in, temp_tdo_reg[40:1]};
            tck = 1'b1;
            #DELAY tck = 1'b0;
        end
        else begin
            for(i=0; i<size-1; i++) begin
                send_tms_tdi(0, tdi_reg[i]);
                temp_tdo_reg = {jtag_tdo, temp_tdo_reg[40:1]};
            end
            // last send tms 1 
            send_tms_tdi(1, tdi_reg[size-1]);
            temp_tdo_reg = {jtag_tdo, temp_tdo_reg[40:1]};
        end
		tdo_reg = temp_tdo_reg >> (41 -  size);
	endtask: send_dr
	
	
	
	// keep in idle state some time
	task jtag_keep_in_idle(int number);
		repeat(number)
			send_tms_tdi(0);
	endtask: jtag_keep_in_idle
	
	task jtag_send_ir(uint64_t ir_reg);
		bit[4:0] ir;
		ir = ir_reg[4:0];
		send_ir(ir);
	endtask: jtag_send_ir
	
	task jtag_send_dr(uint64_t ir_reg, output uint64_t tdo_reg, input int size);
		bit[40:0] ir;
		bit[40:0] dr;
		ir = ir_reg[40:0];
		send_dr(ir, dr, size);
		tdo_reg = {23'b0, dr};
	endtask: jtag_send_dr
	
	
	bit[40:0] cjtag_data_o;
    task switch_jtag2cjtag();
        jtag_reset();

        // reset rsu
        tck = 1'b1;
        repeat(8) begin
            #DELAY tms = 1'b1;
            #DELAY tms = 1'b0;
        end

        tck = 1'b0;
        repeat(50)
            send_tms(1'b1);

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
        repeat(2)
            jtag_sdr_to_sdr();
        jtag_sdr_to_e1d();
        jtag_e1d_to_udr();
       
        // cmd2-cp0-3
        jtag_udr_to_sld();
        jtag_sld_to_cdr();
        jtag_cdr_to_sdr();
        repeat(8)
            jtag_sdr_to_sdr();
        jtag_sdr_to_e1d();
        jtag_e1d_to_udr();
        jtag_udr_to_idle();

        // check command
        repeat(4)
            send_tms(1'b0);
        
        cjtag_mode = 1'b1;

        // read rdback0 
        jtag_idle_to_sld();
        jtag_sld_to_cdr();
        jtag_cdr_to_sdr();
        repeat(8)
            jtag_sdr_to_sdr();
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
        send_dr(41'b0, cjtag_data_o, 32);
        $display("rdback0: %h @%d", cjtag_data_o, $time());

        jtag_e1d_to_udr();
        jtag_udr_to_idle();
        // check command
        repeat(4)
            send_tms(1'b0);

        // quit command
        jtag_idle_to_sld();
        jtag_sld_to_sli();
        jtag_sli_to_cir();
        jtag_cir_to_sir();
        jtag_sir_to_e1i();
        jtag_e1i_to_uir();
        jtag_uir_to_idle();

        jtag_keep_in_idle(5);
    endtask: switch_jtag2cjtag

	

	task automatic write_dm_register(int reg_num, int reg_value, output int error);
		// tck is low, tms is low	
		bit [6:0] addr = reg_num[6:0];
		bit [31:0] data = reg_value;
		bit [40:0] tdi_reg = {addr, data, 2'b10};
		bit [40:0] tdo_reg;
		int timeout = 0;

		error = 0;
		// write ir 0x11, mean access dmi register
		jtag_idle_to_shift_ir();
		send_ir(5'h11);
		// back to idle
		jtag_exit_to_idle();
		// write dmi register, launch one dm register access
		jtag_idle_to_shift_dr();
		send_dr(tdi_reg, tdo_reg, 41);
		
		// get the response
		tdi_reg = {7'h0, 32'h0, 2'b0};

		// get the write response
		while(1) begin
			jtag_exit_to_idle();
			jtag_keep_in_idle(10);
			jtag_idle_to_shift_dr();
			send_dr(tdi_reg, tdo_reg, 41);
			case (tdo_reg[1:0])
				2'b00: // write pass
					break;
				2'b01: begin // reserve
					error = 1;
					break;
				end
				2'b10: begin	// write error
					error = 1;
					break;
				end
				2'b11: begin // 2'b11, busy, need next read
					timeout += 1;
					if (timeout > 10) begin
						error = 1;
						break;
					end
				end
			endcase
		end
		// back to idle
		jtag_exit_to_idle();
		jtag_keep_in_idle(10);
	endtask: write_dm_register

	task automatic read_dm_register(int reg_num, output int value, output int error);
		bit [6:0] addr = reg_num[6:0];
		bit [40:0] tdi_reg = {addr, 32'b0, 2'b01};
		bit [40:0] tdo_reg;
		int timeout = 0;

		error = 0;
		// write ir 0x11, mean access dmi register
		jtag_idle_to_shift_ir();
		send_ir(5'h11);
		// back to idle
		jtag_exit_to_idle();
		// write dmi register, launch one dm register access
		jtag_idle_to_shift_dr();
		send_dr(tdi_reg, tdo_reg, 41);
		
		// get the response
		tdi_reg = {7'h0, 32'h0, 2'b0};
		while(1) begin
			// write dmi 0, get the response
			jtag_exit_to_idle();
			jtag_keep_in_idle(10);
			jtag_idle_to_shift_dr();
			send_dr(tdi_reg, tdo_reg, 41);
			
			case (tdo_reg[1:0])
				2'b00: begin //read pass
					// pass
					value = tdo_reg[33:2];
					break;
				end
				2'b01: begin // reserve
					error = 1;
					break;
				end
				2'b10: begin // read error
					error = 2;
					break;
				end
				2'b11: begin // busy
					timeout += 1;
					if (timeout > 10) begin
						error = 3;
						break;
					end
				end
			endcase
		end
		// enter idle
		jtag_exit_to_idle();
		jtag_keep_in_idle(10);
	endtask: read_dm_register
	
	
	task access_dtm_register(int reg_num, int tdi_size, int tdi_data, output int tdo_data);
		bit[40:0] dr_data_i;
		bit[40:0] dr_data_o;
		
		dr_data_i = {9'b0, tdi_data};
		dr_data_o = 41'b0;
		// write ir 0x11, mean access dmi register
		jtag_idle_to_shift_ir();
		send_ir(reg_num);
		// back to idle
		jtag_exit_to_idle();
		// write dmi register, launch one dm register access
		jtag_idle_to_shift_dr();
		send_dr(dr_data_i, dr_data_o, tdi_size);
		// enter idle
		jtag_exit_to_idle();
		jtag_keep_in_idle(10);
		tdo_data = dr_data_o[40:9];
	endtask: access_dtm_register

    task get_simulate_time(output longint simtime);
        simtime = $time();
    endtask: get_simulate_time

    task debug_error();
        $fatal("jtag_model test failed");
    endtask: debug_error

    task debug_finish();
        // $finish();
        if ($test$plusargs("JTAG_LOAD_SRAM")) begin
        $display("JTAG_LOAD_SRAM");
        end
        else if ($test$plusargs("JTAG_LOAD_FLASH")) begin
        $display("JTAG_LOAD_FLASH");
        end
        else if ($test$plusargs("JTAG_LOAD_FLASH")) begin
        $display("JTAG_LOAD_FLASH");
        end
        else if ($test$plusargs("JTAG_SYS_BUS_NO_HALT_CPU")) begin
        $display("JTAG_LOAD_FLASH_SYS_BUS_NO_HALT_CPU");
        end
        else if ($test$plusargs("JTAG_SYS_BUS_HALT_CPU")) begin
        $display("JTAG_SYS_BUS_HALT_CPU ");  
        end
        else
        $finish();
    endtask: debug_finish

    function int get_jtag_wire_mode(); 
        if (cjtag_mode)
            return 2;
        else
            return 4;
    endfunction: get_jtag_wire_mode

//write_memory_using_sys_bus{{{
  task write_memory_using_sys_bus(input uint64_t addr, input uint64_t data);
    int error;
    int r_data;
    read_dm_register(8'h38, r_data, error);
    if(error)
      $finish();

	  r_data &= ~((7 << 17) | (1 << 20));
	  r_data |= (2 << 17);

	  // write SBCS , select the access size
	  write_dm_register(8'h38, r_data, error);
	  if (error)
	  	$finish();
	  // write address
	  write_dm_register(8'h39, (addr & 32'hffffffff), error);
	  if (error)
	  	$finish();
//      if (riscv_xlen > 32) {
//	  	write_dm_register(8'h3a, ((addr >> 32u) & 32'hffffffff), error);
//	  	if (error)
//	  		$finish();
//	  }
	  // write data
//    if (size > 2) {
//	  	// for rv64, write the upper 32bit value
//	  	write_dm_register(8'h3d, ((data >> 32u) & 32'hffffffff), error);
//	  	if (error)
//	  		$finish();
//	  }
	  write_dm_register(8'h3c, (data & 32'hffffffff), error);
	  if (error)
	  	$finish();
	  // wait the write finish
	  for(int i=0; i<5; i++) begin: wait_the_write_finish
	  	read_dm_register(8'h38, r_data, error);
	  	if (error)
	  		$finish();
	  	// judge the sbbusy bit, if zero, mean access is finish
	  	if ( !((r_data >> 21) & 8'h1)) begin
	  		// write sbcs 0, clear sbreadonaddr , sbaccess, sbautoincrement, abreadondata
	  		write_dm_register(8'h38, 0, error);
	  		$finish();
	  	end
	  end
  endtask:write_memory_using_sys_bus
//}}}

    task halt_cpu(); 
        int error;
        int value;
        read_dm_register(8'h10, value, error);
        if (error)
            $finish();
        // set the haltreq bit
        value |= (1<<31);
        // clear the resumereq bit
        value &= ~(1<<30);
        write_dm_register(8'h10, value, error);
        if (error)
            $finish();
    endtask

	assign jtag_tck = tck;
	assign jtag_tms = tms;
	assign jtag_tdi = tdi;
	
	
	// maintain the state
	localparam TEST_LOGIC_RESET = 5'd0;
	localparam RUN_TEST_IDLE = 5'd1;
	localparam SELECT_DR_SCAN = 5'd2;
	localparam CAPTURE_DR  = 5'd3;
	localparam SHIFT_DR = 5'd4;
	localparam EXIT1_DR = 5'd5;
	localparam PAUSE_DR = 5'd6;
	localparam EXIT2_DR = 5'd7;
	localparam UPDATE_DR = 5'd8;
	localparam SELECT_IR_SCAN = 5'd9;
	localparam CAPTURE_IR = 5'd10;
	localparam SHIFT_IR = 5'd11;
	localparam EXIT1_IR = 5'd12;
	localparam PAUSE_IR = 5'd13;
	localparam EXIT2_IR = 5'd14;
	localparam UPDATE_IR = 5'd15;
	
	wire	  tap_idle;
	
	reg 	  tap_ir_valid;
	reg [4:0] tap_ir;
	reg [4:0] tap_ir_reg;
	reg [4:0] tap_ir_reg_next;

	reg        tap_dr_valid;
	reg [40:0] tap_dr;
	reg [40:0] tap_dr_reg;
	reg [40:0] tap_dr_reg_next;
	
	wire 	   tap_tdo_valid;
	reg [40:0] tap_tdo;
	reg [40:0] tap_tdo_reg;
	reg [40:0] tap_tdo_reg_next;
	
	wire [1:0] tap_dmi_op;
	wire [6:0] tap_dmi_addr;
	wire [31:0] tap_dmi_data;
	
	wire [1:0] tap_rsp_op;
	wire [6:0] tap_rsp_addr;
	wire [31:0] tap_rsp_data;
	
	reg [4:0] tap_state;
	reg [4:0] tap_state_next;
	
	always@(posedge tck or negedge rst_n) begin
		if (!rst_n) begin
			tap_state <= TEST_LOGIC_RESET;
			tap_ir_reg <= 5'b0;
			tap_dr_reg <= 41'b0;
			tap_tdo_reg <= 41'b0;
		end
		else begin
			tap_state <= tap_state_next;
			tap_ir_reg <= tap_ir_reg_next;
			tap_dr_reg <= tap_dr_reg_next;
			tap_tdo_reg <= tap_tdo_reg_next;
		end
	end
	
	always@(*) begin
		tap_state_next = tap_state;
		tap_ir_reg_next = tap_ir_reg;
		tap_dr_reg_next = tap_dr_reg;
		
		case(tap_state)
			TEST_LOGIC_RESET: 
				if (!tms)	
					tap_state_next = RUN_TEST_IDLE;
			RUN_TEST_IDLE:
				if (tms)
					tap_state_next = SELECT_DR_SCAN;
			SELECT_DR_SCAN: begin
				if (tms)
					tap_state_next = SELECT_IR_SCAN;
				else
					tap_state_next = CAPTURE_DR;
			end
			CAPTURE_DR: begin
				if (tms)
					tap_state_next = EXIT1_DR;
				else
					tap_state_next = SHIFT_DR;
			end
			SHIFT_DR: begin
				tap_dr_reg_next = {tdi, tap_dr_reg[40:1]};
				tap_tdo_reg_next = {jtag_tdo, tap_tdo_reg[40:1]};
				if (tms)
					tap_state_next = EXIT1_DR;
			end
			EXIT1_DR: begin
				if (tms)
					tap_state_next = UPDATE_DR;
				else
					tap_state_next = PAUSE_DR;
			end
			PAUSE_DR: begin
				if (tms)
					tap_state_next = EXIT2_DR;
			end
			EXIT2_DR: begin
				if (tms)
					tap_state_next = UPDATE_DR;
				else
					tap_state_next = CAPTURE_DR;
			end
			UPDATE_DR: begin
				if (tms)
					tap_state_next = SELECT_DR_SCAN;
				else
					tap_state_next = RUN_TEST_IDLE;
			end
			SELECT_IR_SCAN: begin
				if (tms)
					tap_state_next = TEST_LOGIC_RESET;
				else
					tap_state_next = CAPTURE_IR;
			end
			CAPTURE_IR: begin
				if (tms)
					tap_state_next = EXIT1_IR;
				else
					tap_state_next = SHIFT_IR;
			end
			SHIFT_IR: begin
				tap_ir_reg_next = {tdi, tap_ir_reg[4:1]};
				if (tms)
					tap_state_next = EXIT1_IR;
			end
			EXIT1_IR: begin
				if (tms)
					tap_state_next = UPDATE_IR;
				else
					tap_state_next = PAUSE_IR;
			end
			PAUSE_IR: begin
				if (tms)
					tap_state_next = EXIT2_IR;
			end
			EXIT2_IR: begin
				if (tms)
					tap_state_next = UPDATE_IR;
				else
					tap_state_next = SHIFT_IR;
			end
			UPDATE_IR: begin
				if (tms)
					tap_state_next = SELECT_DR_SCAN;
				else
					tap_state_next = RUN_TEST_IDLE;
			end
		endcase
	end
	
	always@(posedge tck or negedge rst_n) begin
		if (!rst_n)	 begin
			tap_ir <= 5'b0;
			tap_dr <= 41'b0;
			tap_tdo <= 41'b0;
			tap_ir_valid <= 1'b0;
			tap_dr_valid <= 1'b0;
		end
		else begin
			if (tap_state == EXIT1_IR)
				tap_ir <= tap_ir_reg;
			if (tap_state == EXIT1_DR) begin
				tap_dr <= tap_dr_reg;
				tap_tdo <= tap_tdo_reg;
			end
			
			if (tap_state == EXIT1_IR)
				tap_ir_valid <= 1'b1;
			else
				tap_ir_valid <= 1'b0;
			
			if (tap_state == EXIT1_DR) 
				tap_dr_valid <= 1'b1;
			else
				tap_dr_valid <= 1'b0;
		end
	end
	
	assign tap_tdo_valid = tap_dr_valid;
	
	assign tap_idle = tap_state == RUN_TEST_IDLE;
	
	assign tap_dmi_op = tap_dr[1:0];
	assign tap_dmi_addr = tap_dr[40:34];
	assign tap_dmi_data = tap_dr[33:2];
	
	assign tap_rsp_op = tap_tdo[1:0];
	assign tap_rsp_addr = tap_tdo[40:34];
	assign tap_rsp_data = tap_tdo[33:2];


	task flash_ate_test(input reg [8*100:1]ate_dat_file);  //{{{
        integer        data_file; // file handler
        integer        scan_file; // file handler
        integer        i_line, pat_error_number;
        reg      [7:0] pat_data;
        reg      [3:0] pat_freq, pat_freq_pre;
        reg            gen_clock;
        integer  clk_number;
        `define NULL 0    

        data_file = $fopen(ate_dat_file, "r");
        i_line = 0;
        tck = 1'b0;
        gen_clock = 1'b0;
        pat_freq = 4'hA;
        pat_freq_pre = 4'hA;
        pat_error_number = 0;
        clk_number = 0;
        if (data_file == `NULL)
        begin
          $display("Error : ATE data file can not found!\n");
          $finish;
        end

        // use 40MHz clock to generate the JTAG if signals for ATE;
        while(1)
        fork 
            // clocks;
            begin 
		        #12.5 gen_clock = 1'b1;
		        #12.5 gen_clock = 1'b0;
            end
            
            // 
            begin
                @(posedge gen_clock);
                # 1;
                if (clk_number == (40/pat_freq - 1)) 
                    clk_number = 0; 
                else 
                    clk_number = clk_number + 1;
            end

            begin
                @(posedge gen_clock);
                // read the data file;
                if (clk_number == 0) 
                begin 
                    i_line = i_line + 1;
                    if (!$feof(data_file))
                    begin 
                        pat_freq_pre = pat_freq;
                        scan_file = $fscanf(data_file, "%h %b\n", pat_freq, pat_data); 
                        if (pat_freq_pre != pat_freq)
                            $display("VCS_Infor : JTAG Frequency Change from %h to %h at time %t.......... ", pat_freq_pre, pat_freq, $time);
                        if ((i_line % 1000) == 0)
                            $display("Processing ATE vector line # %d", i_line);
                    end
                    else 
                    begin 
                        $display("VCS_Infor : ATE data file Ending !!!\n");
                        #1000
                        $display ("VCS_Infor : Data file processing finished at time %t........", $time);
                        if (pat_error_number == 0) 
                        begin
                            $display("\nVCS_Infor : Simulation PASSED with Zero Compare Error;\n ");
                            fail(0);
                        end
                        else 
                        begin
                            $display("\nVCS_Infor : Simulation FAILED with %d Compare Errors.\n", pat_error_number);
                            fail(32);
                        end
                        $finish();
                    end
                end 

            end
                
            begin // {{{
                @(posedge gen_clock);
                tms = pat_data[2];
                tdi = pat_data[1];
                if (pat_freq == 4'h4) 
                begin 
                    if (clk_number == 4'h2)
                        tck = pat_data[3];
                    else if (clk_number == 4'h7)
                        tck = 1'h0;
                end
                else if (pat_freq == 4'hA) 
                begin 
                    if (clk_number == 4'h1)
                        tck = pat_data[3];
                    else if (clk_number == 4'h3)
                        tck = 1'h0;
                end

                if ((pat_freq == 4'h4) && (clk_number == 5) || ((pat_freq == 4'hA) && (clk_number == 2))) 
                begin 
                    if (pat_data[0] !== 1'bx)
                    begin 
                        if (pat_data[0] !== jtag_tdo)
                        begin 
                            $display ("Error : ATE Vector Line %d Compare Fail at time %t, expected - %b vs gotted - %b;", i_line, $time, pat_data[0], jtag_tdo);
                            pat_error_number = pat_error_number + 1;
                        end
                    end
                end
            end // }}}

            begin
                if(pat_data[5]==1)
                begin
                    VPP0=1'b1;
                end
                else 
                begin
                    VPP0=1'bz;
                end
            end

            begin
                if(pat_data[6]==1)
                begin
                    VPP1=1'b1;
                end
                else
                begin
                    VPP1=1'bz;
                end
            end

            begin
                if(pat_data[7]==1)
                begin
                    TM0=1'b1;
                end
                else
                begin
                    TM0=1'bz;
                end
            end

        join
    
	endtask // flash_ate_test; }}}

task fail;
  input [31:0] code;
  begin
    if (!code) begin
     $display("*PASS: SPI fail(code=%0x) call @%t.", code, $time);
     $display("\nInfor : Simulation PASSED with PassCode 0x%08h at time %t.", code, $time);
    //     tb_top.sys_trans_if.test_pass = 1'b1;
    //     tb_top.sys_trans_if.exit_code = 0;            

    //     $fdisplayh(fp, "%08x: %08x", 32'h4000, 32'h50415353);
    //     $fdisplayh(fp, "%08x: %08x", 32'h4008, 32'h12345678);
    //     $fdisplayh(fp, "%08x: %08x", 32'h400c, 32'h87654321);
      #100000 $finish();
    end
    else begin
    //      fail_flag = 1;
    //      tb_top.sys_trans_if.test_pass = 1'b0; 
    //      tb_top.sys_trans_if.exit_code = code;

        //$fdisplayh(fp, "%08x: %08x", 32'h3000, 32'h19851201);
    //    $fdisplayh(fp, "%08x: %08x", 32'h4000, 32'h19851201);
    //    $fdisplayh(fp, "%08x: %08x", 32'h4008, 32'h4641494c);
    //    $fdisplayh(fp, "%08x: %08x", 32'h400c, code);
    $display("*E error: SPI fail(code=%0x) call @%t.", code, $time);
    $display("\nInfor : Simulation FAILED with FailCode 0x%08h at time %t.", code, $time);
      // FIXME: should turn this? no mem dump if failed
      #10000 $finish();
    end
  end
endtask

endmodule: jtag_model
