
#include "DaveDataSource.h"

int32_t DAVE_DECL _daveSwap(DaveDtSrc *DtSrc, DavePDU *PDU)
{
    int32_t res;
    if ( (PDU->header[4] == 0) && (PDU->header[5] == 0) )
    { /* do not number already numbered PDUs 12/10/04 */
        DtSrc->PDUNumber++;
        LogExec((daveDebugSwitch & daveDebugSwap),
                LogPut("_daveSwap PDU number: %d\n", DtSrc->PDUNumber));
        PDU->header[5] = DtSrc->PDUNumber % 256;	// test!
        PDU->header[4] = DtSrc->PDUNumber / 256;	// test!
    }
    res = DtSrc->SwapData(DtSrc, PDU);
    LogExec(((daveDebugSwitch & daveDebugSwap) || (daveDebugSwitch & daveDebugErrorReporting)),
            LogPut("result of _daveSwap: %d\n", res));
    return res;
}

DaveDtSrc * DaveNewDtSrc(DaveAlloc Alloc, DaveFile File, int32_t CommType, int32_t Slot, int32_t Rack, int32_t Speed, uint32_t TimeOut, int32_t Protocol)
{
    DaveDtSrc * Self = (DaveDtSrc *)Alloc(sizeof(*Self));
    if ( Self )
    {
        memset(Self, 0, sizeof(*Self));
#ifdef STM32
		Self->TimeOut = TimeOut;
#else
		Self->TimeOut.tv_sec = TimeOut / 1000000;
		Self->TimeOut.tv_usec = TimeOut % 1000000;
#endif			
        Self->File = File;
        Self->CommType = CommType;      // daveCommTypePG
        Self->Slot = Slot;              // 与上合称 远程 TSAP:X.X (
        Self->Rack = Rack;
        Self->Speed = Speed;
        Self->Protocol = Protocol;
        Self->PDUMaxLen = 1920;			// assume an (unreal?) maximum
        Self->CnctNumber = 0x14;	    // 1/10/05 trying Andrew's patch
        Self->PDUNumber = 0xFFFE;		// just a start value; // test!
        Self->MsgNumber = 0;

        switch ( Self->Protocol )
        {
            case daveProtoISOTCP:
            case daveProtoISOTCP243:
                Self->PDUStartO = 7;	/* position of PDU in outgoing messages */
                Self->PDUStartI = 7;	/* position of PDU in incoming messages */
                break;
            default:
                Self->PDUStartO = 8;	/* position of PDU in outgoing messages */
                Self->PDUStartI = 8;	/* position of PDU in incoming messages */
                LogExec((daveDebugSwitch & daveDebugPrintErrors),
						LogPut("Unknown protocol on interface %s\n", Self->Name));
        }
    }
    return Self;
}

void DAVE_DECL _daveSendISOPacket(DaveDtSrc *DtSrc, int32_t SendLen)
{
    uint8_t *SendBuf = DtSrc->MsgOut + DtSrc->PartPos;

    SendLen += 4;
    SendBuf[3] = SendLen % 0x100;	// was %0xFF, certainly a bug
    SendBuf[2] = SendLen / 0x100;
    SendBuf[1] = 0;
    SendBuf[0] = 3;

    LogExec((daveDebugSwitch & daveDebugByte), 
            _daveDumpHex("send packet: ", SendBuf, SendLen));
    
#ifdef HAVE_SELECT
    daveWriteFile(DtSrc->File, SendBuf, size, i);
#endif
	
    _daveSendData(DtSrc, SendBuf, SendLen);
	
}

/*
    Read one complete packet.
    */
int32_t DAVE_DECL _daveReadISOPacket(DaveDtSrc *DtSrc)
{
    int32_t recvlen;
    uint8_t *MsgIn = DtSrc->MsgIn;
    recvlen = _daveRecvData(DtSrc, MsgIn, 4);
    if ( 4 == recvlen )
    {
        int32_t length = MsgIn[3] + 0x100 * MsgIn[2];
        recvlen += _daveRecvData(DtSrc, MsgIn + 4, length - 4);

        LogExec((daveDebugSwitch & daveDebugByte),
                (LogPut("readISOpacket: %d bytes read, %d needed\n", recvlen, length),
                _daveDumpHex("readISOpacket: packet", MsgIn, recvlen)));

        uint8_t lhdr[7];
        int32_t tmplen, follow;
        follow = ((MsgIn[5] == 0xf0) && ((MsgIn[6] & 0x80) == 0));
        while ( follow )
        {
            LogExec((daveDebugSwitch & daveDebugByte),
                    LogPut("readISOpacket: more data follows %d\n", MsgIn[6]));

            tmplen = _daveRecvData(DtSrc, lhdr, 7);
            length = lhdr[3] + 0x100 * lhdr[2];

            LogExec((daveDebugSwitch & daveDebugByte),
                    _daveDumpHex("readISOpacket: follow %d %d", lhdr, tmplen));

            tmplen = _daveRecvData(DtSrc, MsgIn + recvlen, length - 7);

            LogExec((daveDebugSwitch & daveDebugByte),
                    _daveDumpHex("readISOpacket: follow %d %d", MsgIn + recvlen, tmplen));

            recvlen += tmplen;
            follow = ((lhdr[5] == 0xf0) && ((lhdr[6] & 0x80) == 0));
        }
        return recvlen;
    }
    else if ( recvlen <= 0 )
    {
        return 0;   /* TimeOut! */
    }
    else
    {
        LogExec((daveDebugSwitch & daveDebugByte),
                (LogPut("recv real len : %d ", recvlen),
                _daveDumpHex("readISOpacket: short packet", MsgIn, recvlen)));

        return recvlen; /* short packet */
    }
}

#define ISOInTCPMinPacketLength 16
int32_t DAVE_DECL _daveGetResponseISOInTCP(DaveDtSrc *DtSrc)
{
    int32_t recvlen;
    recvlen = _daveReadISOPacket(DtSrc);
    if ( 7 == recvlen )
    {
        LogExec((daveDebugSwitch & daveDebugByte),
                LogPut("CPU sends funny 7 byte packets.\n"));

        recvlen = _daveReadISOPacket(DtSrc);
    }
    if ( 0 == recvlen )
    {
        return daveResTimeout;
    }
    else
    {
        if ( recvlen < ISOInTCPMinPacketLength )
        {
            return daveResShortPacket;
        }
    }
    return daveResOK;
}

/*
Executes the dialog around one message:
*/
int32_t DAVE_DECL _daveSwapTCP(DaveDtSrc *DtSrc, DavePDU * PDU)
{
    uint8_t *MsgOut = DtSrc->MsgOut;
    int32_t totranlen, sendlen;

    LogExec((daveDebugSwitch & daveDebugSwap), LogPut("%s enter _daveSwapTCP\n", DtSrc->Name));

    //_daveSendISOPacket(DtSrc, 3 + PDU->hlen + PDU->plen + PDU->dlen);

    DtSrc->PartPos = 0;
    totranlen = PDU->hlen + PDU->plen + PDU->dlen;
    while ( totranlen )
    {
        if ( totranlen > DtSrc->TPDUsize )
        {
            sendlen = DtSrc->TPDUsize;
            MsgOut[DtSrc->PartPos + 6] = 0x00;
        }
        else
        {
            sendlen = totranlen;
            MsgOut[DtSrc->PartPos + 6] = 0x80;
        }
        MsgOut[DtSrc->PartPos + 5] = 0xf0;
        MsgOut[DtSrc->PartPos + 4] = 0x02;
        _daveSendISOPacket(DtSrc, 3 + sendlen);
        totranlen -= sendlen;
        DtSrc->PartPos += sendlen;
    }
		
    return _daveGetResponseISOInTCP(DtSrc);
}

/*
Sets up pointers to the fields of a received message.
*/
int32_t DAVE_DECL _daveSetupReceivedPDU(DaveDtSrc *DtSrc, DavePDU * PDU)
{
    int32_t res; /* = daveResCannotEvaluatePDU; */
    PDU->header = DtSrc->MsgIn + DtSrc->PDUStartI;
    res = 0;
    if ( PDU->header[1] == 2 || PDU->header[1] == 3 )
    {
        PDU->hlen = 12;
        res = 256 * PDU->header[10] + PDU->header[11];
    }
    else
    {
        PDU->hlen = 10;
    }

    PDU->param = PDU->header + PDU->hlen;
    PDU->plen = 256 * PDU->header[6] + PDU->header[7];
    PDU->data = PDU->param + PDU->plen;
    PDU->dlen = 256 * PDU->header[8] + PDU->header[9];
    PDU->udlen = 0;
    PDU->udata = NULL;

    LogExec((daveDebugSwitch & daveDebugPDU),
            _daveDumpHexPDU(PDU));

    return res;
}

/*
build the PDU for a PDU length negotiation
*/
int32_t DAVE_DECL _daveNegPDUlengthRequest(DaveDtSrc *DtSrc, DavePDU *PDU)
{
    uint8_t pa[ ] = { 0xF0, 0, 0, 1, 0, 1,
        DtSrc->PDUMaxLen / 0x100, //3, 		
        DtSrc->PDUMaxLen % 0x100, //0xC0,
    };
    
    PDU->header = DtSrc->MsgOut + DtSrc->PDUStartO;

    _daveInitPDUheader(PDU, 1);

    _daveAddPDUParam(PDU, pa, sizeof(pa));

    LogExec((daveDebugSwitch & daveDebugPDU), 
            _daveDumpHexPDU(PDU));

    int32_t res = _daveSwap(DtSrc, PDU);
    if ( daveResOK == res )
    {
        DavePDU tmp;
        res = _daveSetupReceivedPDU(DtSrc, &tmp);
        if ( daveResOK == res )
        {
            int16_t CpuPduLimit = daveGetTwoBytefrom(tmp.param + 6);
            if ( DtSrc->PDUMaxLen > CpuPduLimit )
            {
                DtSrc->PDUMaxLen = CpuPduLimit; // use lower number as limit
            }
            LogExec((daveDebugSwitch & daveDebugConnect),
                    LogPut("\n*** Partner offered PDU length: %d used limit %d\n\n", CpuPduLimit, DtSrc->PDUMaxLen));
        }
    }
    return res;
}

int32_t DAVE_DECL _daveCnctPLCInTCP(DaveDtSrc *DtSrc)
{
    int32_t res;

    uint8_t b4[ ] = { // default
        0x11, 
        0xE0, 
        0x00, 0x00, 
        0x00, 0x01, 
        0x00,
        0xC1, 
        2, 
        1, 
        0,
        0xC2, 
        2,
        DtSrc->CommType,
        (DtSrc->Slot | DtSrc->Rack << 5), // hope I got it right this time...
        0xC0, 
        1, 
        0x9,
    };

    uint8_t b4R2[ ] = {	// for routing
        6 + 30 + 30 + 3,// Length over all without this byte (6 byte fixed data, 30 bytes source TSAP (C1), 30 bytes dest TSAP (C2), 3 bytes TPDU size (C0))

        0xE0,		    // TDPU Type CR = Connection Request (see RFC1006/ISO8073)
        0x00, 0x00,	    // TPDU Destination Reference (unknown)
        0x00, 0x01,	    // TPDU Source-Reference (my own reference, should not be zero)

        0x00,		    // TPDU Class 0 and no Option

        0xC1,		    // Parameter Source-TSAP
        28,		        // Length of this parameter
        1,		        // one block of data (???)
        0,		        // Length for S7-Subnet-ID
        0,		        // Length of PLC-Number
        2,		        // Length of Function/Rack/Slot
        0, 0, 0, 0, 0, 0, 0, 0,	// empty Data 
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        1,		        // Function (1=PG,2=OP,3=Step7Basic)
        0,		        // Rack (Bit 7-5) and Slot (Bit 4-0)

        0xC2,	        // Parameter Destination-TSAP
        28,		        // Length of this parameter 
        1,		        // one block of data (???)

        6,		        // Length for S7-Subnet-ID
        1,		        // Length of PLC address
        2,		        // Length of Function/Rack/Slot

        0x01, 0x52,		// first part of S7-Subnet-ID  (look into the S7Project/Network configuration)
        0x00, 0x00,		// fix always 0000 (reserved for later use ?)
        0x00, 0x13,		// second part of S7-Subnet-ID 
        // (see S7Project/Network configuration)

        0x01,			// PLC address

        0, 0, 0, 0, 0, 0, 0, 0,	// empty 
        0, 0, 0, 0, 0, 0, 0,

        DtSrc->CommType,// Function (1=PG,2=OP,3=Step7Basic)
        (DtSrc->Slot | DtSrc->Rack << 5),	// Rack (Bit 7-5) and Slot (Bit 4-0) hope I got it right this time...

        0xC0,		    // Parameter requested TPDU-Size
        1,		        // Length of this parameter 
        0x9,		    // requested TPDU-Size 8=256 Bytes, 9=512, a=1024 Bytes 
    };

    uint8_t b243[ ] = {
        0x11, 0xE0, 0x00,
        0x00, 0x00, 0x01, 0x00,
        0xC1, 2, 'M', 'W',
        0xC2, 2, 'M', 'W',
        0xC0, 1, 9,
    };

    int32_t success = 0, retries = 0;

    DtSrc->PartPos = 0;

    if ( DtSrc->Protocol == daveProtoISOTCP243 )
    {
        memcpy(DtSrc->MsgOut + 4, b243, sizeof(b243));
    }
    else if ( DtSrc->Protocol == daveProtoISOTCP )
    {
        if ( !DtSrc->Routing )
        {
            memcpy(DtSrc->MsgOut + 4, b4, sizeof(b4));
            DtSrc->MsgOut[17] = DtSrc->CommType;
            DtSrc->MsgOut[18] = DtSrc->Slot | DtSrc->Rack << 5; // hope I got it right this time...
        }
        else
        {
            LogExec((daveDebugSwitch & daveDebugConnect),
                    _daveDumpHex("routing data 1: ", (uint8_t *)&(DtSrc->RoutingData), 30));

            b4R2[41] = DtSrc->RoutingData.PLCadrsize;

            int32_t px = 43;

            b4R2[px] = (DtSrc->RoutingData.SubnetID1) / 0x100;
            b4R2[px + 1] = (DtSrc->RoutingData.SubnetID1) % 0x100;
            b4R2[px + 2] = (DtSrc->RoutingData.SubnetID2) / 0x100;
            b4R2[px + 3] = (DtSrc->RoutingData.SubnetID2) % 0x100;
            b4R2[px + 4] = (DtSrc->RoutingData.SubnetID3) / 0x100;
            b4R2[px + 5] = (DtSrc->RoutingData.SubnetID3) % 0x100;

            memcpy(b4R2 + 49, DtSrc->RoutingData.PLCadr, DtSrc->RoutingData.PLCadrsize);

            memcpy(DtSrc->MsgOut + 4, b4R2, sizeof(b4R2));	// with routing over MPI

            // DtSrc->MsgOut[17] = DtSrc->Rack+1;			    // this is probably wrong
            // DtSrc->MsgOut[18] = DtSrc->Slot;
        }
    }

    _daveSendISOPacket(DtSrc, DtSrc->MsgOut[4] + 1);
    do
    {
        res = _daveReadISOPacket(DtSrc);
        LogExec((daveDebugSwitch & daveDebugConnect),
                (LogPut("%s daveCnctPLC() step 1. ", DtSrc->Name),
                _daveDumpHex("got packet: ", DtSrc->MsgIn, res)));

        if ( (res == 22 && !DtSrc->Routing) || (res == 48 && DtSrc->Routing) || (res == 74 && DtSrc->Routing) )
        {
            success = 1;
            int32_t i;
            for ( i = 6; i < res; i++ )
            {
                if ( DtSrc->MsgIn[i] == 0xc0 )
                {
                    DtSrc->TPDUsize = 128 << (DtSrc->MsgIn[i + 2] - 7);
                    LogExec((daveDebugSwitch & daveDebugConnect),
                            LogPut("TPDU len %d = %d\n", DtSrc->MsgIn[i + 2], DtSrc->TPDUsize));
                }
            }
        }
        else
        {
            LogExec((daveDebugSwitch & daveDebugPrintErrors),
                    LogPut("%s error in daveCnctPLC() step 1. retrying...", DtSrc->Name));
        }
        retries++;
    } while ( (success == 0) && (retries < 3) );

    if ( 0 != success )
    {
        retries = 0;
        do
        {
            DavePDU tmp;
            res = _daveNegPDUlengthRequest(DtSrc, &tmp);
            if ( res == 0 )
            {
                return res;
            }
            else
            {
                LogExec((daveDebugSwitch & daveDebugPrintErrors),
                        LogPut("%s error in daveCnctPLC() step 1. retrying...\n", DtSrc->Name));
            }
            retries++;
        } while ( retries < 3 );
    }
    return !daveResOK;
}

int32_t DAVE_DECL _daveStartS7(DaveDtSrc *DtSrc)
{
    uint8_t paMakeRun[ ] = {
        0x28, 0, 0, 0, 0, 0, 0, 0xFD, 0, 0x00, 9, 'P', '_', 'P', 'R', 'O', 'G', 'R', 'A', 'M'
    };
    int32_t res;
    DavePDU pdu1, pdu2;
    pdu1.header = DtSrc->MsgOut + DtSrc->PDUStartO;
    _daveInitPDUheader(&pdu1, 1);
    _daveAddPDUParam(&pdu1, paMakeRun, sizeof(paMakeRun));
    res = _daveSwap(DtSrc, &pdu1);
    if ( daveResOK == res )
    {
        res = _daveSetupReceivedPDU(DtSrc, &pdu2);
        LogExec((daveDebugSwitch & daveDebugPDU),
                _daveDumpHexPDU(&pdu2));
    }
    return res;
}

int32_t DAVE_DECL _daveStopS7(DaveDtSrc *DtSrc)
{
    uint8_t paMakeStop[ ] = {
        0x29, 0, 0, 0, 0, 0, 9, 'P', '_', 'P', 'R', 'O', 'G', 'R', 'A', 'M'
    };
    int32_t res;
    DavePDU pdu1, pdu2;
    pdu1.header = DtSrc->MsgOut + DtSrc->PDUStartO;
    _daveInitPDUheader(&pdu1, 1);
    _daveAddPDUParam(&pdu1, paMakeStop, sizeof(paMakeStop));
    res = _daveSwap(DtSrc, &pdu1);
    if ( daveResOK == res )
    {
        res = _daveSetupReceivedPDU(DtSrc, &pdu2);
        LogExec((daveDebugSwitch & daveDebugPDU), 
                _daveDumpHexPDU(&pdu2));
    }
    return res;
}

void DAVE_DECL davePrepareReadRequest(DaveDtSrc *DtSrc, DavePDU *PDU)
{
    LogExec((daveDebugSwitch & daveDebugPDU), 
        LogPut("davePrepareReadRequest(dc:%p PDU:%p)\n", DtSrc, PDU));
    uint8_t pa[ ] = { daveFuncRead, 0 };
    PDU->header = DtSrc->MsgOut + DtSrc->PDUStartO;
    _daveInitPDUheader(PDU, 1);
    _daveAddPDUParam(PDU, pa, sizeof(pa));
}

int32_t DAVE_DECL _daveTryResultData(DavePDU * PDU)
{
    int32_t res; /*=daveResCannotEvaluatePDU;*/
    if ( (PDU->data[0] == 255) && (PDU->dlen > 4) )
    {
        res = daveResOK;
        PDU->udata = PDU->data + 4;
        PDU->udlen = PDU->data[2] * 0x100 + PDU->data[3];
        if ( PDU->data[1] == 4 )
        {
            PDU->udlen >>= 3;	/* len is in bits, adjust */
        }
        else if ( PDU->data[1] == 9 )
        {
            /* len is already in bytes, ok */
        }
        else if ( PDU->data[1] == 3 )
        {
            /* len is in bits, but there is a byte per result bit, ok */
        }
        else
        {
            LogExec((daveDebugSwitch & daveDebugPDU), 
                    LogPut("fixme: what to do with data type %d?\n", PDU->data[1]));
            res = daveResUnknownDataUnitSize;
        }
    }
    else
    {
        res = PDU->data[0];
    }
    return res;
}

int32_t DAVE_DECL _daveTryReadResult(DavePDU * PDU)
{
    return (PDU->param[0] != daveFuncRead) ? daveResUnexpectedFunc : _daveTryResultData(PDU);
}

/*
    因为缓冲区不足的情况下无法获取结果数据的时候可通过该函数获取上一次成功操作的数据，返回非零表示数据长度需求
*/
int32_t DAVE_DECL _daveGetLastRecvData(DaveDtSrc *DtSrc, uint8_t *buf, int32_t len)
{

    LogExec((daveDebugSwitch & daveDebugUpload),
            _daveDumpHex("Last Upload Data :\n", DtSrc->resultPointer, DtSrc->resultLen));
    if ( DtSrc->resultLen > len )
    {
        return DtSrc->resultLen;
    }
    else
    {
        memcpy(buf, DtSrc->resultPointer, DtSrc->resultLen);
        return daveResOK;
    }
}

/*
    Read len bytes from PLC memory area "area", data block DBnum.
    Return the Number of bytes read.
    If a buffer pointer is provided, data will be copied into this buffer.
    If it's NULL you can get your data from the resultPointer in daveConnection long
    as you do not send further requests.
*/
int32_t DAVE_DECL _daveReadS7Bytes(DaveDtSrc *DtSrc, int32_t area, int32_t DBnum, int32_t start, int32_t len, int32_t isBit, void * buffer)
{
    DavePDU pdu1, pdu2;
    int32_t res;
    DtSrc->resultLen = 0;	// 03/12/05
    DtSrc->resultPointer = NULL;
    pdu1.header = DtSrc->MsgOut + DtSrc->PDUStartO;
    davePrepareReadRequest(DtSrc, &pdu1);
    LogExec((daveDebugSwitch & daveDebugPDU), 
            LogPut("daveAddVarToReadRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d)\n",
            &pdu1, daveAreaName(area), DBnum, start, len));
    daveAddPDUVarToReadRequest(&pdu1, area, DBnum, start, len, isBit);
		res = _daveSwap(DtSrc, &pdu1);
    if ( daveResOK == res )
    {
        res = _daveSetupReceivedPDU(DtSrc, &pdu2);
        LogExec((daveDebugSwitch & daveDebugPDU),
                LogPut("_daveSetupReceivedPDU() returned: %d=%s\n", res, daveStrerror(res)));
        if ( daveResOK == res )
        {
            res = _daveTryReadResult(&pdu2);
            LogExec((daveDebugSwitch & daveDebugPDU),
                    LogPut("_daveTryReadResult() returned: %d=%s\n", res, daveStrerror(res)));
            if ( daveResOK == res )
            {
                if ( pdu2.udlen != 0 )
                {
                    LogExec((daveDebugSwitch & daveDebugPDU),
                            _daveDumpHex("PLC Upload Data :\n", pdu2.udata, pdu2.udlen));
                    DtSrc->resultPointer = pdu2.udata;
                    DtSrc->resultLen = pdu2.udlen;
                    /*
                    copy to user buffer and setup internal buffer pointers:
                    */
                    if ( buffer != NULL && len >= pdu2.udlen)
                    {
                        memcpy(buffer, pdu2.udata, pdu2.udlen);
                        return daveResOK;
                    }
                    else
                    {
                        return daveResOKButMemoryOut;
                    }
                }
                else
                {
                    return daveResCPUNoData;
                }
            }
        }
    }
    return res;
}

void DAVE_DECL davePrepareWriteRequest(DaveDtSrc * DtSrc, DavePDU *PDU)
{
    LogExec((daveDebugSwitch & daveDebugRawWrite),
        LogPut("davePrepareWriteRequest(dc:%p PDU:%p)\n", DtSrc, PDU));
    uint8_t pa[ ] = { daveFuncWrite, 0 };
    PDU->header = DtSrc->MsgOut + DtSrc->PDUStartO;
    _daveInitPDUheader(PDU, 1);
    _daveAddPDUParam(PDU, pa, sizeof(pa));
    PDU->dlen = 0;
}

void DAVE_DECL daveAddToWriteRequest(DavePDU *PDU, int32_t area, int32_t DBnum, int32_t start, int32_t byteCount, void * buffer, uint8_t * da, int32_t dasize, uint8_t * pa, int32_t pasize)
{
    LogExec((daveDebugSwitch & daveDebugRawWrite),
        LogPut("daveAddToWriteRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
           PDU, daveAreaName(area), DBnum, start, byteCount, buffer));

    LogExec((daveDebugSwitch & daveDebugPDU), 
            _daveDumpHexPDU(PDU));

    if ( (area == daveTimer) || (area == daveCounter) || (area == daveTimer200) || (area == daveCounter200) )
    {
        pa[3] = area;
        pa[4] = ((byteCount + 1) / 2) / 0x100;
        pa[5] = ((byteCount + 1) / 2) & 0xff;
    }
    else if ( (area == daveAnaIn) || (area == daveAnaOut) )
    {
        pa[3] = 4;
        pa[4] = ((byteCount + 1) / 2) / 0x100;
        pa[5] = ((byteCount + 1) / 2) & 0xff;
    }
    else
    {
        pa[4] = byteCount / 0x100;
        pa[5] = byteCount & 0xff;
    }
    pa[6] = DBnum / 256;
    pa[7] = DBnum & 0xff;
    pa[8] = area;
    pa[11] = start & 0xff;
    pa[10] = (start / 0x100) & 0xff;
    pa[9] = start / 0x10000;
    if ( PDU->dlen % 2 )
    {
        _daveAddPDUData(PDU, da, 1);
    }
    PDU->param[1]++;

    uint8_t saveData[1024];

    if ( PDU->dlen )
    {
        memcpy(saveData, PDU->data, PDU->dlen);
        memcpy(PDU->data + pasize, saveData, PDU->dlen);
    }
    memcpy(PDU->param + PDU->plen, pa, pasize);

    PDU->plen += pasize;

    ((DavePDUHeader2*)PDU->header)->plenHi = PDU->plen / 256;
    ((DavePDUHeader2*)PDU->header)->plenLo = PDU->plen % 256;

    PDU->data = PDU->param + PDU->plen;
    _daveAddPDUData(PDU, da, dasize);
    _daveAddPDUValue(PDU, buffer, byteCount);
    LogExec((daveDebugSwitch & daveDebugPDU), 
            _daveDumpHexPDU(PDU));

}
void DAVE_DECL daveAddPDUVarToWriteRequest(DavePDU *PDU, int32_t area, int32_t DBnum, int32_t start, int32_t byteCount, void * buffer)
{
    uint8_t da[ ] = { 0, 4, 0, 0, };
    uint8_t pa[ ] = {
        0x12, 0x0a, 0x10,
        0x02,		/* unit (for count?, for consistency?) byte */
        0, 0,		/* length in bytes */
        0, 0,		/* DB number */
        0,		/* area code */
        0, 0, 0		/* start address in bits */
    };

    LogExec((daveDebugSwitch & daveDebugPDU),
        LogPut("daveAddVarToWriteRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
        PDU, daveAreaName(area), DBnum, start, byteCount, buffer));

    daveAddToWriteRequest(PDU, area, DBnum, 8 * start, byteCount, buffer, da, sizeof(da), pa, sizeof(pa));
}

int32_t DAVE_DECL _daveTryWriteResult(DavePDU * PDU)
{
    int32_t res;/* =daveResCannotEvaluatePDU; */

    if ( PDU->param[0] != daveFuncWrite )
    {
        res = daveResUnexpectedFunc;
    }
    else
    {
        if ( (PDU->data[0] == 0xFF) )
        {
            res = daveResOK;
        }
        else
        {
            res = PDU->data[0];
        }
    }
    LogExec((daveDebugSwitch & daveDebugPDU), _daveDumpHexPDU(PDU));
    return res;
}

int32_t DAVE_DECL _daveWriteS7Bytes(DaveDtSrc * DtSrc, int32_t area, int32_t DB, int32_t start, int32_t len, void * buffer)
{
    DavePDU pdu1, pdu2;
    int32_t res;
    pdu1.header = DtSrc->MsgOut + DtSrc->PDUStartO;
    davePrepareWriteRequest(DtSrc, &pdu1);
    daveAddPDUVarToWriteRequest(&pdu1, area, DB, start, len, buffer);
    res = _daveSwap(DtSrc, &pdu1);
    if ( daveResOK == res )
    {
        res = _daveSetupReceivedPDU(DtSrc, &pdu2);
        if ( daveResOK == res )
        {
            res = _daveTryWriteResult(&pdu2);
        }
    }
    return res;
}
