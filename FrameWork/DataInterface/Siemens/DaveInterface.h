#ifndef DAVE_INTERFACE_H
#define DAVE_INTERFACE_H

// #include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "Log.h"

#ifdef LINUX // 未使用
#define DAVE_DECL
#define DAVE_TRAN_INVALID
typedef void DaveFile;
typedef void TmOutType;
#elif WIN32
#ifdef DOEXPORT
#define EXPORTSPEC __declspec (dllexport) 
#else
#define EXPORTSPEC  
#endif
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define DAVE_DECL WINAPI
#define DAVE_TRAN_INVALID INVALID_SOCKET
typedef SOCKET DaveFile;
typedef struct timeval TmOutType;
#elif STM32
#include "lwip_tcp_client.h"
#include <stdlib.h>
#define DAVE_DECL 
#define DAVE_TRAN_INVALID NULL
typedef tcp_client * DaveFile;
typedef uint32_t TmOutType;
#elif OS_KNOWN
#error Fill in what you need for your OS or API.
#endif

typedef struct dave_pdu DavePDU;
typedef struct dave_pdu_header PDUHeader;
typedef struct dave_pdu_header2 DavePDUHeader2;
typedef struct dave_routing_data DaveRoutingData;
typedef struct dave_data_source DaveDtSrc;
typedef struct dave_interface DaveIntfc;

typedef void * (* DaveAlloc)(unsigned);              /** < 内存函数 alloc 声明  */
typedef void(* DaveFree)(void *);                    /** < 内存函数 free 声明  */

typedef int32_t(DAVE_DECL * _DaveGetResponse) (DaveDtSrc *);
typedef int32_t(DAVE_DECL * _DaveGetLastRecvData) (DaveDtSrc *, uint8_t *, int32_t);
typedef int32_t(DAVE_DECL * _DaveReadBytes) (DaveDtSrc *, int32_t, int32_t, int32_t, int32_t, int32_t, void *);
typedef int32_t(DAVE_DECL * _DaveWriteBytes) (DaveDtSrc *, int32_t, int32_t, int32_t, int32_t, void *);
typedef int32_t(DAVE_DECL * _DaveCnctPLC) (DaveDtSrc *);
typedef int32_t(DAVE_DECL * _DaveStartPLC) (DaveDtSrc *);
typedef int32_t(DAVE_DECL * _DaveStopPLC) (DaveDtSrc *);
typedef int32_t(DAVE_DECL * _DaveSwapData) (DaveDtSrc *, DavePDU *);   // exec data stream out PDU and in TCP 

void DaveDel(DaveFree, void * Self);
DaveIntfc * DaveNewIntfc(DaveAlloc, DaveFree, DaveDtSrc *DtSrc);
DaveDtSrc * DaveNewDtSrc(DaveAlloc, DaveFile File, int32_t CommType, int32_t Slot, int32_t Rack, int32_t Speed, uint32_t TimeOut, int32_t Protocol);

int32_t DaveIntfcCnctPLC(DaveIntfc * Intfc);
int32_t DaveIntfcStopPLC(DaveIntfc * Intfc);
int32_t DaveIntfcStartPLC(DaveIntfc * Intfc);
int32_t DaveIntfcReadBytes(DaveIntfc * Intfc, int32_t area, int32_t DBnum, int32_t start, int32_t len, int32_t isBit, void * buffer);
int32_t DaveIntfcWriteBytes(DaveIntfc * Intfc, int32_t area, int32_t DB, int32_t start, int32_t len, void * buffer);

void DaveIntfcDumpHex(const char *name, uint8_t *buf, int32_t len);
int32_t DaveIntfcGetLastRecvData(DaveIntfc *Intfc, uint8_t * buf, int32_t len);

uint8_t DaveSwapIedOneByte(uint8_t Src);
uint16_t DaveSwapIedTwoByte(uint16_t Src);
uint32_t DaveSwapIedFourByte(uint32_t Src);

/*
*  Some definitions for debugging:
*/
#define daveDebugOFF  	        0x00	/* Closed all Debug Put*/
#define daveDebugRawRead  	    0x01	/* Show the single bytes received */
#define daveDebugSpecialChars  	0x02	/* Show when special chars are read */
#define daveDebugRawWrite	    0x04	/* Show the single bytes written */
#define daveDebugListReachables 0x08	/* Show the steps when determine devices in MPI net */
#define daveDebugInitAdapter 	0x10	/* Show the steps when Initilizing the MPI adapter */
#define daveDebugConnect 	    0x20	/* Show the steps when connecting a PLC */
#define daveDebugPacket 	    0x40
#define daveDebugByte 		    0x80
#define daveDebugCompare 	    0x100
#define daveDebugSwap    	    0x200
#define daveDebugPDU 		    0x400	/* debug PDU handling */
#define daveDebugUpload		    0x800	/* debug PDU loading program blocks from PLC */
#define daveDebugMPI 		    0x1000
#define daveDebugPrintErrors	0x2000	/* Print error messages */
#define daveDebugInside  	    0x4000  /* Print Inside Data messages */
#define daveDebugErrorReporting	0x8000  
#define daveDebugOpen		    0x10000 /* print messages in FileOpen */
#define daveDebugAll            0x1ffff /* print all messages */

/*
*  Protocol types to be used :
*/
#define daveProtoMPI	        0	    /* MPI for S7 300/400 */
#define daveProtoMPI2	        1	    /* MPI for S7 300/400, "Andrew's version" without STX */
#define daveProtoMPI3	        2	    /* MPI for S7 300/400, Step 7 Version, not yet implemented */
#define daveProtoMPI4	        3	    /* MPI for S7 300/400, "Andrew's version" with STX */
#define daveProtoPPI	        10	    /* PPI for S7 200 */
#define daveProtoAS511	        20	    /* S5 programming port protocol */
#define daveProtoS7online       50	    /* use s7onlinx.dll for transport */
#define daveProtoISOTCP	        122     /* ISO over TCP */
#define daveProtoISOTCP243      123     /* ISO over TCP with CP243 */
#define daveProtoMPI_IBH        223	    /* MPI with IBH NetLink MPI to ethernet gateway */
#define daveProtoPPI_IBH        224	    /* PPI with IBH NetLink PPI to ethernet gateway */
#define daveProtoNLpro          230	    /* MPI with NetLink Pro MPI to ethernet gateway */
#define daveProtoUserTransport  255	    /* Libnodave will pass the PDUs of S7 Communication to user */

/*
*  ProfiBus speed constants:
*/
#define daveSpeed9k     0
#define daveSpeed19k    1
#define daveSpeed187k   2
#define daveSpeed500k   3
#define daveSpeed1500k  4
#define daveSpeed4500k  5
#define daveSpeed9300k  6

/*
    Result codes. Genarally, 0 means ok,
    >0 are results (also errors) reported by the PLC
    <0 means error reported by library code.
*/
#define daveResERROR                   -1	/* means no error */
#define daveResOKButMemoryOut           -2	/* means all ok */
#define daveResOK                       0	/* means all ok but user provide buffer no enough*/
#define daveResNoPeripheralAtAddress    1	/* CPU tells there is no peripheral at address */
#define daveResMultipleBitsNotSupported 6 	/* CPU tells it does not support to read a bit block with a */
/* length other than 1 bit. */
#define daveResItemNotAvailable200      3	/* means a a piece of data is not available in the CPU, e.g. */
/* when trying to read a non existing DB or bit bloc of length<>1 */
/* Self code seems to be specific to 200 family. */

#define daveResItemNotAvailable         10	/* means a a piece of data is not available in the CPU, e.g. */
/* when trying to read a non existing DB */

#define daveAddressOutOfRange           5	/* means the data address is beyond the CPUs address range */
#define daveWriteDataSizeMismatch       7	/* means the write data size doesn't fit item size */
#define daveResCannotEvaluatePDU        -123/* PDU is not understood by libnodave */
#define daveResCPUNoData                -124 
#define daveUnknownError                -125 
#define daveEmptyResultError            -126 
#define daveEmptyResultSetError         -127 
#define daveResUnexpectedFunc           -128 
#define daveResUnknownDataUnitSize      -129
#define daveResNoBuffer                 -130
#define daveNotAvailableInS5            -131
#define daveResInvalidLength            -132
#define daveResInvalidParam             -133
#define daveResNotYetImplemented        -134

#define daveResShortPacket              -1024 
#define daveResTimeout                  -1025 

/*
    Some S7 communication function codes (yet unused ones may be incorrect).
*/
#define daveFuncOpenS7Connection	    0xF0
#define daveFuncRead			        0x04
#define daveFuncWrite			        0x05
#define daveFuncRequestDownload		    0x1A
#define daveFuncDownloadBlock		    0x1B
#define daveFuncDownloadEnded		    0x1C
#define daveFuncStartUpload		        0x1D
#define daveFuncUpload			        0x1E
#define daveFuncEndUpload		        0x1F
#define daveFuncInsertBlock		        0x28

/*
    Max number of bytes in a single message.
    An upper limit for MPI over serial is:
    8		transport header
    +2*240	max PDU len *2 if every character were a DLE
    +3		DLE,ETX and BCC
    = 491

    Later I saw some programs offering up to 960 bytes in PDU size negotiation

    Max number of bytes in a single message.
    An upper limit for MPI over serial is:
    8		transport header
    +2*960	max PDU len *2 if every character were a DLE
    +3		DLE,ETX and BCC
    = 1931

    For now, we take the rounded max of all this to determine our buffer size. Self is ok
    for PC systems, where one k less or more doesn't matter.
*/
#define daveMaxRawLen           2048

/*
    Communication types
*/
#define daveCommTypePG         1	/* (Monopoly) Programmer communication with programming device (PG) (default in Libnodave) */
#define daveCommTypeOP         2	/* (Share) communication with operator panel (OP)) */
#define daveCommTypeS7Basic    3	/* communication with another CPU ? */

/*
    Use these constants for parameter "area" in daveReadBytes and daveWriteBytes
*/
#define daveSysInfo     0x3		/* System info of 200 family    */
#define daveSysFlags    0x5	    /* System flags of 200 family   */
#define daveAnaIn       0x6		/* analog inputs of 200 family  */
#define daveAnaOut      0x7		/* analog outputs of 200 family */

#define daveP           0x80    /* direct peripheral access */
#define daveInputs      0x81    /* Input memory image       */
#define daveOutputs     0x82    /* Output memory image      */
#define daveFlags       0x83    /* Flags/Markers            */
#define daveDB          0x84	/* data blocks              */
#define daveDI          0x85	/* instance data blocks     */
#define daveLocal       0x86 	/* not tested               */
#define daveV           0x87	/* don't know what it is    */
#define daveCounter     28	    /* S7 counters              */
#define daveTimer       29	    /* S7 timers                */
#define daveCounter200  30	    /* IEC counters (200 family)*/
#define daveTimer200    31		/* IEC timers (200 family)  */
#define daveSysDataS5   0x86	/* system data area ?       */
#define daveRawMemoryS5 0		/* just the raw memory      */

#define daveTypeByteLen     1   /* Recv S7-200's Byte TypeLen */
#define daveTypeWordLen     2   /* Recv S7-200's WORD TypeLen  */
#define daveTypeCountLen    3   /* Recv S7-200's daveCounter200 TypeLen  */
#define daveTypeDWordLen    4   /* Recv S7-200's DWORD TypeLen  */
#define daveTypeTimeLen     5   /* Recv S7-200's daveTimer200 TypeLen  */

#endif /* DAVE_INTERFACE_H */
