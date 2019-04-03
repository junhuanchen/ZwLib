#ifndef DAVE_DATA_SOURCE_H
#define DAVE_DATA_SOURCE_H

#include "DavePDU.h"

extern int32_t daveDebugSwitch;
extern int32_t DAVE_DECL _daveRecvData(DaveDtSrc *DtSrc, uint8_t *BufIn, int32_t RecvLen);
extern void DAVE_DECL _daveSendData(DaveDtSrc *DtSrc, uint8_t *BufOut, int32_t SendLen);

struct dave_data_source
{
    DaveFile File;
    char * Name;	        /* just a name that can be used in programs dealing with multiple */
    uint8_t MsgNumber;      /* current MPI message number */
    int32_t Slot;           /* slot number for ISO over TCP */
    int32_t Rack;		    /* rack number for ISO over TCP */
    int32_t CommType;	    /* (1=PG Communication,2=OP Communication,3=Step7Basic Communication) */
    int32_t PartPos;   	    // remember position for ISO over TCP fragmentation
    int32_t Routing;	    // nonzero means routing enabled
    int32_t TPDUsize; 	    // size of TPDU for ISO over TCP
    int32_t PDUMaxLen;      // MaxLen of PDU
    int32_t PDUStartO;	    /* position of PDU in outgoing messages. Self is different for transport methodes. */
    int32_t PDUStartI;	    /* position of PDU in incoming messages. Self is different for transport methodes. */
    int32_t PDUNumber; 	    /* current PDU number */
    int32_t Speed;
    int32_t CnctNumber;
    int32_t Protocol;
    int32_t resultLen;	        /* length of last message */
    uint8_t * resultPointer;	    /* used to retrieve single values from the result byte array */

    TmOutType TimeOut; // Time Out Length

    uint8_t MsgIn[daveMaxRawLen];
    uint8_t MsgOut[daveMaxRawLen];

    DaveRoutingData RoutingData;
    _DaveSwapData SwapData;
};

int32_t DAVE_DECL _daveGetLastRecvData(DaveDtSrc *DtSrc, uint8_t * buf, int32_t len);

// S7 ISO In TCP 
int32_t DAVE_DECL _daveGetResponseISOInTCP(DaveDtSrc *DtSrc);
int32_t DAVE_DECL _daveCnctPLCInTCP(DaveDtSrc *DtSrc);
int32_t DAVE_DECL _daveSwapTCP(DaveDtSrc *DtSrc, DavePDU * PDU);
int32_t DAVE_DECL _daveStopS7(DaveDtSrc *DtSrc);
int32_t DAVE_DECL _daveStartS7(DaveDtSrc *DtSrc);
int32_t DAVE_DECL _daveReadS7Bytes(DaveDtSrc *DtSrc, int32_t area, int32_t DBnum, int32_t start, int32_t len, int32_t isBit, void * buffer);
int32_t DAVE_DECL _daveWriteS7Bytes(DaveDtSrc * DtSrc, int32_t area, int32_t DB, int32_t start, int32_t len, void * buffer);

#endif // DAVE_DATA_SOURCE_H
