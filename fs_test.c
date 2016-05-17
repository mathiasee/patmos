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
//#include "fatfs/fat_filelib.h"
#include "libmp/mp.h"
#include <machine/uart.h>
#include "include/fatf/ff.h"
#include "include/fatf/ff.c"
#include "include/fatf/diskio.h"
#include "include/fatf/diskio.c"

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
#define SD_freq  400000//13000000//100000//100000//
#define clk_freq 80000000
FRESULT scan_files (char* path );

FATFS FatSD;		/* FatFs work area needed for each volume */

    
static
void get_line (char *buff, int len)
{
	BYTE c;
	int i = 0;

	for (;;) {
		c = getchar();
		if (c == '\r') break;
		if ((c == '\b') && i) {
			i--;
			while(!UART_rdy);
			UART = c;
			//uart_putc(c);
			continue;
		}
		if (c >= ' ' && i < len - 1) {	/* Visible chars */
			buff[i++] = c;
			while(!UART_rdy);
			UART = c;
			//xputc(c);
		}
	}
	buff[i] = 0;
	send_uart("\n");
	//xputc('\n');
}




int main() {
	FIL Filin, Filout;			/* File object needed for each open file */
	UINT bw;
	char line[80];
	char *ptr, *ptr2;
	long p1, p2, p3;
	BYTE res, b1, *bp;
	//UINT s1, s2, cnt, blen = sizeof Buff;
	DWORD ofs, sect = 0;
	BYTE buffer[4096];
	char buff[256];
	UINT br;
	//FATFS *fs;
	//RTC rtc;

 
	int i, j, l, k, hardware=1;
	char msg[100];
	k=hardware;
	if(hardware){
	for(i=0;i<400;i++)	
		for(j=0;j<400;j++)
			LED = 0;
	}
 
	 res = f_mount(&FatSD, "", 0);		/* Give a work area to the default drive */
/*
	res = disk_status(MMC);		
     snprintf(msg,100, "Status : %d\n", res);
	send_uart(msg);
*/
    // Initialise media
    //disk_initialize(MMC);
/*
    res = disk_status(MMC);		
     snprintf(msg,100, "Status : %d\n", res);
	send_uart(msg);
   */  
    
 
   
 	//snprintf(msg,100, "Mount response: %d\n", res);
	//send_uart(msg);
  
	//res |= f_mkfs("0:", 1, 1);
   	//disk_read(MMC, FatSD.win, 0, 1);
   	//FatSD.winsect = 1;
   	//res = move_window(&FatSD, 0);

        
   	      
	//snprintf(msg,100, "mkfs response: %d\n", res);
	//send_uart(msg);
    /*         	
	res = disk_status(MMC);		
     snprintf(msg,100, "Status : %d\n", res);
	send_uart(msg);
*/
	//res = f_open(&Filout, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
	//res = f_open(&Filin, "test.txt", FA_READ);
	//snprintf(msg,100, "Open response: %d\n", res);
	//send_uart(msg);

    
	/*disk_read(MMC,Buff,0,1);
		for(i = 0; i<512; i++){
			snprintf(msg,100, "%02X", Buff[i]);
			send_uart(msg);
		}
	*/ 
	//if ( res == FR_OK) {	/* Create a file */
	//		send_uart("open file");
			
			//res = f_write(&Filout, "It works!\r\n", 11, &bw);	/* Write data to the file */
			//snprintf(msg,100, "Write response: %d\n", res);
			//res = f_close(&Filout);
			
			//send_uart(msg);
			
			//snprintf(msg,100, "F Close response: %d\n", res);
			//send_uart(msg);

	//	if (bw == 11) {		/* Lights green LED if data written well */
	//		LED = 1;	/* Set PB4 high */
	//	}
	//}  

	
	//res = disk_status(MMC);		
    // snprintf(msg,100, "\nStatus : %d\n", res);
	//send_uart(msg);
	//while(1);
	/*
	disk_ioctl(1, GET_SECTOR_COUNT, &p2);
	snprintf(msg,100, "Drive size: %lu sectors\n", p2);
	send_uart(msg);

	res = f_getlabel(ptr2, (char*)Buff, (DWORD*)&p1);
				//if (res) { put_rc(res); break; }
				Buff[0] ? snprintf(msg,100, "Volume name is %s\n", Buff) : snprintf(msg,100, "No volume label\n");
				send_uart(msg);
				//xprintf(Buff[0] ? PSTR("Volume name is %s\n") : PSTR("No volume label\n"), Buff);
				//xprintf(PSTR("Volume S/N is %04X-%04X\n"), (WORD)((DWORD)p1 >> 16), (WORD)(p1 & 0xFFFF));*/  
  
	while(1){
	send_uart("\nWelocme to FatF for Patmos\n");
	send_uart("Options are:\n");
	send_uart("O [r/w] [filename] [content if writemode]\n");
	send_uart("l for file and dir structure\n");
	send_uart("Opening a file in readmode will print the content to terminal\n");
	send_uart("Write mode is not implemented\n");
	send_uart("use ^M (ctrl-V) + enter to terminate input\n");
	
	ptr = line;
	get_line(ptr, sizeof line);
	switch (*ptr++) {

		case 'T' :
			while (*ptr == ' ') ptr++;

			/* Quick test space */

			break;
		case 'O' :
		case 'o' :
			while (*ptr == ' ') ptr++;
			switch(*ptr++){
				case 'R' :
				case 'r' :
					while (*ptr == ' ') ptr++;
					res = f_open(&Filin, ptr, FA_READ);
					res = f_read(&Filin, buffer, sizeof(buffer), &br);	/* read data from the file */	
					res = f_close(&Filin);								/* Close the file */
					send_uart("\nThis is the content of the file: \n\n");
					for(i=0; i < br; i++){
   					snprintf(msg,100, "%c", buffer[i]);
					send_uart(msg);
					}
					send_uart("\n\n"); 
					break;

				case 'W' : 
				case 'w' : 
					//ptr = line;
					//send_uart("Enter filename:\n");
					//get_line(ptr, sizeof line);
					res = f_open(&Filout, "newtest.txt", FA_WRITE | FA_CREATE_ALWAYS);
					snprintf(msg,100, "\nOpen response : %d\n", res);
					send_uart(msg);
					//res = f_open(&Filout, ptr, FA_WRITE | FA_CREATE_ALWAYS);
					//res = f_open(&Filin, ptr, FA_READ);
					//&Filout, "It works!\r\n", 11, &bw)
					//ptr = line;
					//send_uart("Enter content:\n" 	);
					//get_line(ptr, sizeof line);
					res = f_write(&Filout, "It works!\r\n", 11, &bw);
					snprintf(msg,100, "\nWrite response : %d\n", res);
					send_uart(msg);
					//res = f_write(&Filout, ptr, sizeof(line), &br);	/* read data from the file */	
					res = f_close(&Filout);								/* Close the file */

			}
			break;
		case 'l' :
		case 'L' :
			strcpy(buff, "/");
        	res = scan_files(buff);
        	break;
		}

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

	



FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                sprintf(&path[i = strlen(path)], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                send_uart(path);
                send_uart("/");
                send_uart(fno.fname);
                send_uart("\n");
                //printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}
	






