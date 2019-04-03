
#include "DavePDU.h"

extern int32_t daveDebugSwitch;

/*
    Get the eror code from a PDU, if one.
    */
int16_t DAVE_DECL daveGetPDUerror(DavePDU * PDU)
{
    LogPut("daveGetPDUerror(PDU:%p\n", PDU);
    if ( PDU->header[1] == 2 || PDU->header[1] == 3 )
    {
        return daveGetTwoBytefrom(PDU->header + 10);
    }
    else
    {
        return 0;
    }
}

void DAVE_DECL _daveDumpHexPDU(DavePDU * PDU)
{
    int i, dl;
    uint8_t * pd;
    _daveDumpHex("PDU header", PDU->header, PDU->hlen);
    LogPut("plen: %d dlen: %d\n", PDU->plen, PDU->dlen);
    if ( PDU->plen > 0 )
    {
        _daveDumpHex("Parameter", PDU->param, PDU->plen);
    }
    if ( PDU->dlen > 0 )
    {
        _daveDumpHex("Data     ", PDU->data, PDU->dlen);
    }
    if ( (PDU->plen == 2) && (PDU->param[0] == daveFuncRead) )
    {
        pd = PDU->data;
        for ( i = 0; i < PDU->param[1]; i++ )
        {
            _daveDumpHex("Data hdr ", pd, 4);

            dl = 0x100 * pd[2] + pd[3];
            if ( pd[1] == 4 ) dl /= 8;
            pd += 4;
            _daveDumpHex("Data     ", pd, dl);
            if ( i < PDU->param[1] - 1 ) dl = dl + (dl % 2);  	// the PLC places extra bytes at the end of all 
            // but last result, if length is not a multiple 
            // of 2
            pd += dl;
        }
    }
    else if ( (PDU->header[1] == 1) &&/*(PDU->plen==2)&&*/(PDU->param[0] == daveFuncWrite) )
    {
        pd = PDU->data;
        for ( i = 0; i < PDU->param[1]; i++ )
        {
            _daveDumpHex("Write Data hdr ", pd, 4);
            dl = 0x100 * pd[2] + pd[3];
            if ( pd[1] == 4 ) dl /= 8;
            pd += 4;
            _daveDumpHex("Data     ", pd, dl);
            if ( i < PDU->param[1] - 1 ) dl = dl + (dl % 2);  	// the PLC places extra bytes at the end of all 
            // but last result, if length is not a multiple 
            // of 2
            pd += dl;
        }
    }
    else
    {
        if ( PDU->dlen>0 )
        {
            if ( PDU->udlen == 0 )
                _daveDumpHex("Data     ", PDU->data, PDU->dlen);
            else
                _daveDumpHex("Data hdr ", PDU->data, 4);
        }
        if ( PDU->udlen > 0 )
        {
            _daveDumpHex("result Data ", PDU->udata, PDU->udlen);
        }
    }
    if ( (PDU->header[1] == 2) || (PDU->header[1] == 3) )
    {
        LogPut("error: %s\n", daveStrerror(daveGetPDUerror(PDU)));
    }
}

/*
    set up the header. Needs valid header pointer
    */
void DAVE_DECL _daveInitPDUheader(DavePDU * PDU, int32_t type)
{
    memset(PDU->header, 0, sizeof(PDUHeader));
    PDU->hlen = (type == 2 || type == 3) ? 12 : 10;
    PDU->param = PDU->header + PDU->hlen;
    ((PDUHeader*)PDU->header)->P = 0x32;
    ((PDUHeader*)PDU->header)->type = type;
    PDU->dlen = 0;
    PDU->plen = 0;
    PDU->udlen = 0;
    PDU->data = NULL;
    PDU->udata = NULL;
}

/*
    add parameters after header, adjust pointer to data.
    needs valid header
    */
void DAVE_DECL _daveAddPDUParam(DavePDU * PDU, uint8_t * Param, uint16_t Len)
{
    LogExec((daveDebugSwitch & daveDebugPDU),
            LogPut("_daveAddPDUParam(PDU:%p, param %p, len:%d)\n", PDU, Param, Len));
    PDU->plen = Len;
    memcpy(PDU->param, Param, Len);
    ((DavePDUHeader2*)PDU->header)->plenHi = Len / 256;
    ((DavePDUHeader2*)PDU->header)->plenLo = Len % 256;
    // ((PDUHeader*)PDU->header)->plen = daveSwapIed_16(Len);
    PDU->data = PDU->param + Len;
    PDU->dlen = 0;
}

/*
    add data after parameters, set dlen
    needs valid header,parameters
*/
void DAVE_DECL _daveAddPDUData(DavePDU * PDU, void * data, int32_t len)
{
    LogExec((daveDebugSwitch & daveDebugPDU), 
            (LogPut("_daveAddData(PDU:%p, data %p, len:%d)\n", PDU, data, len),
            _daveDumpHexPDU(PDU)));
    uint8_t * dn = PDU->data + PDU->dlen;
    PDU->dlen += len;
    memcpy(dn, data, len);
    ((DavePDUHeader2*)PDU->header)->dlenHi = PDU->dlen / 256;
    ((DavePDUHeader2*)PDU->header)->dlenLo = PDU->dlen % 256;
    //    ((PDUHeader*)p->header)->dlen=daveSwapIed_16(p->dlen);
}

/*
add values after value header in data, adjust dlen and data count.
needs valid header,parameters,data,dlen
*/
void DAVE_DECL _daveAddPDUValue(DavePDU * PDU, void * data, int len)
{
    uint16_t dCount;
    uint8_t * dtype;

    LogExec((daveDebugSwitch & daveDebugPDU),
            (LogPut("_daveAddValue(PDU:%p, data %p, len:%d)\n", PDU, data, len),
            _daveDumpHexPDU(PDU)));

    dtype = PDU->data + PDU->dlen - 4 + 1;			/* position of first byte in the 4 byte sequence */

    dCount = PDU->data[PDU->dlen - 4 + 2 + 1];
    dCount += 256 * PDU->data[PDU->dlen - 4 + 2];

    LogExec((daveDebugSwitch & daveDebugPDU),
            LogPut("dCount: %d\n", dCount));

    if ( *dtype == 4 )
    {	/* bit data, length is in bits */
        dCount += 8 * len;
    }
    else if ( *dtype == 9 )
    {	/* byte data, length is in bytes */
        dCount += len;
    }
    else if ( *dtype == 3 )
    {	/* bit data, length is in bits */
        dCount += len;
    }
    else
    {
        LogExec((daveDebugSwitch & daveDebugPDU),
                LogPut("unknown data type/length: %d\n", *dtype));
    }
    if ( PDU->udata == NULL )
    {
        PDU->udata = PDU->data + 4;
    }

    PDU->udlen += len;

    LogExec((daveDebugSwitch & daveDebugPDU), LogPut("dCount: %d\n", dCount));

    PDU->data[PDU->dlen - 4 + 2] = dCount / 256;
    PDU->data[PDU->dlen - 4 + 2 + 1] = dCount % 256;

    _daveAddPDUData(PDU, data, len);
}

void DAVE_DECL daveAddPDUVarToReadRequest(DavePDU *PDU, int32_t area, int32_t DBnum, int32_t start, int32_t byteCount, int32_t isBit)
{
    uint8_t pa[ ] = {
        0x12, 0x0a, 0x10,
        0x02,		/* 1=single bit, 2=byte, 4=word */
        0, 0,		/* length in bytes */
        0, 0,		/* DB number */
        0,		    /* area code */
        0, 0, 0		/* start address in bits */
    };

    if ( (area == daveAnaIn) || (area == daveAnaOut) /*|| (area==daveP)*/ )
    {
        pa[3] = 4;
        start *= 8;			/* bits */
    }
    else if ( (area == daveTimer) || (area == daveCounter) || (area == daveTimer200) || (area == daveCounter200) )
    {
        pa[3] = area;
    }
    else
    {
        if ( isBit )
        {
            pa[3] = 1;
        }
        else
        {
            start *= 8;			/* bit address of byte */
        }
    }

    pa[4] = byteCount / 256;
    pa[5] = byteCount & 0xff;
    pa[6] = DBnum / 256;
    pa[7] = DBnum & 0xff;
    pa[8] = area;
    pa[11] = start & 0xff;
    pa[10] = (start / 0x100) & 0xff;
    pa[9] = start / 0x10000;

    PDU->param[1]++;
    memcpy(PDU->param + PDU->plen, pa, sizeof(pa));
    PDU->plen += sizeof(pa);

    ((DavePDUHeader2*)PDU->header)->plenHi = PDU->plen / 256;
    ((DavePDUHeader2*)PDU->header)->plenLo = PDU->plen % 256;

    PDU->data = PDU->param + PDU->plen;
    PDU->dlen = 0;

    LogExec((daveDebugSwitch & daveDebugPDU),
            _daveDumpHexPDU(PDU));
}

