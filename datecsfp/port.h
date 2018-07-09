
#define USECOMM

#define MAXBLOCK        300

#define MAXLEN_TEMPSTR  300

#define RXQUEUE         1024
#define TXQUEUE         1024

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#define IntervalTimeout 5

#define DEF_REP_COUNT_WAIT 50
#define DEF_STAB_PROT 10

typedef struct tagTTYINFO
{
   HANDLE  idComDev ;
   BYTE    bPort;
   BOOL    fConnected, fXonXoff, fLocalEcho ;
   BYTE    bByteSize, bFlowCtrl, bParity, bStopBits ;
   DWORD   dwBaudRate ;

#ifdef W_LOG
	HANDLE  hLog ;
	BOOL    fLogCreated;
#endif

} TTYINFO, *NPTTYINFO ;

#define COMDEV( x ) (x -> idComDev)
#define PORT( x )   (x -> bPort)
#define CONNECTED( x ) (x -> fConnected)
#define XONXOFF( x ) (x -> fXonXoff)
#define BYTESIZE( x ) (x -> bByteSize)
#define FLOWCTRL( x ) (x -> bFlowCtrl)
#define PARITY( x ) (x -> bParity)
#define STOPBITS( x ) (x -> bStopBits)
#define BAUDRATE( x ) (x -> dwBaudRate)
#define XOFFSET( x ) (x -> xOffset)
#define YOFFSET( x ) (x -> yOffset)

#ifdef W_LOG
	#define HLOG( x ) (x -> hLog)
	#define LOGCREATE( x ) (x -> fLogCreated)
#endif

#define _fmemset   memset
#define _fmemmove  memmove

#if defined OLDBR
LRESULT CreateTTYInfo( int ) ;
#else
LRESULT CreateTTYInfo( int, unsigned long ) ;
#endif
BOOL DestroyTTYInfo() ;
int ReadCommBlock( LPSTR, int, BOOL ) ;
BOOL WriteCommBlock( LPSTR, DWORD);
BOOL OpenConnection() ;
BOOL SetupConnection() ;
BOOL ReOpenConnection() ;
BOOL CloseConnection() ;
void ClearPort();
HANDLE GetPortHandle();
