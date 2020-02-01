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
 BT.c file
 Written by: Camel, nieyong
 Author E-mail: 375836945@qq.com
 Compilation environment: MDK-Lite Version: 4.23
 Initial release time: 2014-01-28
 Features:
 1.Bluetooth transparent transmission module power enable BT_EN ---> PB2
 2.Turn on the Bluetooth power-> BT_EN = 1;
 ------------------------------------
 */

#include "BT.h"
#include "delay.h"
#include "UART1.h"
#include "stdio.h"
#include "string.h"
#include "Sys_Fun.h"
#include "control.h"
#include "Led.h"

// Instantiate a Bluetooth structure
BTtype CrazeponyBT;

// Whether Bluetooth needs to write parameter flags
float BTstate;

/********************************************
 * Bluetooth power initialization function
 ********************************************/
void BT_PowerInit(void) {
	RCC->APB2ENR |= 1 << 3;     // Enable the PORTB clock
	GPIOB->CRL &= 0XFFFFF0FF;   // PB2 push-pull output
	GPIOB->CRL |= 0X00000300;
	GPIOB->ODR |= 1 << 2;        // Pull PB2
	BT_on();

}

// Command return
char Cmdreturn[CmdreturnLength];

/********************************************
 * Write a command packet to Bluetooth
 ********************************************/
void Uart1SendaBTCmd(const char *p) {
	uint8_t i;

	// Release instruction receive buffer
	for (i = 0; i < CmdreturnLength; i++)
		Cmdreturn[i] = 0;

	LedA_on;

	// After writing an instruction, delay 500ms to receive the cache again
	delay_ms(100);
	LedA_off;

	for (i = 0; i < strlen(p); i++)
		UART1_Put_Char(*(p + i));

	// After writing an instruction, delay 500ms to receive the cache again
	delay_ms(100);

	i = 0;
	// When the serial port buffer is not empty, assign the serial port buffer to the instruction result buffer
	while (UartBuf_Cnt(&UartRxbuf) != 0)
		Cmdreturn[i++] = UartBuf_RD(&UartRxbuf);
}

/********************************************
 * Determine if a command returns equal to the set value
 * Return value: 0-> the instruction does not match the set value
 * 1-> The instruction matches the set value
 ********************************************/
char CmdJudgement(const char *p) {
	uint8_t i;
	for (i = 0; i < strlen(p); i++)
		if (Cmdreturn[i] != *(p + i))
			break;
	if (i != strlen(p))
		return 0;
	return 1;
}

const char ATcmdAsk[] = { "AT" };
const char ATcmdAnswer[] = { "OK" };

const char ATcmdNameAsk[] = { "AT+NAME?" };
const char ATcmdNameAnswer[] = { "OK+NAME:Crazepony2" };	//{BT_BAUD_AT};//
const char ATcmdNameSet[] = { "AT+NAMECrazepony2" }; // Set the name of the Bluetooth device: Crazepony, of course, you can modify it to what ever you want ...
const char ATcmdCodeAsk[] = { "AT+PIN?" };
const char ATcmdCodeAnswer[] = { "OK+PIN:1234" };
const char ATcmdCodeSet[] = { "AT+PIN1234" };          //Bluetooth pairing default password is 1234

const char ATcmdRenewAsk[] = { "AT+RENEW" };	//reset
const char ATcmdRenewAnswer[] = { "OK+RENEW" };

const char ATcmdBaudAsk[] = { "AT+BAUD?" };
const char ATcmdBaudAnswer[] = { "OK+BAUD:115200" };
const char ATcmdBaudSet[] = { "AT+BAUD4" };            //Modify Here, you can modify Bluetooth baud rate



// HM-06 module, the configuration of Bluetooth 2.1 module
//baud1--->1200
//baud2--->2400
//baud3--->4800
//baud4--->9600
//baud5--->19200
//baud6--->38400
//baud7--->57600
//baud8--->115200

// HM-11 module, which is the configuration of Bluetooth 4.0 BLE module.
// Note the difference from HM-06.
//baud0--->9600
//baud4--->115200                                        

// Poll all possible baud rates of the Bluetooth module to get the current baud rate
// and configure its baud rate to 115200
u32 BT_Scan_Baud(void) {
	// Bluetooth baud rate table, put 9600 (the default baud rate) and 230400 (the legacy bug of hm-06) to the front
	// 115200 (the baud rate to be configured) is put at the end
	static u32 bandsel[9] = { 230400, 9600, 1200, 2400, 4800, 19200, 38400,
			57600, 115200 };
	uint8_t i;

	for (i = 0; i < 9; i++) {
		UART1_init(SysClock, bandsel[i]);
		Uart1SendaBTCmd(ATcmdAsk);
		if (CmdJudgement(ATcmdAnswer) == true) {
			return bandsel[i];
		}
	}

	return 115200;
}

/********************************************
Write Bluetooth parameter function
 ********************************************/
void BT_ATcmdWrite(void) {
	uint8_t i;
	static u32 BT_CurBaud;
	static u32 bandsel[9] = { 230400, 9600, 1200, 2400, 4800, 19200, 38400,
			57600, 115200 };

	Uart1SendaBTCmd(ATcmdAsk);
	if (CmdJudgement(ATcmdAnswer) == true) {
		// HM-11 module is already 115200 baud rate, no configuration required
		return;
	}

	printf("BT baud check and init begin.printf is useless.\r\n\r\n");

	BT_CurBaud = BT_Scan_Baud();

	// First check if the serial port of the Bluetooth module has been configured as 115200
	if (BT_CurBaud != BT_BAUD_Set) {

		// Change the name of Bluetooth to Crazepony
		Uart1SendaBTCmd(ATcmdNameSet);

		// Modify the Bluetooth baud rate to 115200
		Uart1SendaBTCmd(ATcmdBaudSet);

		// LED flashes which baud rate is the original Bluetooth module
		for (i = 0; i < 9; i++) {
			LedA_on;
			LedB_on;
			delay_ms(1000);
			LedC_on;
			LedD_on;
			delay_ms(1000);
			LedA_off;
			LedB_off;
			delay_ms(1000);
			LedC_off;
			LedD_off;
			delay_ms(1000);

			if (BT_CurBaud == bandsel[i]) {
				break;
			}
		}
	} else {
		// It is already 115200, can communicate directly
		printf("BT module baud is 115200 okay\r\n");
	}

	// The final STM32 baud rate is set back to 115200
	UART1_init(SysClock, BT_BAUD_Set);

	printf("\r\nBT baud check and init end.\r\n");

}

