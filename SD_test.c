/*
    This is a minimal C program executed on the FPGA version of Patmos.
    An embedded Hello World program: a blinking SD_test.

    Additional to the blinking SD_test we write to the UART '0' and '1' (if available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <machine/spm.h>
#include <machine/patmos.h>

#include "libcorethread/corethread.h"
#include "libmp/mp.h"
#include <machine/uart.h>

#define SD_ctrl ( *( ( volatile _IODEV unsigned * ) 		0xf00C0004 ) )
#define SD_cmd1 ( *( ( volatile _IODEV unsigned * ) 		0xf00C0008 ) )
#define SD_cmd2 ( *( ( volatile _IODEV unsigned * ) 		0xf00C000C ) )
#define SD_rdy ( *( ( volatile _IODEV unsigned * ) 			0xf00C0010 ) )
#define SD_cmdresp1 ( *( ( volatile _IODEV unsigned * ) 	0xf00C0014 ) )
#define SD_cmdresp2 ( *( ( volatile _IODEV unsigned * ) 	0xf00C0018 ) )
#define SD_clkdiv ( *( ( volatile _IODEV unsigned * ) 		0xf00C0020 ) )
#define SD_dataQueue ( *( ( volatile _IODEV unsigned * ) 	0xf00C010C ) )
#define UART ( *( ( volatile _IODEV unsigned * ) 			0xF0080004 ) )
#define UART_rdy ( *( ( volatile _IODEV unsigned * ) 		0xF0080001 ) )
#define LED ( *( ( volatile _IODEV unsigned * ) 0xf0090000 ) )
#define SEND_NOPS 0x01
#define SEND_CMD 0x02
#define SD_freq  20000000//13000000//100000//100000//
#define clk_freq 80000000

int init_SD(int uart);
long long sd_send(int cmd, int arg, int cmdtype, int uart);
void send_uart(char* str);

int main() {
	
	int i, j, l, k, hardware=0;
	char msg[100];
	char buffer[100];
	k=hardware;
	if(hardware){
	for(i=0;i<400;i++)	
		for(j=0;j<400;j++)
			LED = 1;
	}

	SD_clkdiv = (clk_freq/2)/SD_freq;

	sd_send(0x0,0x74,SEND_NOPS,0);

	if(hardware){
	k = init_SD(hardware);
	} else {
		sd_send(16,512,SEND_CMD,hardware);
	} 
	if(hardware){
	SD_clkdiv = (clk_freq/2)/12000000;
	for(j=511;j<512;j++){
	 for(i=0 ; i< 128; i++){
		SD_dataQueue = i;
	}}}

	sd_send(24,j,SEND_CMD,hardware);
	sd_send(17,j,SEND_CMD,hardware);
	
	if(hardware){
	snprintf(buffer,100, "block data %d\n",j);
	send_uart(buffer);
	
	for(i=0 ; i< 128; i++){
		l = SD_dataQueue;
		LED = 0;
		snprintf(buffer,100, " %08X",l);
			send_uart(buffer);
	}
	send_uart("\n");
	}
	
	for (;;) {
		if(k==0){
		for(l=0; l<10; l++){
			for(i=0;i<2000;i++)	
				for(j=0;j<2000;j++)
					LED = 1;
			for(i=0;i<2000;i++)	
				for(j=0;j<2000;j++){
					LED = 0;
					}
		}
		for(i=0;i<2000;i++)	
			for(j=0;j<2000;j++)
				for(l=0;l<20;l++)
					LED = 0;
		}
	}
	
	return 0;
}

	


int init_SD( int uart) {
	int old=0;
	char buffer[100];
	long long resp;
	if(uart){send_uart("Initializing..\n");}
	resp = sd_send(0,0,SEND_CMD,uart);
	resp = resp & 0xFF;
		if( resp == 0x01 ){
			if(uart){send_uart("response to reset OK\n");}
			resp = sd_send(8,0x1AA,SEND_CMD,uart);
			//resp &= 0xFFF;
			
				if( (resp & 0xFF00000000) == 0x0100000000 ){
						if(uart){send_uart("cmd8 legal\n");}
						if( (resp & 0xFF) == 0xAA ){
						if(uart){send_uart("new card \n");}
						old = 0;
							if( ((resp >> 8) & 0xFF) != 0x1 ){
								if(uart){send_uart("failed voltage check\n");}
								return 5;
							}
						}
				}	else {
						if( (resp & 0xFF00000000) == 0x0500000000 ){
						if(uart){send_uart("cmd8 illegal\n");}
						old = 1;
						if(uart){send_uart("old card \n");}
						}
					}

			if(uart){send_uart("Waiting for idle");}

			if(old){
			for(int i=0;i<1000;i++){
				resp = sd_send(1,0,SEND_CMD,0);
				resp &= 0xFF;
				if(resp == 0x00){break;}
			}}
			if(!old){	
			for(int i=0;i<100;i++){
				sd_send(55,0,SEND_CMD,0);
				resp = sd_send(41,0x40000000,SEND_CMD,1);
				resp &= 0xFF;
				if(resp == 0x00){break;}
				//if(uart){send_uart(".");}
			}}
			if(resp != 0x00){
				if(uart){send_uart(" failed");}
				return 5;}
			if(uart){send_uart("now idle \n");}
			if(!old){
				resp = sd_send(58,0,SEND_CMD,uart);
				resp = (resp >> 30) & 0x1;
					if(resp == 0x01){
					 if(uart){send_uart("High capacity card\n");}
						} else {
					 if(uart){send_uart("Standard capacity card\n");}
						}

			}

		} else {
			if(uart){send_uart("no resp \n");}
			return 20;}
		if(uart){send_uart("Initializing complete\n");}
		return 0;
	
}


	long long sd_send(int cmd, int arg, int cmdtype, int uart){
		int cmd1;
		int cmd2 = 0x4000 | ((cmd << 8) & 0xFF00) | ((arg >> 24) & 0xFF);
		int CRC= 0xFF;
		if(cmd == 8){
			CRC = 0x87;
		}
		if(cmd == 0){
			CRC = 0x95;
		}
		if(cmdtype == SEND_CMD){
			cmd1 = CRC | ((arg << 8) & 0xFFFFFF00 );
		} else 	{cmd1 = arg;}

		
		
		long long resp = 0;
		char buffer[100];
			if(uart)
			{
			snprintf(buffer,100, "Sending cmd%d...",cmd);
			send_uart(buffer);
			}
					LED = 1;
					while(!SD_rdy);
					SD_cmd2 = cmd2; 
					SD_cmd1 = cmd1;
					SD_ctrl = cmdtype;
					LED = 0;
					while(!SD_rdy);
			resp = 	SD_cmdresp2;
			resp = ((resp << 32) & 0xFF00000000);
			resp = resp | SD_cmdresp1;
			if(uart)
			{
			snprintf(buffer,100, ".... Response: 0x%llX \n",resp);
			send_uart(buffer);
			}
		return resp;
	}

	


void send_uart(char* str){
	for(int i=0; i<strlen(str);i++){
			while(!UART_rdy);
			UART = str[i];
		}
}