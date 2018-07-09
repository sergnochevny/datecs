// datecsfp.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

//#define STRICT 1
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "port.h"
#include "fp.h"
#include "winerror.h" 

#include <assert.h>
//#define TESTING

enum RcvStates { START,LEN,SEQ,CMD,DATA,SEP,STAT,EOT,CHKSUM,CTRC };

void RS01(), RSlen(), RSseq(), RScmd(), RSdata();
void RS04(), RSstatus(), RS05(), RSbcc(), RS03();
void GetResponse(void);


struct Answer
	{
	unsigned char Status[6];
	unsigned char Seq;
	unsigned char Cmd;
	char Data[300];
	unsigned char ReadyFlag;
	unsigned char RecError;
	unsigned char Len;
	unsigned CalcSum;
	unsigned CheckSum;
	};

static HANDLE RSportId;

RcvStates RcvState=START;
Answer  far ans;
char WholeRet[200];
struct RetData rtd;
LPSTR rdata=(LPSTR)WholeRet;
unsigned char Counter,CurCh,Seq=0,LastSeq=0;
unsigned long StatusFlag;

static bool WinDos=true;
//static bool WinDos=false;

static unsigned char table[256]=
{
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,

0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0x85, 0xA9, 0xC0, 0xAB, 0xAC, 0xAD, 0xAE, 0xC2,
0xB0, 0xB1, 0x49, 0x69, 0xB4, 0xB5, 0xB6, 0xB7, 0xA5, 0xB9, 0xC1, 0xBB, 0xBC, 0xBD, 0xBE, 0xC3,
0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF
};

static unsigned char table1[256]=
{
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,

0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0xAB, 0xAC, 0xAD, 0xAE, 0xC2,
0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
0xAA, 0xBA, 0xAF, 0xBF, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

DCB RSport;




//Windows95 5.10.98
COMMTIMEOUTS timeouts;
DWORD dwEvtMask;
//


char ver[100]="Ver. 2.0 28.02.06";

UINT TimerId,rdCount,ResponseMsg=0x1099;
char LastStr[300];
//HWND RespWnd;

//Начало - 04.12.2000
//int  Ticks=0,LastCmd=0,Decimals=0,RetryCount=3;
int  Ticks=0,LastCmd=0,Decimals=2,RetryCount=3;  //последнее изменение 04.12.2000
//учитывает наличие копеек в гривнях
//Конец - 04.12.2000


int ShowAmount=1; //последнее изменение 17.02.2003

BOOL ReturnMode=TRUE;
int DebugCnt=0;

void (* RSfun[10])()= { *RS01,*RSlen,*RSseq,*RScmd,*RSdata,*RS04,*RSstatus,*RS05,*RSbcc,*RS03 };
//                       START, LEN,   SEQ,   CMD,   DATA,   SEP,   STAT,    EOT, CHKSUM,CTRC 
LPSTR NotValidCmd= "Неверная команда";
LPSTR CmdNames[]= {
		 "Очистка дисплея",                                 	// 33
		 "",
		 "Дисплей - нижний ряд",                              // 35
     "","",
		 "Открыть нефискальный чек",                          // 38
		 "Закрыть нефискальный чек",                          // 39
		 "","",
		 "Печать нефискального текста",                 		// 42
		 "Установить Header, Footer и опции печати",       	// 43
		 "Пропустить строки на принтере",                     // 44
		 "Отрезать чек",													// 45
       "",
		 "Дисплей - верхний ряд",                             // 47
		 "Открыть фискальный чек",                            // 48
       "",
//		 "Регистрация продажи",                               // 49   ******
		 "Печать налоговых ставок за период",                 // 50   ******
		 "Подсумма",                                          // 51
		 "Регистрация продажи  и вывод на дисплей",           // 52
		 "Общая сумма чека (Total)",                          // 53
		 "Печать строк в фискальном чеке",                   	// 54
       "",																  	// 55
//		 "Печать информации о текущем фиск. чеке",  				// 55   ******
		 "Закрыть фискальный чек",                            // 56
		 "Аннулировать чек",												// 57
       "Регистрация продажи",                               // 58
       "","",
		 "Установить дату и время",                           // 61
		 "Получить дату и время",                           	// 62
		 "Вывести дату и время на дисплее",                   // 63
		 "Информация о последнем Z-отчете",           			// 64
		 "Суммы за день",                      					// 65   ******
		 //"Просмотреть фискальную память",                   // 66
       "",
		 "Суммы коррекций за день",                     		// 67
		 "Размер фискальной памяти",                          // 68   ******
		 "X- или Z- отчет",                                   // 69   ******
		 "Служебный внос/вынос суммы",                        // 70
		 "Печать диагностической информации",                 // 71
		 "Фискализация",                                      // 72
		 "Полный периодический отчет по номеру", 					// 73
		 "Получить состояние регистратора",                   // 74
		 "",                              							// 75
		 "Состояние фискальной транзакции",    					// 76
		 "",                            								// 77
		 "",
		 "Сокращенный периодический отчет (по дате)",         // 79
		 "Звуковой сигнал",       										// 80
       "","",
		 "Установить множитель, запятую, валюту и количество групп налогов",  // 83
		 "Режим продаж",            									// 84
		 "Открыть чек возврата",										// 85
       "",
		 "",            													// 87
		 "",               												// 88
		 "Программирование тестовой области",         			// 89
		 "Запрос диагностической информации",                 // 90  ******
		 "Установить заводской номер", 								// 91
		 "Установить фискальный номер", 								// 92
       "",
		 "Полный периодический отчет по дате",  					// 94
		 "Сокращенный периодический отчет по номеру",    		// 95  ******
		 "",                      										// 96
		 "Ставки налогов",                         				// 97
		 "Установить налоговый или иденификационный номер",   // 98
		 "Прочитать налоговый или иденификационный номер",    // 99
		 "Управление дисплеем",                               // 100      ******
		 "Задать пароль оператора",									// 101
       "Задать имя оператора",                              // 102
       "Информация о текущем чеке",                         // 103
       "Обнулить данные оператора",                         // 104
       "Отчет оператора",												// 105
       "Открыть денежный ящик",										// 106
       "Программирование артикулов и получение информации о них", //107
       "",
       "Печать копии чека",											// 109
		 "Дополнительная информация по типам оплаты",         // 110
		 "Отчет товаров" , 										      // 111
		 "Информация об операторе" ,									// 112
		 "Номер последнего чека" , 									// 113
		 "Получение информации из фискальной памяти" ,			// 114
		 "Загрузка логотипа"  ,											// 115
       "","",
		 "Пароль администратора" , 									// 118
		 "Обнулить операторские пароли" };						   // 119

/////////////////////////////////////////////////////////////////////////////
// comlog - log file for testing
void comlog(int cmd, char *infodata, LPARAM Status)
{


  FILE *stream1;
  stream1 = fopen("fpllog.dat", "at+");

      fputs(" \n",stream1);
      char  dat[200]="Код команды:";
      char  cmdx[20];
      _itoa(cmd,cmdx,10);
      strcat(dat,cmdx);
      fputs(dat,stream1);

      strcpy(dat,"Данные: ");
      strcat(dat,infodata);
      fputs(" \n",stream1);
      fputs(dat,stream1);

      strcpy(dat,"Статус: ");
      _ltoa(cmd,cmdx,16);
      strcat(dat,cmdx);
      fputs(" \n",stream1);
      fputs(dat,stream1);


  fclose(stream1);
}


BOOL GetInBuff( void )
{
#ifndef DUMMY

   int dwLength;
   LPSTR szStore;
//   BYTE szStore[ RXQUEUE ];
   BOOL pContinue, pWait;
	int vStab = DEF_STAB_PROT;
	int i = 0;
	static int SYNcnt;
  
	pContinue = TRUE;
	pWait = FALSE;
	
	szStore = (LPSTR) malloc(RXQUEUE);
	memset( (LPSTR) szStore, 0x00, RXQUEUE ) ;
	do
   {
      if ( dwLength = ReadCommBlock( (LPSTR) szStore, MAXBLOCK, pWait ) )
      {
         if ((dwLength > 0) && (szStore[dwLength - 1] == SYN)) pWait = TRUE;
         else {
			   while ( i < dwLength ){
				   CurCh=szStore[i++];
				   SYNcnt=0;
				   if (CurCh<0x20) 
					   switch (CurCh)
					   {
						   case PREAMBLE:
							   RcvState=START; RSfun[RcvState](); 
							   pWait = TRUE;
							   break;
						   case NACK:
							   RcvState=START; ans.RecError=4;
							   ans.ReadyFlag=1; 
							   free( szStore );
							   return ( FALSE );
							   break;
						   case TERMINATOR:
							   if (RcvState!=CTRC) ans.RecError=8;
							   RcvState=CTRC; RSfun[RcvState](); 
							   pWait = FALSE;
							   break;
						   default:
							   SYNcnt=0;
							   RSfun[RcvState]();
					   }
				   else
					   RSfun[RcvState]();
			   }
			   if ( ans.ReadyFlag == 1 ){
				   free( szStore );
				   return ( TRUE );
			   }
         }
		}
		else 
			if ( !vStab-- ) pContinue = FALSE;

		dwLength = 0;
		memset( (LPSTR) szStore, 0x00, RXQUEUE ) ;
      if ( !pContinue ) break;
	}
	while ( TRUE );
	
	free( szStore );
	return ( FALSE );
#else

	return ( TRUE );

#endif
}

//Windows98 5.10.98
HANDLE CALLBACK  GetRSId(void)
//int   CALLBACK GetRSId(void);
	{

  //Windows95 5.10.98
  return RSportId;
//  return RSport.Id;
  }

void RS01()                        // Preamble
	{
	if (CurCh==PREAMBLE) RcvState=LEN;
	ans.CalcSum=0;
	}

void RSlen()                       // Data length
	{
	if (CurCh>=0x2b)
		{ ans.Len=(unsigned char)(CurCh-0x2b); RcvState=SEQ; ans.CalcSum+=CurCh; }
	else
		RcvState=START;
	}

void RSseq()                       // Seq. block number
	{
	if (CurCh>=0x20)
		{ ans.Seq=(unsigned char)(CurCh-0x20); RcvState=CMD; ans.CalcSum+=CurCh; }
	else
		RcvState=START;
	}

void RScmd()                       // Command
	{
	ans.Cmd=CurCh; Counter=0; ans.CalcSum+=CurCh; *(ans.Data)='\0';
	if (ans.Len==0) RcvState=SEP; else RcvState=DATA;
	}

void RSdata()                      // Data bytes
	{
	if (Counter<ans.Len)
		{	*(ans.Data+Counter)=CurCh; Counter++; ans.CalcSum+=CurCh; }
	if (Counter==ans.Len)
		{ *(ans.Data+Counter)='\0'; RcvState=SEP; }
	}

void RS04()                        // Separator
	{
	if (CurCh==4)
		{ RcvState=STAT; Counter=0; ans.CalcSum+=CurCh; }
	else
		RcvState=START;
	}

void RSstatus()                    // Status bytes
	{
	if (Counter<6)
		{ *(ans.Status+Counter)=CurCh; Counter++; ans.CalcSum+=CurCh; }
	if (Counter==6)
		RcvState=EOT;
	}

void RS05()                        // Postamble
	{
	if (CurCh==POSTAMBLE)
		{ RcvState=CHKSUM; Counter=0; ans.CheckSum=0; ans.CalcSum+=CurCh; }
	else
		RcvState=START;
	}

void RSbcc()                       // CheckSum
	{
	if (Counter<4)
		{ ans.CheckSum=(ans.CheckSum<<4)+CurCh-0x30; Counter++; }
	if (Counter==4)
		RcvState=CTRC;
	}

void RS03()                       // Terminator
	{
	RcvState=START;
	if (CurCh==TERMINATOR) ans.ReadyFlag=1;
	}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
   switch (ul_reason_for_call)
   {
      case(DLL_PROCESS_ATTACH):
         break;
   }

   return TRUE;
}

LPSTR CALLBACK CmdDescription(int Cmd)
	{
	if ((Cmd>=33)&&(Cmd<120)&&(CmdNames[Cmd-33][0]!='\0'))
		return CmdNames[Cmd-33];
	else
		return NotValidCmd;
  }



int CALLBACK  InitFPport(int PortNo, int Bps, int seq)
{
	Seq = seq;
#ifndef LOGGED

   if ( CreateTTYInfo( PortNo, Bps ) )
   {
      if ( OpenConnection() )
      {
			ans.ReadyFlag=1;

			return ( 1 );
		}
	}

	return ( 0 );

#else
	char Cmd[32];

	sprintf(Cmd,"prot_COM%d.dat",PortNo);

   RSportId=CreateFile( Cmd, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
	return 1;
#endif
}

int CALLBACK  CloseFPport(void)
{
	if ( DestroyTTYInfo( ) )
		return 1;
	else 
		return 0;
}

RetData* CALLBACK SendCmdAll(int Cmd,LPSTR Data)
	{
//	if (hwnd==NULL) return 
		SendCmdCbk(Cmd,Data);
//	else return SendCmdMsg(hwnd,Cmd,Data);
      return &rtd;
	}

int CALLBACK SendCmdMsg(int Cmd, LPSTR Data)
{
	unsigned char Len,i,Ch;
	unsigned CheckSum;
   unsigned int yloop;
	char MsgString[200],OemData[200];
   unsigned int len;
	unsigned int iloop;
	int nack_loop = 0;
	int reconnect_loop = 0;
	
#ifndef LOGGED
	do 
	{
#ifndef TESTING
#ifndef DUMMY
		if (!ans.ReadyFlag) return (-1);
#endif
#endif
#endif
		memset(LastStr,0x00,300);
		strcpy(LastStr,Data); LastCmd=Cmd; LastSeq=Seq;
		rtd.CmdCode=Cmd;
		Seq++;
#ifdef TESTING
		char TestStr[80];
	  StatusFlag=0L;
		ans.Status[0]=0xC0; ans.Status[1]=0xC0; ans.Status[2]=0xC0;
		ans.Status[3]=0xC0; ans.Status[4]=0xC0; ans.Status[5]=0xBA;
	  ans.RecError=0; ans.ReadyFlag=1;
		switch (Cmd)
			{
			case 68:
				strcpy(ans.Data,"1200,1200");
				break;
			case 74:
				strncpy(ans.Data,ans.Status,6);
				break;
			case 90:
				strcpy(ans.Data,"1.10 02Feb98 1200,FFFF,00000000,6,DAT 12345678");
				break;
			case 83:
				strcpy(ans.Data,"0,2,ГРН ,2");
				break;
			case 97:
				strcpy(ans.Data,"22,00,00,00");
				break;
			case 99:
				strcpy(ans.Data,"DAT 12345678901234");
			break;
			case 62:
			strcpy(ans.Data,"01-01-98 11:11:11");
				break;
			case 110:
				strcpy(ans.Data,"1050,2000,0,0,8,2");
				break;
			case 67:
				strcpy(ans.Data,"0,100,2000,2,2");
				break;
			case 70:
				strcpy(ans.Data,"P,4050,3000,0");
				break;
			case 65:
				strcpy(ans.Data,"0,3050,0,0,0");
				break;
			default:
			ans.ReadyFlag=0;
				ans.Data[0]='\0';
			}
	  RespWnd=hwnd; rdCount=0;
	  ans.Len=strlen(ans.Data)+4;
		ans.CalcSum=0; ans.CheckSum=0;
		ans.Cmd=Cmd; ans.Seq=Seq;
		GetResponse(); ans.ReadyFlag=1;
		if (((unsigned long)RetFun)<=0xffff)
			PostMessage(RespWnd,ResponseMsg,(WPARAM)LastCmd,(LPARAM)(&rtd));
		else
		  {
			((RetCallback)(RetFun)) (rtd);
		  ReturnMode=TRUE;
			}
		return 0;
#else

		CheckSum=0; StatusFlag=0L; rdCount=0;
		memset(MsgString, 0x00, 200);
		memset(OemData, 0x00, 200);
		for (i=0; i<6; i++) ans.Status[i]=0;
		len=strlen(Data);
		for(iloop=0; iloop<=len;++iloop)
		{
		yloop=(unsigned int)(Data[iloop]) & 0xFF;
		if(!WinDos)OemData[iloop]=table[yloop];
		 else OemData[iloop]=yloop;
		}
		ans.ReadyFlag=0; ans.RecError=0; ans.Data[0]='\0';
		Len=(unsigned char)(strlen(Data)+4);
		sprintf(MsgString,"\001%c%c%c%s\0050000\003",Len+0x20,(Seq%0x5F)+0x20,Cmd,OemData);
		for (i=0; i<Len; i++)
			{ Ch=*(MsgString+i+1); CheckSum+=Ch; }
		for (i=0; i<4; i++)
			{ MsgString[Len+4-i]=(char)(CheckSum%16+0x30); CheckSum/=16; }
		Len=(unsigned char)(strlen(MsgString));

	//Windows95 5.10.98
	//   FlushFileBuffers(RSportId);

		DebugCnt=0;
#ifndef LOGGED
		reconnect_loop = 0;
		do {
			if ( WriteCommBlock( MsgString, Len ) )
			{
				reconnect_loop++;
				if ( GetInBuff() )
				{
#ifndef DUMMY
					GetResponse();

					if ( ans.ReadyFlag == 1 ) return (1);
#else
					return (1);
#endif
				}
				else 
					if (!(( ans.ReadyFlag == 1 ) && ( (ans.RecError & 4) == 4 ))) 
						::RaiseException(0x123456, 0, 0, NULL);
			}
			else {
				if ( reconnect_loop < 1 ) {
					if ( !ReOpenConnection() )
						::RaiseException(0x123456, 0, 0, NULL);
				} 
				else
					::RaiseException(0x123456, 0, 0, NULL);
			}
		}
		while (reconnect_loop++ < 1 );
	}
	while ( nack_loop++ < 2 );
#else
		WriteCommBlock( MsgString, Len );
		return (1);
#endif

#ifdef _COMLOG
			comlog(Cmd, Data, rtd.Status);
#endif

		return (0);
#endif
}

int CALLBACK  SendCmdCbk(int Cmd, LPSTR Data)
	{
//	RetFun=(void (PASCAL FAR *)(RetData far &))Fn;
	return SendCmdMsg(Cmd,Data);
  }

int CALLBACK  SendLast(BOOL Increment)
	{
	if (!ans.ReadyFlag) return (-1);
	if (!Increment) Seq=LastSeq;
	return SendCmdMsg(LastCmd,LastStr);
  }

void GetResponse(void)
	{
	unsigned char StatByte;
	int i;
	StatusFlag = 0;
	if (ans.ReadyFlag)
    {
		if (ans.CheckSum!=ans.CalcSum) ans.RecError|=1;
		if (ans.Seq!=Seq) ans.RecError|=2;
		}
	for (i=0; i<6; i++) rtd.OrigStat[i]=(unsigned char)(ans.Status[i]&0x7f);
	if (!ans.ReadyFlag) StatusFlag|=TIMEOUT_ERROR;
	else if (ans.RecError&4) StatusFlag|=NACK_RECEIVED;
	else if (ans.RecError&1) StatusFlag|=PROTOCOL_ERROR; // !!????
	else
		{
		StatByte=ans.Status[0];
		if (StatByte)
			{
			if (StatByte&0x01) StatusFlag|=SYNTAX_ERROR;
			if (StatByte&0x02) StatusFlag|=INVALID_CMD;
			if (StatByte&0x04) StatusFlag|=INVALID_TIME;
			if (StatByte&0x10) StatusFlag|=PRINT_ERROR;
			if (StatByte&0x20) StatusFlag|=COMMON_ERROR;
			}
		StatByte=ans.Status[1];
		if (StatByte)
			{
			if (StatByte&0x01) StatusFlag|=SUM_OVERFLOW;
			if (StatByte&0x02) StatusFlag|=CMD_NOT_ALLOWED;
			if (StatByte&0x04) StatusFlag|=RAM_CLEARED;
			if (StatByte&0x08) StatusFlag|=PRINT_RESTART;
			if (StatByte&0x10) StatusFlag|=RAM_DESTROYED;
			if (StatByte&0x20) StatusFlag|=SERVICE_OPEN;
			}
		StatByte=ans.Status[2];
		if (StatByte)
			{
			if (StatByte&0x01) StatusFlag|=PAPER_OUT;
			if (StatByte&0x08) StatusFlag|=FISCAL_OPEN;
			if (StatByte&0x20) StatusFlag|=NONFISCAL_OPEN;
			}
		StatByte=ans.Status[4];
		if (StatByte)
			{
			if (StatByte&0x01) StatusFlag|=F_WRITE_ERROR;
			if (StatByte&0x04) StatusFlag|=F_ABSENT;
			if (StatByte&0x08) StatusFlag|=F_LESS_30;
			if (StatByte&0x10) StatusFlag|=F_FULL;
			if (StatByte&0x20) StatusFlag|=F_COMMON_ERROR;
			}
		StatByte=ans.Status[5];
		if (StatByte)
			{
			if (StatByte&0x01) StatusFlag|=F_READ_ONLY;
			if (StatByte&0x02) StatusFlag|=F_FORMATTED;
			if (StatByte&0x04) StatusFlag|=F_CLOSE_ERROR;
			if (StatByte&0x08) StatusFlag|=F_FISCALIZED;
			if (StatByte&0x10) StatusFlag|=F_SER_NUM;
			if (StatByte&0x20) StatusFlag|=F_MODULE_NUM;
			}
		}

   unsigned int len=strlen(ans.Data);
   unsigned int yloop;
   for(unsigned int iloop=0; iloop<=len;++iloop)
   {
   yloop=(unsigned int)(ans.Data[iloop]) & 0xFF;
//   ans.Data[iloop]=table1[yloop];
//   if(!WinDos)ans.Data[iloop]=table1[yloop];
   if(!WinDos)ans.Data[iloop]=table1[yloop];
    else ans.Data[iloop]=yloop;
   }
   //	OemToAnsi(ans.Data,ans.Data);

	_fmemccpy((LPSTR)rdata,(LPSTR)ans.Data,'\0',128);
	if (strlen(ans.Data)>1)
		{
		switch (LastCmd)
			{
      case 50:; case 53:
				for (i=strlen(ans.Data); i>=1; i--) *(ans.Data+i+1)=*(ans.Data+i);
				*(ans.Data+1)=','; break;
			case 62:
				*(ans.Data+8)=','; break;
			case 90:
				for (i=0; i<20; i++)
					if (*(ans.Data+i)==' ') *(ans.Data+i)=',';
        break;
			}
		}
	rtd.Status=StatusFlag;
	rtd.CmdName=CmdDescription(LastCmd);
	rtd.SendStr=LastStr;
	rtd.Whole=rdata;
	if (ans.Data[0]=='\0')
		rdCount=0;
	else
		{
    rtd.RetItem[0]=(LPSTR)ans.Data;
		for (i=0; ans.Data[i]; i++) if (ans.Data[i]==',')
			{
      rdCount++; ans.Data[i]='\0';
			rtd.RetItem[rdCount]=(LPSTR)(ans.Data+i+1);
			}
		rdCount++;
		}
	rtd.Count=rdCount;
	}

void CALLBACK  SetMessageNum(UINT Message)
	{
	ResponseMsg=Message;
	}

int CALLBACK  SetDecimals(int DecCount)
	{
  if ((DecCount>3)||(DecCount<0)) return (-1);
	Decimals=DecCount;
	return (0);
  }

int CALLBACK  SetShowAmount(int Show)
	{
  if ((Show!=1)&&(Show!=0)) return (-1);
	ShowAmount=Show;
	return (0);
  }

RetData* CALLBACK  OpenNonfiscalReceipt(void)
	{
	return SendCmdAll(38,"");
	}

RetData* CALLBACK  CloseNonfiscalReceipt(void)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
   strcpy(Tmp,"");
//	strcpy(Tmp,"00");
//	if (Number) Tmp[0]='1';
//	if (Time)   Tmp[1]='1';
	return SendCmdAll(39,Tmp);
}

RetData* CALLBACK  PrintNonfiscalText(LPSTR Text)
{
//   unsigned int len=strlen(Text);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Text[iloop]=table[Text[iloop]];
	return SendCmdAll(42,Text);
}

RetData* CALLBACK  SetHeaderFooter(int Line,LPSTR Text)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
   if(Line >= 0 && Line <= 7) 
      Tmp[0]=(char)(Line+0x30);
      else {
         if(Line == 'C' || Line == 'L' || Line == 'J' || Line == 'I')
            Tmp[0]=(unsigned char)(Line);
      }
//   unsigned int len=strlen(Text);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Text[iloop]=table[Text[iloop]];
	strcpy((Tmp+1),Text);
	return SendCmdAll(43,Tmp);
}


RetData* CALLBACK  AdvancePaper(int Lines)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%d",Lines);
	return SendCmdAll(44,Tmp);
}

RetData* CALLBACK  DisplayTextLL(LPSTR Text)
{
//   unsigned int len=strlen(Text);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Text[iloop]=table[Text[iloop]];
	return SendCmdAll(35,Text);
}

RetData* CALLBACK  OpenFiscalReceipt(
	 DWORD Operator=1,	LPSTR Text="0000", DWORD TillNumber=1, BOOL TaxCheck=true)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%05lu,%s,%05lu",Operator, Text, TillNumber);
//   if(TaxCheck)strcat(Tmp,",I");

   strcat(Tmp,",I");

//	sprintf(Tmp,"00001,0000,00000,I");

	return SendCmdAll(48,Tmp);
}

RetData* CALLBACK  OpenRepaymentReceipt(
	 DWORD Operator=1,	LPSTR Text="0000", DWORD TillNumber=1, BOOL TaxCheck=true)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%05lu,%s,%05lu",Operator, Text, TillNumber);
//   if(TaxCheck)strcat(Tmp,",I");
   strcat(Tmp,",I");
//	sprintf(Tmp,"00001,0000,00000,I");

	return SendCmdAll(85,Tmp);
}



RetData* CALLBACK  GetTaxRates(LPSTR psw, LPSTR Start,LPSTR End)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%s,%s,%s",psw,Start,End);
	return SendCmdAll(50,Tmp);
 }

RetData* CALLBACK SaleArticle( bool sign, int numart, double qwant, double perc, double  dc)
{
	double iqwant = qwant;
//if(perc != 0. && dc != 0.) return -1;
//if(iqwant < 0.) return -1;
//if(perc < -99.99 && perc > 99.99) return -1;

char FprmStr[200];

if(sign)FprmStr[0]=0;
  else strcpy(FprmStr,"-");

char art1[30];
_itoa(numart,art1,10);
strcat(FprmStr,art1);

//if(iqwant > 0)
//	{
	 strcat(FprmStr,"*");
    char qw[30];
    sprintf(qw,"%-#.3lf",iqwant);
    strcat(FprmStr,qw);
//   }

if(perc != 0)
	{
	 strcat(FprmStr,",");
    char prc[30];
    sprintf(prc,"%-#.2lf",perc);
    strcat(FprmStr,prc);
   }

if(dc != 0)
	{
	 strcat(FprmStr,";");
    char dcstr[30];
    sprintf(dcstr,"%-#.2lf",dc);
    strcat(FprmStr,dcstr);
   }

	return SendCmdAll(58,FprmStr);

}

RetData* CALLBACK SaleArticleAndDisplay(bool sign, int numart, double qwant, double perc, double  dc)
{
	double iqwant = qwant;
//if(perc != 0. && dc != 0.)return -1;
//if(iqwant < 0.)return -1;
//if(perc < -99.99 && perc > 99.99)return -1;

char FprmStr[200];

if(sign)FprmStr[0]=0;
  else strcpy(FprmStr,"-");

char art1[30];
_itoa(numart,art1,10);
strcat(FprmStr,art1);

//if(iqwant > 0)
//	{
	 strcat(FprmStr,"*");
    char qw[30];
    sprintf(qw,"%-#.3lf",iqwant);
    strcat(FprmStr,qw);
//   }

if(perc != 0)
	{
	 strcat(FprmStr,",");
    char prc[30];
    sprintf(prc,"%-#.2lf",perc);
    strcat(FprmStr,prc);
   }

if(dc != 0)
	{
	 strcat(FprmStr,";");
    char dcstr[30];
    sprintf(dcstr,"%-#.2lf",dc);
    strcat(FprmStr,dcstr);
   }

	return SendCmdAll(52,FprmStr);

}

RetData* CALLBACK  GetLastTaxRates( LPSTR psw)
{

//	 sprintf(Tmp,"%s,%s",Start,End);
	 return SendCmdAll(50,psw);
}



RetData* CALLBACK  SetTaxNumber( LPSTR tn, int type)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
    if(type == 1)
    strcpy(Tmp,"%s,1");
    else{
		if(type == 0)
			strcpy(Tmp,"%s,0");
    }
	 return SendCmdAll(98,Tmp);
	 }


RetData* CALLBACK  SubTotal(BOOL Print,BOOL Display, double Percent=0.0, double DS=0.0)
{

	char FormatStr[200];
	char TmpTT[200];
   char Tmp[300];


//<Print><Display>[,<[Sign]Percent>]
   _strset_s(FormatStr, 200, 0);
   _strset_s(TmpTT, 200, 0);
   _strset_s(Tmp, 300, 0);
	strcpy(Tmp,"00");
	if (Print)   Tmp[0]='1';
	if (Display) Tmp[1]='1';

//if(Percent != 0.0 && DS != 0.0)return -1;

   if(Percent != 0.0 && DS == 0.0)
   {
			strcpy(FormatStr,"%s,%-#.2lf");
   		sprintf(TmpTT,FormatStr,Tmp,Percent);
   }

   if(Percent == 0.0 && DS != 0.0)
   {
			strcpy(FormatStr,"%s;%-#.2lf");
   		sprintf(TmpTT,FormatStr,Tmp,DS);
   }

   if(Percent == 0.0 && DS == 0.0)
   {
			strcpy(FormatStr,"%s");
		   sprintf(TmpTT,FormatStr,Tmp);
   }


	return SendCmdAll(51,TmpTT);


/*
   if(Percent != 0.0)
   {
   sprintf(TmpTT,FormatStr,Tmp,Percent);
	return SendCmdAll(hwnd,Fn,51,TmpTT);
   }
   else
	return SendCmdAll(hwnd,Fn,51,Tmp);
*/

	}




RetData* CALLBACK  Total(
		LPSTR Comment,char PaidCode,double Amount)
	{
//[<Line1>][<Lf><Line2>]<Tab>[<PaidMode>]<[Sign]Amount>
//   unsigned int len=strlen(Comment);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Comment[iloop]=table[Comment[iloop]];
   char Tmp[300];
	char TmpFloat[200];

   _strset_s(Tmp, 300, 0);
   _strset_s(TmpFloat, 200, 0);
	strcpy(TmpFloat,"%s\tP%-#.2lf");
  if (PaidCode=='\0') TmpFloat[3]='P';
	else TmpFloat[3]=PaidCode;
//	TmpFloat[strlen(TmpFloat)-3]=Decimals+0x30;
  sprintf(Tmp,TmpFloat,Comment,Amount);
	return SendCmdAll(53,Tmp);
	}

RetData* CALLBACK  PrintFiscalText(LPSTR Text)
	{
//   unsigned int len=strlen(Text);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Text[iloop]=table[Text[iloop]];
	return SendCmdAll(54,Text);
	}


RetData* CALLBACK  CloseFiscalReceipt(void)
	{
	return SendCmdAll(56,"");
	}

RetData* CALLBACK ResetReceipt(void)
	{
	return SendCmdAll(57,"");
	}

RetData* CALLBACK  SetDateTime(LPSTR Date,LPSTR Time)
{
	char FormatStr[200];
   char StrTmp[300];
   
   _strset_s(FormatStr,200,0);
   _strset_s(StrTmp,00,0);
   strcpy(FormatStr, "%s %s");
   sprintf(StrTmp,FormatStr,Date,Time);
	return SendCmdAll(61,StrTmp);
}

RetData* CALLBACK  GetDateTime(void)
{
	return SendCmdAll(62,"");
}

RetData* CALLBACK  LastFiscalClosure(int option)
{
   if(option == 1)
	return SendCmdAll(64,"1");
   else
   {
     if(option == 0)return SendCmdAll(64,"0");
	  else return NULL;
    }
}

        

RetData* CALLBACK  GetCurrentTaxes( int  option)
{
    switch (option)
     {
      case 0: return SendCmdAll(65,"0");
      case 1: return SendCmdAll(65,"1");
      case 2: return SendCmdAll(65,"2");
      case 3: return SendCmdAll(65,"3");
      default: return NULL;
     }

}

RetData* CALLBACK FiscalMemoryLookup(int Closure)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%04d",Closure);
	return SendCmdAll(66,Tmp);
}

RetData* CALLBACK GetCurrentSums(void)
	{
	return SendCmdAll(67,"");
	}

RetData* CALLBACK  GetFreeClosures(void)
	{
	return SendCmdAll(68,"");
	}

RetData* CALLBACK  FiscalClosure( LPSTR psw, char Option)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%s,%c",psw,Option);
	return SendCmdAll(69,Tmp);
}

RetData* CALLBACK ArticulsReport( LPSTR psw, char param)
{
        //if(param != 'S' && param != 'P' && param != 'G')return -1;
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%s,%c",psw,param);
	return SendCmdAll(111,Tmp);
}

RetData* CALLBACK  PrintDiagnosticInfo(void)
	{
	return SendCmdAll(71,"");
	}

RetData* CALLBACK  PrinterBeep(void)
	{
	return SendCmdAll(80,"");
	}

//int CALLBACK  Fiscalise(HWND hwnd,void *Fn,LPSTR SerialNum,
//                               LPSTR RegNum)
RetData* CALLBACK  Fiscalise(LPSTR psw, LPSTR SerialNum,
                               LPSTR RegNum, int regtype)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	strcpy(Tmp,psw);
   strcat(Tmp,",");
	strcat(Tmp,SerialNum);
   if(RegNum == "")return SendCmdAll(72,Tmp);
   strcat(Tmp,",");
   strcat(Tmp,RegNum);
   strcat(Tmp,",");
   if(regtype == 1)strcat(Tmp,"1");
    else
    {
     if(regtype == 0)strcat(Tmp,"0");
    }
	return SendCmdAll(72,Tmp);
}


RetData* CALLBACK  PrintFiscalMemoryByNum( LPSTR psw, int Start,
																						int End)
//int CALLBACK  PrintFiscalMemoryByNum(HWND hwnd,void *Fn,int Start,
//																						int End,BOOL Short)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%s,%04d,%04d",psw,Start,End);
//	if (!Short) Tmp[strlen(Tmp)-1]='\0';
	return SendCmdAll(73,Tmp);
}

RetData* CALLBACK  GetStatus(BOOL NoWait)
{
	if (NoWait) return SendCmdAll(74,"X");
	else return SendCmdAll(74,"W");
}


RetData* CALLBACK  GetFiscalClosureStatus(BOOL Current)
{
	if (Current) return SendCmdAll(76,"T");
	else return SendCmdAll(76,"");
}

RetData* CALLBACK  PrintReportByDate( LPSTR psw,
																			 LPSTR Start,LPSTR End)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%s,%s,%s",psw,Start,End);
	return SendCmdAll(79,Tmp);
}

RetData* CALLBACK  SetMulDecCurRF(
		LPSTR psw, int Dec,LPSTR enabled, double taxA, double taxB, double taxC, double taxD)
{
//   unsigned int len=strlen(Currency);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Currency[iloop]=table[Currency[iloop]];
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%s,%d,%s,%#4.2f,%#4.2f,%#4.2f,%#4.2f",psw,Dec,enabled,taxA,taxB,taxC,taxD);
	return SendCmdAll(83,Tmp);
}

/*
int CALLBACK  SetMulDecCurRF(HWND hwnd,void *Fn,
		int Mult,int Dec,LPSTR Currency,int RatesFewer)
	{
//   unsigned int len=strlen(Currency);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Currency[iloop]=table[Currency[iloop]];
	sprintf(Tmp,"%d,%d,%s ,%d",Mult,Dec,Currency,RatesFewer);
	return SendCmdAll(hwnd,Fn,83,Tmp);
	}
*/

RetData* CALLBACK  GetMulDecCurRF(void)
	{
	return SendCmdAll(83,"");
	}


RetData* CALLBACK  SetTaxType( int type)
	{
//   unsigned int len=strlen(Text);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Text[iloop]=table[Text[iloop]];
   if(type == 1)return SendCmdAll(84,"1");
   if(type == 0)return SendCmdAll(84,"0");
   return NULL;
	}

/*
int CALLBACK  SetFMNumber(HWND hwnd,void *Fn, LPSTR FMNumber)
	{
	return SendCmdAll(hwnd,Fn,87,FMNumber);
	}
*/

RetData* CALLBACK  ProgramTestArea(BOOL Test)
	{
	if (Test) return SendCmdAll(89,"T");
	else return SendCmdAll(89," ");
  }

RetData* CALLBACK GetDiagnosticInfo(BOOL Calc)
	{
	if (Calc) return SendCmdAll(90,"1");
	else return SendCmdAll(90,"0");
	}

RetData* CALLBACK  SetCountrySerial(int Country,LPSTR Serial)
{
   //страна - Украина - 2
   //номер это 10 байт - две буквы и 8 цифр
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%d,%s",Country,Serial);
	return SendCmdAll(91,Tmp);
}

RetData* CALLBACK  SetFiscalNumber( LPSTR Fiscal)
{
   //номер это 10 байт - одни цифры
   char Tmp[300];
   
   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%s",Fiscal);
	return SendCmdAll(92,Tmp);
}

RetData* CALLBACK  PrintFiscalMemoryByDate(
		LPSTR psw, LPSTR Start,LPSTR End)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%s,%6s,%6s",psw,Start,End);
//	if (!Short) Tmp[strlen(Tmp)-1]='\0';
	return SendCmdAll(94,Tmp);
}

RetData* CALLBACK  PrintReportByNum(
																		 LPSTR psw,	int Start,int End)
{
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
	sprintf(Tmp,"%s,%04d,%04d",psw,Start,End);
	return SendCmdAll(95,Tmp);
}



RetData* CALLBACK  GetCurrentTaxRates(void)
	{
	return SendCmdAll(97,"");
	}

RetData* CALLBACK  GetTaxNumber(void)
	{
	return SendCmdAll(99,"");
	}

RetData* CALLBACK  ClearDisplay(void)
	{
  return SendCmdAll(33,"");
	}

RetData* CALLBACK  DisplayDateTime(void)
	{
  return SendCmdAll(63,"");
	}

RetData* CALLBACK  DisplayTextUL(LPSTR Str)
	{
//   unsigned int len=strlen(Str);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Str[iloop]=table[Str[iloop]];
	return SendCmdAll(47,Str);
	}

RetData* CALLBACK  DisplayFreeText(LPSTR Str)
	{
//   unsigned int len=strlen(Str);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Str[iloop]=table[Str[iloop]];
	return SendCmdAll(100,Str);
	}



RetData* CALLBACK  ServiceInputOutput( double Sum)
{
   char FormatStr[200];
   char Tmp[300];
   
   _strset_s(Tmp,300,0);
   _strset_s(FormatStr, 200, 0);
	strcpy(FormatStr,"%-#.2lf");
   FormatStr[4]=(char)(Decimals+0x30);
	sprintf(Tmp,FormatStr,Sum);
	if(Sum != 0.)return SendCmdAll(70,Tmp);
       else return SendCmdAll(70,"");
}

RetData* CALLBACK  DayInfo(void)
	{
	return SendCmdAll(110,"");
  }

RetData* CALLBACK OpenDrawer( int msec)
{

  char tmper[100];
   
  _strset_s(tmper,100,0);
  if(msec <= 4)return SendCmdAll(106,"");
       else
          {
				sprintf(tmper,"%d",msec);
            return SendCmdAll(106,tmper);
          }
}

RetData* CALLBACK GetReceiptInfo(void)
{
  return SendCmdAll(103,"");
}

RetData* CALLBACK MakeReceiptCopy( char Count)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%c",Count);
   return SendCmdAll(109,Tmp);
}

RetData* CALLBACK CutReceipt(void)
{
	return SendCmdAll(45,"");
}

RetData* CALLBACK OperatorsReport( LPSTR psw)
{
	return SendCmdAll(105,psw);
}

RetData* CALLBACK GetLastReceipt(void)
{
	return SendCmdAll(113,"");
}

RetData* CALLBACK GetOperatorInfo( int Operator)
{
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
	sprintf(Tmp,"%d",Operator);
   return SendCmdAll(112,Tmp);
}

RetData* CALLBACK SetOperatorPassword( int NumOper, LPSTR OldPass, LPSTR NewPass)
{
	char FormatStr[200];
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
   _strset_s(FormatStr, 200, 0);
   strcpy(FormatStr,"%d,%s,%s");
   sprintf(Tmp,FormatStr,NumOper,OldPass,NewPass);
	return SendCmdAll(101,Tmp);
}

RetData* CALLBACK SetOperatorName( int NumOper, LPSTR Password, LPSTR Name)
{
//   unsigned int len=strlen(Name);
//   for(unsigned int iloop=0; iloop<len;++iloop)
//   Name[iloop]=table[Name[iloop]];
	char FormatStr[200];
   char Tmp[300];

   _strset_s(Tmp, 300, 0);
   _strset_s(FormatStr, 200, 0);
   strcpy(FormatStr,"%d,%s,%s");
   sprintf(Tmp,FormatStr,NumOper,Password,Name);
	return SendCmdAll(102,Tmp);
}

RetData* CALLBACK GetCommonArticleInfo(void)
{
   		return SendCmdAll(107,"I");
}

RetData* CALLBACK ProgrammingArticle( char tax, int numart,
     int group, double price, LPSTR psw, LPSTR name)
{
//if(numart <= 0)return -1;
//if(tax != 'А' && tax != 'Б' && tax != 'В' && tax != 'Г' && tax != 'Д' && 
//     (unsigned char)tax != (unsigned char)0x80 && (unsigned char)tax != (unsigned char)0x81 && 
//     (unsigned char)tax != (unsigned char)0x82 && (unsigned char)tax != (unsigned char)0x83 && 
//     (unsigned char)tax != (unsigned char)0x84)return -2;
//if(group < 1 || group > 99)return -3;  //проверить, что больше нуля
//if(price <= 0.)return -4;

  char tmper[400];

  _strset_s(tmper, 400, 0);
  sprintf(tmper,"P%c%d,%d,%-#.2lf,%s,%s",tax,numart,group,price,psw,name);
  return SendCmdAll(107,tmper);
}

RetData* CALLBACK DeleteArticle( int numart, LPSTR psw)
{
//if(numart < 0)return -1;

char frmt[200];
char tmper[30];

_strset_s(frmt, 200, 0);
_strset_s(tmper, 30, 0);
if(numart == 0)
 {
  strcpy(frmt,"DA,");
  strcat(frmt,psw);
  return SendCmdAll(107,frmt);
 }
else
 {
  strcpy(frmt,"D");
  sprintf(tmper,"%d,",numart);
  strcat(frmt,tmper);
  strcat(frmt,psw);
  return SendCmdAll(107,frmt);
 }
}

RetData* CALLBACK ChangeArticlePrice( int numart, double price, LPSTR psw)
{
//if(numart <= 0)return -1;

  char tmper[200];
  _strset_s(tmper, 200, 0);
  sprintf(tmper,"C%d,%-#.2lf,%s",numart,price,psw);
  return SendCmdAll(107,tmper);
}

RetData* CALLBACK GetFirstFreeArticleNum(void)
{
//   		return SendCmdAll(107,"X");
   		return SendCmdAll(107,"x");
}

RetData* CALLBACK GetArticleInfo( int numart)
{
  //if(numart < 0)return -1;

  char frmt[200];
  char tmper[30];

  _strset_s(tmper, 30, 0);
  _strset_s(frmt, 200, 0);
  strcpy(frmt,"R");
  sprintf(tmper,"%d",numart);
  strcat(frmt,tmper);
  return SendCmdAll(107,frmt);
}


RetData* CALLBACK GetFirstArticleNum(void)
{
   		return SendCmdAll(107,"F");
}

RetData* CALLBACK GetNextArticleNum(void)
{
   		return SendCmdAll(107,"N");
}

int CALLBACK SetWinDosPage(bool win)
{
 WinDos=win;
 return WinDos;
}

bool CALLBACK GetWinDosPage()
{
 return WinDos;
}

RetData* CALLBACK LogoLoad( LPSTR psw, int row, char *Data)
{
  //if(row < 0 || row > 95)return -1;

  char tmper[400];
  _strset_s(tmper, 400, 0);
  sprintf(tmper,"%s,%d,%s",psw,row,Data);
  return SendCmdAll(115,tmper);
}

RetData* CALLBACK SetAdminPassword( LPSTR OldPsw, LPSTR NewPsw)
{
  char tmper[40];
  _strset_s(tmper, 40, 0);
  sprintf(tmper,"%s,%s",OldPsw,NewPsw);
  return SendCmdAll(118,tmper);
}

RetData* CALLBACK ClearOperatorPassword( int oper, LPSTR psw)
{
  //if(oper < 1 || oper > 15)return -1;
  char tmper[40];
  _strset_s(tmper, 40, 0);
  sprintf(tmper,"%d,%s",oper,psw);
  return SendCmdAll(119,tmper);
}

RetData* CALLBACK OperatorDataNull( int oper, LPSTR psw)
{
  //if(oper < 1 || oper > 15)return -1;
  char tmper[40];
  _strset_s(tmper, 40, 0);
  sprintf(tmper,"%d,%s",oper,psw);
  return SendCmdAll(104,tmper);
}
