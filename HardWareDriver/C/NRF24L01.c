/*    
      ____                      _____                  +---+
     / ___\                     / __ \                 | R |
    / /                        / /_/ /                 +---+
   / /   ________  ____  ___  / ____/___  ____  __   __
  / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
 / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
 \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
                                                 / /
                                            ____/ /
                                           /_____/
 NRF24L01.c file
 Writers: Pony (Camel)
 Author E-mail: 375836945@qq.com
 Compiler environment: MDK-Lite Version: 4.23
 First edition Time: 2014-01-28
 Features:
 1.NRF24L01 initialization
 2. stm32 interface with the interface hardware SPI_1
 ------------------------------------
 */

#include "NRF24L01.h"
#include "SPI.h"
#include "ReceiveData.h"
#include "delay.h"
#include "UART1.h"
#include "stdio.h"
uint8_t NRF24L01_RXDATA[RX_PLOAD_WIDTH]; //nrf24l01 received data 
uint8_t NRF24L01_TXDATA[RX_PLOAD_WIDTH]; //nrf24l01 data needs to be sent

// modify the receive and transmit addresses, can be used for a plurality of aircraft flying in the same area, the data undisturbed
u8 RX_ADDRESS[RX_ADR_WIDTH] = { 0x34, 0xc3, 0x10, 0x10, 0x00 };	// address received

// write register
uint8_t NRF_Write_Reg(uint8_t reg, uint8_t value) {
	uint8_t status;
	SPI_CSN_L();
	status = SPI_RW(reg);
	SPI_RW(value);	// Write data
	SPI_CSN_H(); // disable the device
	return status;
}

// read register
uint8_t NRF_Read_Reg(uint8_t reg) {
	uint8_t reg_val;
	SPI_CSN_L();
	SPI_RW(reg);
	reg_val = SPI_RW(0); // read the register returns data
	SPI_CSN_H();
	return reg_val;
}

// write buffer
uint8_t NRF_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars) {
	uint8_t i;
	uint8_t status;
	SPI_CSN_L(); // gating device
	status = SPI_RW(reg); // write register address
	for (i = 0; i < uchars; i++) {
		SPI_RW(pBuf[i]); // write data
	}
	SPI_CSN_H(); // disable the device *
	return status;
}

// read buffer
uint8_t NRF_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars) {
	uint8_t i;
	uint8_t status;
	SPI_CSN_L(); // gating device
	status = SPI_RW(reg); // write register address
	for (i = 0; i < uchars; i++) {
		pBuf[i] = SPI_RW(0); // read return data
	}
	SPI_CSN_H(); // disable the device
	return status;
}

// write packet
void NRF_TxPacket(uint8_t *tx_buf, uint8_t len) {
	SPI_CE_L(); // StandBy I mode
	NRF_Write_Buf(WR_TX_PLOAD, tx_buf, len); // load data
	SPI_CE_H(); // set high CE, excitation data transmission
}

//initialization
char NRF24L01_INIT(void) {
	SPI1_INIT();

	//check if NRF24L01 is in the SPI bus
	NRF24L01_Check();

	//set the NRF RX Address,priority to the latest address in eeprom
	// NRF24L01 frequency receiver address, a priority match address on the eeprom
	NRFmatching();
}

// receive mode
void SetRX_Mode(void) {
	SPI_CE_L();
	NRF_Write_Reg(FLUSH_RX, 0xff); // Clear TX FIFO register
	NRF_Write_Buf(NRF_WRITE_REG + RX_ADDR_P0, (uint8_t*) RX_ADDRESS,
	RX_ADR_WIDTH); // write the RX node address
	NRF_Write_Reg(NRF_WRITE_REG + EN_AA, 0x01); // enable automatic answering channel 0
	NRF_Write_Reg(NRF_WRITE_REG + EN_RXADDR, 0x01); // can address the reception channel 0
	NRF_Write_Reg(NRF_WRITE_REG + RF_CH, 40); // set the RF communication frequency
	NRF_Write_Reg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); // channel 0 select valid data width
	NRF_Write_Reg(NRF_WRITE_REG + RF_SETUP, 0x0f); // set the transmission parameters TX, 0db gain, 2Mbps, low noise gain Open
	NRF_Write_Reg(NRF_WRITE_REG + CONFIG, 0x0f); // Configure parameters of the basic mode of operation; PWR_UP, EN_CRC, 16BIT_CRC, reception mode
	SPI_CE_H();
	//printf("NRF24L01 Set to Receiving Mode,RX_ADDR 0x%x...\r\n",RX_ADDRESS[4]);
}

// query break
void Nrf_Irq(void) {
	uint8_t sta = NRF_Read_Reg(NRF_READ_REG + NRFRegSTATUS);
	if (sta & (1 << RX_DR)) // receiving flag in rotation
			{
		NRF_Read_Buf(RD_RX_PLOAD, NRF24L01_RXDATA, RX_PLOAD_WIDTH);	// read receive payload from RX_FIFO buffer
		ReceiveDataFormNRF(); // make changes themselves
		NRF_Write_Reg(0x27, sta); // Clear interrupt flag nrf
		sta = 0;
	}

}

// receiver function
u8 NRF24L01_RxPacket(u8 *rxbuf) {
	u8 sta;
	//SPI2_SetSpeed(SPI_SPEED_4); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）
	sta = NRF_Read_Reg(NRFRegSTATUS);  //读取状态寄存器的值
	NRF_Write_Reg(NRF_WRITE_REG + NRFRegSTATUS, sta); //清除TX_DS或MAX_RT中断标志
	if (sta & RX_OK) //接收到数据
	{
		NRF_Read_Buf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); //读取数据
		NRF_Write_Reg(FLUSH_RX, 0xff); //清除RX FIFO寄存器
		return 0;
	}
	return 1; //没收到任何数据
}

//判断SPI接口是否接入NRF芯片是否可用
u8 NRF24L01_Check(void) {
	u8 buf[5] = { 0xC2, 0xC2, 0xC2, 0xC2, 0xC2 };
	u8 buf1[5];
	u8 i = 0;

	/*写入5 个字节的地址.  */
	NRF_Write_Buf(NRF_WRITE_REG + TX_ADDR, buf, 5);

	/*读出写入的地址 */
	NRF_Read_Buf(TX_ADDR, buf1, 5);

	/*比较*/
	for (i = 0; i < 5; i++) {
		if (buf1[i] != 0xC2)
			break;
	}

	if (i == 5) {
		printf("NRF24L01 found...\r\n");
		return 1;
	}        //MCU 与NRF 成功连接
	else {
		printf("NRF24L01 check failed...\r\n");
		return 0;
	}        //MCU与NRF不正常连接
}

