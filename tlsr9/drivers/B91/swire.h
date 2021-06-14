/*
 * Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of TELINK nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/********************************************************************************************************
 * @file	swire.h
 *
 * @brief	This is the header file for B91
 *
 * @author	Driver Group
 *
 *******************************************************************************************************/

#ifndef _SWIRE_H_
#define _SWIRE_H_
#include "compiler.h"
#include "reg_include/register_b91.h"



/**
 * @brief     This function resets the SWIRE module.
 * @return    none
 */
static inline void swire_reset(void)
{
	 reg_rst0 &= (~FLD_RST0_SWIRE);
	 reg_rst0 |= (FLD_RST0_SWIRE);
}
/**
 * @brief     This function is used to wait for the completion of writing.
 * @return    none
 */
static inline void swire_wait_wr_done(void)
{
	while (reg_swire_ctl & FLD_SWIRE_WR);
}
/**
 * @brief     This function is used to write cmd.
 * @param[in] cmd 	- cmd value.
 * @return    none
 */
static inline void swire_master_write_cmd(unsigned char cmd)
{
	reg_swire_data = cmd;
	reg_swire_ctl = (FLD_SWIRE_CMD | FLD_SWIRE_WR );
	swire_wait_wr_done();
}
/**
 * @brief     This function is used to write data.
 * @param[in] data 	- data value.
 * @return    none
 */
static inline void swire_master_write_data(unsigned char data)
{
	reg_swire_data = data;
	reg_swire_ctl =  FLD_SWIRE_WR ;
	swire_wait_wr_done();
}
/**
 * @brief     This function is to disable fifo mode.
 *			  The default is this mode. When multi-byte reads and writes, the address will automatically increase by 1.
 *			  For example, for write instructions, the address is 1, and the data is 0x11, 0x22,
 *			  then 0x11 will be written to address 1, and 0x22 will be written to address 2.
 * @return    none
 */
static inline void swire_fifo_mode_dis(void)
{
	BM_CLR(reg_swire_id, FLD_SWIRE_FIFO_MODE);

}
/**
 * @brief     This function is to enable fifo mode.
 *			  When multi-byte read and write, the address will not be automatically incremented by 1,
 *			  but the value is written to the same address.
 *			  For example, if you want to operate the fifo register on the slave side, you will use this function.
 * @return    none
 */
static inline void swire_fifo_mode_en(void)
{
	BM_SET(reg_swire_id, FLD_SWIRE_FIFO_MODE);

}
/**
 * @brief     This function is to send a specific timing to reset the status of the slave device.
 * 			  When the master device connects the DP pin of the slave device, this function must be called first,
 * 			  because the DP pin of the slave device is USB function by default. Only after calling this function to send a specific timing,
 * 			  the slave device will be enabled as the swire function.
 * 			  When communication fails, you can also try to call this function to resume communication.
 *
 * @param[in] dp_through_swire_en - If the master is connected to the DP pin of the slave device, this parameter needs to be set to 1.
 * @return	  none.
 */
void swire_sync (unsigned char dp_through_swire_en);
/**
 * @brief     This function is to set GPIO_PC0 as the swm function.
 * @return	  none.
 */
void swire_set_swm_en(void);
/**
 * @brief     This function is to set GPIO_PA7 as the sws function.
 * @return	  none.
 */
void swire_set_sws_en(void);

/**
 * @brief     This function is to set the clock of the swire module,
 * 			  the clock source of the swire module is HCLK, the clock of the actual swire module:  swire_clock = HCLK/div.
 * 			  swire_clock cannot be greater than HCLK.(div cannot be 0).
 * 			  Whether the chip as a master or slave, clock source are the following principles:
 * 			  the clock for sending data is swire_clock, and the clock for detecting received data is HCLK.
 *
 * @return	  none.
 */
void swire_set_clk(unsigned int swire_clock);

/**
 * @brief     This function is used to set the ID of the slave.
 *			  When the host accesses multiple slaves, the device can be distinguished by the ID of the slave.
 * @param[in] id	- slave id.The setting range is 0~0x1f.
 * @return	  none.
 */
void swire_set_slave_id(unsigned char id);
/**
 * @brief     This function is used by the master device to write data to the slave device.
 *			  The swire protocol is to send the address first and then send the data.
 * @param[in] slave_id	- slave id.The setting range is 0~0x1f.
 * 						  [6:5]: this function tells the slave to operate in byte/half-word/word.
 * 						  		 the reason is that some addresses on the slave side can only be operated with 4 bytes.
 * 						  		 10--32bit; 01--16bit ; 00--8bit
 * 						  [4:0]: ID
 * @param[in] addr 		- slave address(little endian format).
 * @param[in] addr_len 	- slave address byte length,It is determined by the slave device, for example: B91s is four bytes, B85s is three bytes.
 * @param[in] data 		- The data that needs to be written to the slave.
 * @param[in] data_len	- data length.
 * @return	  none.
 */

void swire_master_write(unsigned char slave_id,unsigned char *addr, unsigned char addr_len,unsigned char *data,unsigned int data_len);
/**
 * @brief      This function is used by the master device to read data to the slave device.
 *			   The swire protocol is to send the address first and then wait to read the data returned by the slave.
 * @param[in] slave_id	 - slave id.The setting range is 0~0x1f.
 * 						 	 [6:5]: this function tells the slave to operate in byte/half-word/word.
 * 						  			the reason is that some addresses on the slave side can only be operated with 4 bytes.
 * 						  			10--32bit; 01--16bit ; 00--8bit
 * 						  	 [4:0]: ID
 * @param[in]  addr 	 - slave address(little endian format).
 * @param[in]  addr_len  - slave address byte length,It is determined by the slave device, for example: B91s is four bytes, B85s is three bytes.
 * @param[out] data 	 - this is the data read from the slave.
 * @param[in]  data_len	 - data length.
 * @return	   0:read timeout  1:read success.
 */
unsigned char  swire_master_read (unsigned char slave_id,unsigned char *addr, unsigned char addr_len,unsigned char *data,unsigned int data_len);

#endif
