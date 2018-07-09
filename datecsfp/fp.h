#define WM_FPRESPONSE  0x1099
#define PREAMBLE      0x01
#define NACK          0x15
#define SYN           0x16
#define TERMINATOR    0x03
#define POSTAMBLE     0x05
#define NOPAPER       0x13

#define SYNTAX_ERROR                  0x00000001
#define INVALID_CMD                   0x00000002
#define INVALID_TIME                  0x00000004
#define PRINT_ERROR                   0x00000008
#define SUM_OVERFLOW                  0x00000010
#define CMD_NOT_ALLOWED               0x00000020
#define RAM_CLEARED                   0x00000040
#define PRINT_RESTART                 0x00000080
#define RAM_DESTROYED                 0x00000100
#define PAPER_OUT                     0x00000200
#define FISCAL_OPEN                   0x00000400
#define NONFISCAL_OPEN                0x00000800
#define SERVICE_OPEN                  0x00001000
#define F_ABSENT                      0x00002000
#define F_MODULE_NUM                  0x00004000
#define F_WRITE_ERROR                 0x00010000
#define F_FULL                        0x00020000
#define F_READ_ONLY                   0x00040000
#define F_CLOSE_ERROR                 0x00080000
#define F_LESS_30                     0x00100000
#define F_FORMATTED                   0x00200000
#define F_FISCALIZED                  0x00400000
#define F_SER_NUM                     0x00800000

#define PROTOCOL_ERROR                0x01000000
#define NACK_RECEIVED                 0x02000000
#define TIMEOUT_ERROR                 0x04000000
#define COMMON_ERROR                  0x08000000
#define F_COMMON_ERROR                0x10000000
#define ADD_PAPER                     0x20000000

#define ANY_ERROR                     0xff000000

struct RetData {
               int Count;
               int CmdCode;
               LPARAM UserData;
               LPARAM Status;
               LPSTR CmdName;
               LPSTR SendStr;
               LPSTR Whole;
               LPSTR RetItem[20];
               unsigned char OrigStat[6];
               };

//extern "C" {

//Windows95 5.10.98
int CALLBACK InitFPport(int, int, int);

int   CALLBACK CloseFPport(void);
int   CALLBACK SendCmdMsg(int, LPSTR);
int   CALLBACK SendCmdCbk(int, LPSTR);
RetData* CALLBACK SendCmdAll(int, LPSTR);

int CALLBACK SendLast(BOOL);
LPSTR CALLBACK CmdDescription(int);
void  CALLBACK SetMessageNum(UINT);
int   CALLBACK SetDecimals(int);
int CALLBACK  SetShowAmount(int);

RetData* CALLBACK OpenNonfiscalReceipt(void);
RetData* CALLBACK CloseNonfiscalReceipt(void);
RetData* CALLBACK PrintNonfiscalText(LPSTR);
RetData* CALLBACK SetHeaderFooter(int, LPSTR);
RetData* CALLBACK AdvancePaper(int);
RetData* CALLBACK DisplayTextLL(LPSTR);
RetData* CALLBACK ClearDisplay(void);
RetData* CALLBACK DisplayTextUL(LPSTR);
RetData* CALLBACK  OpenFiscalReceipt(DWORD,	LPSTR, DWORD, BOOL);
RetData* CALLBACK GetTaxRates(LPSTR,LPSTR, LPSTR);
RetData* CALLBACK SubTotal(BOOL,BOOL, double, double);
RetData* CALLBACK Total(LPSTR, char, double);
RetData* CALLBACK PrintFiscalText(LPSTR);
RetData* CALLBACK CloseFiscalReceipt(void);
//анулировать фискальный чек ResetReceipt (новая команда)  +++++++
RetData* CALLBACK ResetReceipt(void);
RetData* CALLBACK SetDateTime(LPSTR, LPSTR);
RetData* CALLBACK GetDateTime(void);
RetData* CALLBACK DisplayDateTime(void);
RetData* CALLBACK LastFiscalClosure(int);
RetData* CALLBACK GetCurrentTaxes(int);
RetData* CALLBACK GetCurrentSums(void);
RetData* CALLBACK GetFreeClosures(void);
RetData* CALLBACK FiscalClosure(LPSTR, char);
RetData* CALLBACK ServiceInputOutput(double);
RetData* CALLBACK PrintDiagnosticInfo(void);
//Fiscalise - сильно изменена
RetData* CALLBACK Fiscalise(LPSTR, LPSTR, LPSTR, int);
//PrintFiscalMemoryByNum доработать исходный текст
RetData* CALLBACK PrintFiscalMemoryByNum(LPSTR, int, int);
RetData* CALLBACK GetStatus(BOOL);
RetData* CALLBACK GetFiscalClosureStatus(BOOL);
//PrintReportByDate доработать исходный текст
RetData* CALLBACK PrintReportByDate(LPSTR, LPSTR, LPSTR);
RetData* CALLBACK PrinterBeep(void);
//SetMulDecCurRF проверить
RetData* CALLBACK  SetMulDecCurRF(LPSTR, int, LPSTR, double, double, double, double);
RetData* CALLBACK GetMulDecCurRF(void);

RetData* CALLBACK OperatorDataNull(int, LPSTR);



//SetTaxType - новая команда
RetData* CALLBACK SetTaxType(int);
//OpenRepaymentReceipt - новая команда
RetData* CALLBACK  OpenRepaymentReceipt(DWORD,	LPSTR, DWORD, BOOL);
RetData* CALLBACK ProgramTestArea(BOOL);
RetData* CALLBACK GetDiagnosticInfo(BOOL);
RetData* CALLBACK SetCountrySerial(int, LPSTR);
RetData* CALLBACK SetFiscalNumber(LPSTR);
RetData* CALLBACK PrintFiscalMemoryByDate(LPSTR, LPSTR, LPSTR);
RetData* CALLBACK PrintReportByNum(LPSTR, int, int);
RetData* CALLBACK GetCurrentTaxRates(void);
RetData* CALLBACK GetLastTaxRates(LPSTR);
RetData* CALLBACK SetTaxNumber(LPSTR, int);
RetData* CALLBACK GetTaxNumber(void);
RetData* CALLBACK DisplayFreeText(LPSTR);
RetData* CALLBACK SetOperatorPassword(int, LPSTR, LPSTR);
RetData* CALLBACK SetOperatorName(int, LPSTR, LPSTR);
RetData* CALLBACK GetReceiptInfo(void);
RetData* CALLBACK OpenDrawer(int);
RetData* CALLBACK OperatorsReport(LPSTR);
RetData* CALLBACK ArticulsReport(LPSTR, char);
RetData* CALLBACK MakeReceiptCopy(char);
RetData* CALLBACK DayInfo(void);
RetData* CALLBACK GetOperatorInfo(int);
RetData* CALLBACK GetLastReceipt(void);


RetData* CALLBACK SaleArticle( bool, int, double, double, double);
RetData* CALLBACK SaleArticleAndDisplay(bool, int, double, double, double);





//int   CALLBACK GetTaxRates(LPSTR, LPSTR);
//??????
RetData* CALLBACK FiscalMemoryLookup(int);




//FP3530T only
RetData* CALLBACK CutReceipt(void);




//Windows95 5.10.98
HANDLE  CALLBACK GetRSId(void);

//int   CALLBACK GetRSId(void);
RetData* CALLBACK GetCommonArticleInfo(void);
RetData* CALLBACK ProgrammingArticle(char, int, int, double, LPSTR, LPSTR);
RetData* CALLBACK DeleteArticle(int, LPSTR);
RetData* CALLBACK ChangeArticlePrice(int, double, LPSTR);
RetData* CALLBACK GetFirstFreeArticleNum(void);
RetData* CALLBACK GetArticleInfo(int);
RetData* CALLBACK GetFirstArticleNum(void);
RetData* CALLBACK GetNextArticleNum(void);

RetData* CALLBACK LogoLoad(LPSTR, int, char *);
RetData* CALLBACK SetAdminPassword(LPSTR, LPSTR);
RetData* CALLBACK ClearOperatorPassword(int, LPSTR);


int CALLBACK SetWinDosPage(bool);
bool CALLBACK GetWinDosPage();

//}
