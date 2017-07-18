/*---------------------------------------------------------------------------*/
/* Copyright(C)  2017  OMRON Corporation                                     */
/*                                                                           */
/* Licensed under the Apache License, Version 2.0 (the "License");           */
/* you may not use this file except in compliance with the License.          */
/* You may obtain a copy of the License at                                   */
/*                                                                           */
/*     http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                           */
/* Unless required by applicable law or agreed to in writing, software       */
/* distributed under the License is distributed on an "AS IS" BASIS,         */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and       */
/* limitations under the License.                                            */
/*---------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "uart.h"
#include "inc/stdThread.h"

#define BAUDRATE B115200
#define SERIAL_PORT "/dev/ttyACM0"  /* シリアルインターフェースに対応するデバイスファイル */
//static HANDLE hCom = INVALID_HANDLE_VALUE;
static int fd;
static struct termios oldtio, newtio;    /* シリアル通信設定 */

/* UART */
void com_close(void)
{
	ioctl(fd, TCSETS, &oldtio);       /* ポートの設定を元に戻す */
	close(fd);                        /* デバイスのクローズ */
	/*
    if ( hCom != INVALID_HANDLE_VALUE ) {
        CloseHandle(hCom);
        hCom = INVALID_HANDLE_VALUE;
    }
	*/
}

int com_init(S_STAT *stat)
{
	fd = open(SERIAL_PORT, O_RDWR);   /* デバイスをオープンする */
	if (fd < 0){
    printf("com_init err err\n");
    return FALSE;
  }
	tcflush(fd, TCIFLUSH);
	tcflush(fd, TCOFLUSH);
  bzero(&newtio, sizeof(newtio));

	ioctl(fd, TCGETS, &oldtio);       /* 現在のシリアルポートの設定を待避させる */
	newtio = oldtio;                  /* ポートの設定をコピー */
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR | ICRNL;
	newtio.c_oflag = 0;
  newtio.c_lflag = 0;

	ioctl(fd, TCSETS, &newtio);       /* ポートの設定を有効にする */

  printf(" UART init\n");
/*
    DCB dcb;
    BOOL fSuccess;
    char device[16];

    com_close();

    sprintf_s(device, 16, "\\\\.\\COM%d", stat->com_num);
    hCom = CreateFile(device,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if ( hCom == INVALID_HANDLE_VALUE ) {
        return(FALSE);
    }

    fSuccess = GetCommState(hCom,&dcb);
    if ( !fSuccess ) {
        com_close();
        return(FALSE);
    }

    dcb.BaudRate = stat->BaudRate;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutxCtsFlow = 0;
    dcb.fTXContinueOnXoff = 0;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;

    fSuccess = SetCommState(hCom,&dcb);
    if ( !fSuccess ) {
        com_close();
        return(FALSE);
    }

    fSuccess = SetupComm(hCom, 10240, 10240);
    if ( !fSuccess ) {
        com_close();
        return(FALSE);
    }
	*/
    return TRUE;
}

int com_send(unsigned char *buf, int len)
{
    DWORD dwSize = 0;
	  dwSize = write(fd, buf, len);
    //printf(" SendSize=%d\n",dwSize);
	/*
	if ( hCom != INVALID_HANDLE_VALUE ) {
        WriteFile(hCom,buf,len,&dwSize,NULL);
    }
	*/
    return (int)dwSize;
}

int com_recv(int inTimeOutTimer, unsigned char *buf, int len)
{
	
    DWORD ierr;
	//COMSTAT stat;
    DWORD dwSize = 0;

    int ret = 0;
    int totalSize = 0;
    int readSize = 0;
    //printf(" recv1\n");
  	do {
      ioctl(fd,FIONREAD,&readSize);
      if( readSize >= 1)
      {
       // printf(" readFile aru\n");
			  ret = len - totalSize;
        //printf(" ret:%d\n",ret);
			  dwSize = read(fd, &buf[totalSize], ret);
			  totalSize += (int)dwSize;
       // printf(" totalSize:%d\n",totalSize);
      }
		  if (totalSize >= len) break;
	  } while (1);
    //printf(" recv2\n");
	/*
    double finishTime = 0.0;

    LARGE_INTEGER timeFreq = {0, 0};
    LARGE_INTEGER stopTime = {0, 0};
    LARGE_INTEGER startTime = {0, 0};

    QueryPerformanceFrequency(&timeFreq);

    if ( hCom != INVALID_HANDLE_VALUE ) {
        QueryPerformanceCounter(&startTime);
        do{
            ClearCommError(hCom,&ierr,&stat);
            if ( stat.cbInQue >= 1 ) {
                ret = len - totalSize;
                if ( ret > (int)stat.cbInQue ) ret = stat.cbInQue;
                ReadFile(hCom,&buf[totalSize],ret,&dwSize,NULL);
                totalSize += (int)dwSize;
            }
            if ( totalSize >= len ) break;

            QueryPerformanceCounter(&stopTime);
            finishTime = (double)(stopTime.QuadPart - startTime.QuadPart) * 1000 / (double)timeFreq.QuadPart;
            if ( finishTime >= (double)inTimeOutTimer ) break;
        }while(1);
    }
	*/
    return totalSize;
}
