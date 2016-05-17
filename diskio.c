/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
//#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */

/* Definitions of physical drive number for each drive */
#define ATA		1	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	switch (pdrv) {
	/*case ATA :
		result = ATA_disk_status();

		// translate the reslut code here

		return stat;
	*/
	case MMC :
		stat = MMC_disk_status();

		// translate the reslut code here

		return stat;
	/*
	case USB :
		result = USB_disk_status();

		// translate the reslut code here

		return stat;
	*/
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	switch (pdrv) {
	/*
	case ATA :
		result = ATA_disk_initialize();

		// translate the reslut code here

		return stat;
	*/
	case MMC :
		stat = MMC_disk_initialize();

		// translate the reslut code here

		return stat;
	/*
	case USB :
		result = USB_disk_initialize();

		// translate the reslut code here

		return stat;
		*/
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buff to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;
	switch (pdrv) {
	/*
	case ATA :
		// translate the arguments here

		result = ATA_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	*/
	case MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return RES_OK;
	/*
	case USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	
	*/
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
	switch (pdrv) {
	/*
	case ATA :
		// translate the arguments here

		result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	*/
	case MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return RES_OK;
	/*
	case USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	*/
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* buff to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	/*
	case ATA :

		// Process of the command for the ATA drive

		return res;
	*/
	case MMC :

		// Process of the command for the MMC/SD card

		return RES_OK;
	/*
	case USB :

		// Process of the command the USB drive

		return res;
	*/
	}
	
	return RES_PARERR;
}


int MMC_disk_status(){
	//Proper implementation needed
	return Stat; 
}

int MMC_disk_initialize()
{
    return init_SD(1);
}

int MMC_disk_read(BYTE* buff, DWORD sector, UINT count)
{
    unsigned long i;
    int dataword;
    for (i=0;i<count;i++)
    {
    	sd_send(17,sector,SEND_CMD,0);
    	for(int k = 0; k<4096*8;k++)
			__asm__("nop");
		}
    	for(int j=0 ; j< 128; j++){
		dataword = SD_dataQueue;
		*buff++ = (dataword >> 24);
		*buff++ = (dataword >> 16);
		*buff++ = (dataword >> 8);
		*buff++ = dataword;
        sector ++;
    }
    return 1;
}

int MMC_disk_write(const BYTE* buff, DWORD sector, UINT count)
{
    unsigned long i;
    int dataword;
    char msg[100];
    long long resp;

    for (i=0;i<count;i++)
    {
		for(int k = 0; k<4096*8;k++){
			__asm__("nop");
		}
		for(int j=0 ; j< 128; j++){
		for(int k=0; k<4; k++){
		dataword = (dataword << 8) & 0xFFFFFF00;
		dataword |= *buff;
		buff ++;
		}
		SD_dataQueue = dataword;
		snprintf(msg,100, "%08X", dataword);
		send_uart(msg);
		}
		for(int k = 0; k<4096*8*8;k++){
			__asm__("nop");
		}
		sd_send(24,sector,SEND_CMD,0);
	
        sector ++;
    }

    return 1;
}

int init_SD( int uart) {
	SD_clkdiv = (80000000/2)/400000;
	sd_send(0x0,0x74,SEND_NOPS,0);

	int old=0;
	char buff[100];
	long long resp;
	if(uart){send_uart("Initializing..\n");}
	resp = sd_send(0,0,SEND_CMD,uart);
	resp = resp & 0xFF;
		if( resp == 0x01 ){
			if(uart){send_uart("response to reset OK\n");}
			resp = sd_send(8,0x1AA,SEND_CMD,uart);

				if( (resp & 0xFF00000000) == 0x0100000000 ){
						if(uart){send_uart("cmd8 legal\n");}
						if( (resp & 0xFF) == 0xAA ){
						if(uart){send_uart("new card \n");}
						old = 0;
							if( ((resp >> 8) & 0xFF) != 0x1 ){
								if(uart){send_uart("failed voltage check\n");}
								return Stat;
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
			}}
			if(resp != 0x00){
				if(uart){send_uart(" failed");}
				return Stat;}
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
			return Stat;}
		if(uart){send_uart("Initializing complete\n");}
    	sd_send(16,512,SEND_CMD,uart);
    	SD_clkdiv = (80000000/2)/12000000;
    	Stat &= ~STA_NOINIT;
		return Stat;
	
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
		char buff[100];
			if(uart)
			{
			snprintf(buff,100, "Sending cmd%d...",cmd);
			send_uart(buff);
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
			snprintf(buff,100, ".... Response: 0x%llX \n",resp);
			send_uart(buff);
			}
		return resp;
	}

	void send_uart(char* str){
	for(int i=0; i<strlen(str);i++){
			while(!UART_rdy);
			UART = str[i];
		}
}

