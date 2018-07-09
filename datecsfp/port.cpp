
#include "stdafx.h"
#include "fp.h"
#include "port.h"



NPTTYINFO   npTTYInfo ;

#if defined OLDBR

LRESULT CreateTTYInfo( int Port )
{
  if ( Port == 0 )
    return ( (LRESULT) -1 ) ;

	if (NULL == (npTTYInfo =
                   (NPTTYINFO) LocalAlloc( LPTR, sizeof( TTYINFO ) )))
      return ( (LRESULT) -1 ) ;

   // initialize TTY info structure

	COMDEV( npTTYInfo )       = 0 ;
	CONNECTED( npTTYInfo )    = FALSE ;
	PORT( npTTYInfo )         = Port ;
	BAUDRATE( npTTYInfo )     = CBR_115200 ;
	BYTESIZE( npTTYInfo )     = 8 ;
	PARITY( npTTYInfo )       = EVENPARITY ;
	STOPBITS( npTTYInfo )     = TWOSTOPBITS ;
	XONXOFF( npTTYInfo )      = FALSE ;

	return ( (LRESULT) TRUE ) ;

} // end of CreateTTYInfo()

#else

LRESULT CreateTTYInfo( int Port, unsigned long BaudRate )
{
  if ( Port == 0 || BaudRate == 0 )
    return ( (LRESULT) -1 ) ;

	if (NULL == (npTTYInfo =
                   (NPTTYINFO) LocalAlloc( LPTR, sizeof( TTYINFO ) )))
      return ( (LRESULT) -1 ) ;

   // initialize TTY info structure

	COMDEV( npTTYInfo )       = 0 ;
	CONNECTED( npTTYInfo )    = FALSE ;
	PORT( npTTYInfo )         = Port ;
	BAUDRATE( npTTYInfo )	  = BaudRate;
	BYTESIZE( npTTYInfo )     = 8 ;
	PARITY( npTTYInfo )       = NOPARITY;
	STOPBITS( npTTYInfo )     = ONESTOPBIT;
	XONXOFF( npTTYInfo )      = FALSE ;

	return ( (LRESULT) TRUE ) ;

} // end of CreateTTYInfo()

#endif

BOOL DestroyTTYInfo( void )
{
   // force connection closed (if not already closed)

   if (npTTYInfo)
   {
		if (CONNECTED( npTTYInfo )) CloseConnection() ;

		LocalFree( npTTYInfo ) ;
      npTTYInfo = NULL;
   }
   return ( TRUE ) ;

} // end of DestroyTTYInfo()

#if defined LOGGED

//GetDllStr('Datecs.dll','PrnCheck','1;'+AppHandle+';2')
BOOL OpenConnection( void )
{
   char       szPort[ 15 ] ;

   wsprintf( szPort, "prot_COM%d.dat", PORT( npTTYInfo ) ) ;

   // open COMM device

   if ((COMDEV( npTTYInfo ) =
      CreateFile( szPort, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  CREATE_ALWAYS,
                  FILE_ATTRIBUTE_NORMAL 
//                  |FILE_FLAG_OVERLAPPED
                  , // overlapped I/O
                  NULL )) == (HANDLE) -1 )
   {
      CONNECTED( npTTYInfo ) = FALSE ;
		LOGCREATE( npTTYInfo ) = FALSE ;
      return ( FALSE ) ;
	 }
	else
	{
		SetFilePointer( HLOG( npTTYInfo ), 0, NULL, FILE_END );
		LOGCREATE( npTTYInfo ) = TRUE ;
	}

	CONNECTED( npTTYInfo ) = TRUE ;
   return TRUE ;

} // end of OpenConnection()

#else

BOOL OpenConnection( void )
{
   char       szPort[ 15 ] ;
   BOOL       fRetVal ;

   COMMTIMEOUTS  CommTimeOuts ;
#ifndef DUMMY
   wsprintf( szPort, "\\\\.\\COM%d", PORT( npTTYInfo ) ) ;

   if ((COMDEV( npTTYInfo ) =
      CreateFile( szPort, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL 
//                  |FILE_FLAG_OVERLAPPED
                  , // overlapped I/O
                  NULL )) == (HANDLE) -1 )
      return ( FALSE ) ;
   else
   {
#endif
#ifdef W_LOG
		char       szLog[ 15 ] ;
      
		memset( szLog, 0x00, 15 );
		wsprintf( szLog, "prot_COM%d.dat", PORT( npTTYInfo ) ) ;

		// open LOG device

		if ((HLOG( npTTYInfo ) =
			CreateFile( szLog, GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,                 // no security attrs
							OPEN_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL )) == (HANDLE) -1 )
		{
			LOGCREATE( npTTYInfo ) = FALSE ;
		}
		else
		{
			SetFilePointer( HLOG( npTTYInfo ), 0, NULL, FILE_END );
			LOGCREATE( npTTYInfo ) = TRUE ;
		}
#endif
#ifndef DUMMY
		SetupComm( COMDEV( npTTYInfo ), RXQUEUE, TXQUEUE ) ;

      PurgeComm( COMDEV( npTTYInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

      CommTimeOuts.ReadIntervalTimeout = 10*IntervalTimeout ;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
      // CBR_9600 is approximately 1byte/ms. For our purposes, allow
      // double the expected time per character for a fudge factor.
      CommTimeOuts.WriteTotalTimeoutMultiplier = 0; //2*CBR_9600/BAUDRATE( npTTYInfo ) ;
      CommTimeOuts.WriteTotalTimeoutConstant = 500 ;
      SetCommTimeouts( COMDEV( npTTYInfo ), &CommTimeOuts ) ;
   }

   fRetVal = SetupConnection() ;

   if (fRetVal)
   {
      CONNECTED( npTTYInfo ) = TRUE ;
//      EscapeCommFunction( COMDEV( npTTYInfo ), SETDTR ) ;

      SetCommMask( COMDEV( npTTYInfo ), EV_RXCHAR | EV_BREAK | EV_ERR ) ;

   }
   else
   {
      CONNECTED( npTTYInfo ) = FALSE ;
      CloseHandle( COMDEV( npTTYInfo ) ) ;

#ifdef W_LOG
		LOGCREATE( npTTYInfo ) = FALSE ;
      CloseHandle( HLOG( npTTYInfo ) ) ;
#endif

   }

   return ( fRetVal ) ;
#else
   CONNECTED( npTTYInfo ) = TRUE ;
   return ( TRUE ) ;
#endif
} // end of OpenConnection()

#endif

BOOL SetupConnection( void )
{
   BOOL       fRetVal ;
   DCB        dcb ;

   dcb.DCBlength = sizeof( DCB ) ;

   GetCommState( COMDEV( npTTYInfo ), &dcb ) ;

   dcb.BaudRate = BAUDRATE( npTTYInfo ) ;
   dcb.ByteSize = BYTESIZE( npTTYInfo ) ;
   dcb.Parity = PARITY( npTTYInfo ) ;
   dcb.StopBits = STOPBITS( npTTYInfo ) ;

   dcb.fDtrControl = DTR_CONTROL_DISABLE ;
   dcb.fRtsControl = RTS_CONTROL_DISABLE ;
   dcb.fInX = dcb.fOutX = 0 ;
   dcb.fTXContinueOnXoff = 1 ;
   dcb.XonChar = ASCII_XON ;
   dcb.XoffChar = ASCII_XOFF ;
   dcb.XonLim = 100 ;
   dcb.XoffLim = 100 ;
   dcb.fBinary = TRUE ;
   dcb.fParity = TRUE ;

   fRetVal = SetCommState( COMDEV( npTTYInfo ), &dcb ) ;

   return ( fRetVal ) ;

} // end of SetupConnection()

#if defined LOGGED

BOOL CloseConnection(  void  )
{
   // set connected flag to FALSE

   CONNECTED( npTTYInfo ) = FALSE ;

   CloseHandle( COMDEV( npTTYInfo ) ) ;

   // change the selectable items in the menu

   return ( TRUE ) ;

} // end of CloseConnection()

#else

BOOL CloseConnection(  void  )
{
   CONNECTED( npTTYInfo ) = FALSE ;
#ifndef DUMMY
   FlushFileBuffers(COMDEV( npTTYInfo ));

   SetCommMask( COMDEV( npTTYInfo ), 0 ) ;

//   EscapeCommFunction( COMDEV( npTTYInfo ), CLRDTR ) ;

   PurgeComm( COMDEV( npTTYInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                   PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
   CloseHandle( COMDEV( npTTYInfo ) ) ;
#endif
#ifdef W_LOG
	if ( LOGCREATE( npTTYInfo ) )
	{
		LOGCREATE( npTTYInfo ) = FALSE ;
		FlushFileBuffers( HLOG( npTTYInfo ) );
      CloseHandle( HLOG( npTTYInfo ) ) ;
	}
#endif

   return ( TRUE ) ;

} // end of CloseConnection()

#endif


BOOL ReOpenConnection( void )
{
   // force connection closed (if not already closed)

   if (npTTYInfo)
   {
		if (CONNECTED( npTTYInfo )) {
			CloseConnection() ;
			return ( OpenConnection() ) ;
		}
   }
   return ( FALSE ) ;

} // end of ReOpenConnection()


BOOL WriteCommBlock( LPSTR lpByte , DWORD dwBytesToWrite)
{

   BOOL        fWriteStat = 0;
   DWORD       dwBytesWritten = 0 ;

#ifdef W_LOG
	if ( LOGCREATE( npTTYInfo ) )
	{
		char * szBuffer ;
      char timebuf[10] ;
      char datebuf[10] ;

		szBuffer = ( char* ) malloc( 50 ) ;
		memset( szBuffer, 0x00, 50 );

		_tzset();

		memset( timebuf, 0x00, 10 );
		_strtime_s( timebuf, 10 );
		memset( datebuf, 0x00, 10 );
		_strdate_s( datebuf, 10 );
		int bLen = sprintf_s( szBuffer, 50, "%s %s -->\t", datebuf, timebuf );
		WriteFile( HLOG( npTTYInfo ), szBuffer, DWORD(bLen), &dwBytesWritten, NULL ) ;

		WriteFile( HLOG( npTTYInfo ), lpByte, dwBytesToWrite, &dwBytesWritten, NULL ) ;

		memset( szBuffer, 0x00, 50 );
		bLen = sprintf_s( szBuffer, 50, "\n" );
		WriteFile( HLOG( npTTYInfo ), szBuffer, DWORD(bLen), &dwBytesWritten, NULL ) ;

		free( szBuffer );
	}
#endif
#ifndef DUMMY
	dwBytesWritten = 0;
   fWriteStat = WriteFile( COMDEV( npTTYInfo ), lpByte, dwBytesToWrite,
                           &dwBytesWritten, NULL ) ;
   return ( (fWriteStat > 0) && (dwBytesWritten > 0) && (dwBytesToWrite == dwBytesWritten) ) ;
#else
	return ( TRUE );
#endif
} // end of WriteCommBlock()

int ReadCommBlock( LPSTR lpszBlock, int nMaxLength, BOOL pWait = TRUE )
{
   BOOL       fReadStat ;
   COMSTAT    ComStat ;
   DWORD      dwErrorFlags ;
   DWORD      dwLength;
//	DWORD      dwEvMask ;
   LPSTR      lpszPoint = lpszBlock;
   int cWait;

   ComStat.cbInQue = 0;

//   if (lpWait) WaitCommEvent(COMDEV( npTTYInfo ), &dwEvMask, NULL );

   do
   {
      if ( !ComStat.cbInQue ) 
      {
         ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
         dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;
      }               

      if ( dwLength > 0 )
      {
         fReadStat = ReadFile( COMDEV( npTTYInfo ), lpszPoint,
		                       dwLength, &dwLength, NULL ) ;

         if ( fReadStat ) lpszPoint += dwLength ;
         if ((BYTE)lpszBlock[lpszPoint-lpszBlock-1] == TERMINATOR) 
            dwLength = 0;

         ComStat.cbInQue = 0;
      } 
      else
      {
         if ( pWait )
         {
            if ((BYTE)lpszBlock[lpszPoint-lpszBlock] != TERMINATOR)
            {
               cWait = 0;
               do
               {
                  Sleep( IntervalTimeout*CBR_115200/BAUDRATE( npTTYInfo ) ) ;
                  ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
                  dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;
               } while ( (dwLength == 0) &&  (DEF_REP_COUNT_WAIT > ++cWait) );
            }
         }
         else
         {
            Sleep( 2*IntervalTimeout*CBR_115200/BAUDRATE( npTTYInfo ) ) ;
            ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
            dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;
         }
      }
   } while ( dwLength > 0 ) ;

#ifdef W_LOG
	if ( LOGCREATE( npTTYInfo ) )
	{
		char * szBuffer ;
		char timebuf[10] ;
		char datebuf[10] ;
		DWORD dwBWritten ;
		DWORD dwBToWrite ;

		szBuffer = ( char* ) malloc( 50 ) ;
		memset( szBuffer, 0x00, 50 );

		_tzset();

		memset( timebuf, 0x00, 10 );
		_strtime_s( timebuf, 10 );
		memset( datebuf, 0x00, 10 );
		_strdate_s( datebuf, 10 );
		int bLen = sprintf_s( szBuffer, 50, "%s %s <--\t", datebuf, timebuf );
		WriteFile( HLOG( npTTYInfo ), szBuffer, DWORD(bLen), &dwBWritten, NULL ) ;
		dwBToWrite = lpszPoint - lpszBlock ;
		WriteFile( HLOG( npTTYInfo ), lpszBlock, dwBToWrite, &dwBWritten, NULL ) ;

		memset( szBuffer, 0x00, 50 );
		bLen = sprintf_s( szBuffer, 50, "\n" );
		WriteFile( HLOG( npTTYInfo ), szBuffer, DWORD(bLen), &dwBWritten, NULL ) ;

		free( szBuffer );
	}
#endif

   return ( lpszPoint - lpszBlock ) ;

} // end of ReadCommBlock()

int ReadCommBlock_( LPSTR lpszBlock, int nMaxLength )
{
   BOOL       fReadStat ;
   COMSTAT    ComStat ;
   DWORD      dwErrorFlags ;
   DWORD      dwLength ;//, dwEvMask ;
   LPSTR      lpszPoint = lpszBlock;

/*
   fReadStat = ReadFile( COMDEV( npTTYInfo ), lpszPoint,
		                    1, &dwLength, NULL ) ;

   if ( fReadStat && dwLength ) 
   {
      lpszPoint ++ ;
      dwLength = 0 ;
*/
//   if ( WaitCommEvent( COMDEV( npTTYInfo ), &dwEvMask, NULL ) )
   {
//      if ( ( ( dwEvMask & EV_RXCHAR ) == EV_RXCHAR ) )
//                  ( ( dwEvMask & EV_TXEMPTY ) == EV_TXEMPTY ) )
      {
         do
         {
            dwLength = 0 ;

            Sleep( IntervalTimeout * CBR_115200/BAUDRATE( npTTYInfo ) ) ;
            ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
            dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;

            if ( dwLength > 0 )
            {
               fReadStat = ReadFile( COMDEV( npTTYInfo ), lpszPoint,
		                             dwLength, &dwLength, NULL ) ;

               if ( fReadStat ) lpszPoint += dwLength ;
            }

         } while ( dwLength > 0 ) ;

      }
//      ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
   }
//   }

   return ( lpszPoint - lpszBlock ) ;

} // end of ReadCommBlock()

void ClearPort(void)
{
   PurgeComm( COMDEV( npTTYInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                   PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
}

HANDLE GetPortHandle(void)
{
   return ( COMDEV( npTTYInfo ) );
}
