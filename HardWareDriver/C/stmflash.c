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
 moto.c file
 编写者：小马  (Camel)
 作者E-mail：375836945@qq.com
 编译环境：MDK-Lite  Version: 4.23
 初版时间: 2014-01-28
 功能：
 1.内部flash初始化，相当于一个片内的模拟EEPROM
 2.具体操作细节还有些BUG，有些地址读出来的数据有问题，我暂时没找到原因，望各路热血青年一起来解决
 ------------------------------------
 */
#include "stmflash.h"
#include "delay.h"
#include "UART1.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////
//stm32f103t8u6--->64K Bytes  flash
//The last page address stm32 small capacity for 0x08007c00, end address 0x08007fff
//Each page size of 1K bytes
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//Unlock the STM32 FLASH
//////////////////////////////////////////////////////////////////////////////////
void STMFLASH_Unlock(void)
{
	FLASH->KEYR = FLASH_KEY1; //Written unlock sequence.
	FLASH->KEYR = FLASH_KEY2;
	printf("Inner FLASH unlock success...\r\n");
}
//////////////////////////////////////////////////////////////////////////////////
//flash lock
//////////////////////////////////////////////////////////////////////////////////
void STMFLASH_Lock(void)
{
	FLASH->CR |= 1 << 7; //Lock
}
//////////////////////////////////////////////////////////////////////////////////
//Get FLASH status
//////////////////////////////////////////////////////////////////////////////////
u8 STMFLASH_GetStatus(void)
{
	u32 res;
	res = FLASH->SR;
	if (res & (1 << 0))
		return 1;		    //busy
	else if (res & (1 << 2))
		return 2;	//Programming error
	else if (res & (1 << 4))
		return 3;	//Write protect error
	return 0;						//The operation is complete
}
//////////////////////////////////////////////////////////////////////////////////
//Await completion
//time: to the length of delay
//Returns: status.
//////////////////////////////////////////////////////////////////////////////////
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res = STMFLASH_GetStatus();
		if (res != 1)
			break;						//Not busy, no wait, exit.
		delay_us(1);
		time--;
	} while (time);
	if (time == 0)
		res = 0xff;						//TIMEOUT
	return res;
}
//////////////////////////////////////////////////////////////////////////////////
//Erase page
//paddr: page address
//Returns: Implementation
//////////////////////////////////////////////////////////////////////////////////
u8 STMFLASH_ErasePage(u32 paddr)
{
	u8 res = 0;
	res = STMFLASH_WaitDone(0X5FFF);//Waiting for the end of the last operation,> 20ms
	if (res == 0)
	{
		FLASH->CR |= 1 << 1;						//Page Erase
		FLASH->AR = paddr;						//Settings page address
		FLASH->CR |= 1 << 6;						//Begin erasing
		res = STMFLASH_WaitDone(0X5FFF);	//Wait for the operation ends,> 20ms
		if (res != 1)						//Non-busy
		{
			FLASH->CR &= ~(1 << 1);						//Clear Page Erase flag.
		}
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////////
//FLASH half-word written in a specified address
//faddr: specified address (this address must be a multiple of 2 !!)
//dat: data to be written
//Returns: the case of written
//////////////////////////////////////////////////////////////////////////////////
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;
	res = STMFLASH_WaitDone(0XFF);
	if (res == 0)						//OK
	{
		FLASH->CR |= 1 << 0;						//Programming Enable
		*(vu16*) faddr = dat;						//data input
		res = STMFLASH_WaitDone(0XFF);						//Await completion
		if (res != 1)						//Successful operation
		{
			FLASH->CR &= ~(1 << 0);						//Clear PG position.
		}
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////////
//The read address specified half word (16 bits)
//faddr: read address
//Return Value: corresponding data.
//////////////////////////////////////////////////////////////////////////////////
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*) faddr;
}
#if STM32_FLASH_WREN	//If enabled write
//Do not check writing
//WriteAddr: starting address
//pBuffer: Pointer Data
//NumToWrite: half-word (16-bit) number
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)
{
	u16 i;
	for (i = 0; i < NumToWrite; i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr, pBuffer[i]);
		WriteAddr += 2;						//Address incremented by two.
	}
}
//////////////////////////////////////////////////////////////////////////////////
//Start writing data from the specified address length specified
//WriteAddr: start address (this address must be a multiple of 2 !!)
//pBuffer: Pointer Data
//NumToWrite: half-word (16-bit) number (the number of 16-bit data is to be written.)
//////////////////////////////////////////////////////////////////////////////////
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //byte
#else 
#define STM_SECTOR_SIZE	2048
#endif

u16 STMFLASH_BUF[STM_SECTOR_SIZE / 2];						//At most 2K bytes

//////////////////////////////////////////////////////////////////////////////////
void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)
{
	u32 secpos;	   //Sector address
	u16 secoff;	   //The sector offset address (16-bit word calculation)
	u16 secremain; //Within a sector remaining addresses (16-bit word calculation)
	u16 i;
	u32 offaddr;   //After removing the address 0X08000000
	if (WriteAddr < STM32_FLASH_BASE
			|| (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
		return;   //Illegal Address
	STMFLASH_Unlock();						//Unlock
	offaddr = WriteAddr - STM32_FLASH_BASE;		//The actual offset.
	secpos = offaddr / STM_SECTOR_SIZE;	//Sector address 0 ~ 127 for STM32F103RBT6
	secoff = (offaddr % STM_SECTOR_SIZE) / 2;//Offset in the sector (2 bytes as a basic unit.)
	secremain = STM_SECTOR_SIZE / 2 - secoff;		//The remaining sector space
	if (NumToWrite <= secremain)
		secremain = NumToWrite;		//It is not larger than the sector range
	while (1)
	{
		STMFLASH_Read(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF,
		STM_SECTOR_SIZE / 2);	//Read the contents of the entire sector
		for (i = 0; i < secremain; i++)		//Check data
		{
			if (STMFLASH_BUF[secoff + i] != 0XFFFF)
				break;		//Be erased
		}
		if (i < secremain)		//Be erased
		{
			STMFLASH_ErasePage(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE);//Erase this sector
			for (i = 0; i < secremain; i++)		//copy
			{
				STMFLASH_BUF[i + secoff] = pBuffer[i];
			}
			STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE,
					STMFLASH_BUF, STM_SECTOR_SIZE / 2);	//Write the entire sector
		}
		else
			STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain);//Wrote already erased and written directly to the sector remaining zone.
		if (NumToWrite == secremain)
			break;		//Been finalized
		else		//Write is not completed
		{
			secpos++;				//Sector address by 1
			secoff = 0;				//Offset position is 0
			pBuffer += secremain;  	//Pointer offset
			WriteAddr += secremain;	//Write address offset
			NumToWrite -= secremain;	//Bytes (16 bits) number down
			if (NumToWrite > (STM_SECTOR_SIZE / 2))
				secremain = STM_SECTOR_SIZE / 2;//The next sector or never finish
			else
				secremain = NumToWrite;	//Next sector can be finished
		}
	};
	STMFLASH_Lock();	//Lock
}
#endif

//////////////////////////////////////////////////////////////////////////////////
//Reading of the data from the specified address length specified
//ReadAddr: starting address
//pBuffer: Pointer Data
//NumToWrite: half-word (16-bit) number
//////////////////////////////////////////////////////////////////////////////////
void STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead)
{
	u16 i;
	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);	//Reading two bytes.
		ReadAddr += 2;	//Offset 2 bytes.
	}
}

//////////////////////////////////////////////////////////////////////////////////
//WriteAddr: starting address
//WriteData: data to be written
//////////////////////////////////////////////////////////////////////////////////
void Test_Write(u32 WriteAddr, u16 WriteData)
{
	STMFLASH_Write(WriteAddr, &WriteData, 1);	//Write a letter
}

