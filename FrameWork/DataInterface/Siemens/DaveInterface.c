
#include "DaveInterface.h"

#include "DaveDataSource.h"

int32_t daveDebugSwitch = daveDebugOFF;

struct dave_interface
{
    DaveDtSrc *DtSrc;
    _DaveGetResponse GetResponse;
    _DaveGetLastRecvData GetLastRecvData;
    _DaveCnctPLC CnctPLC;
    _DaveStartPLC StartPLC;
    _DaveStopPLC StopPLC;
    _DaveReadBytes ReadBytes;
    _DaveWriteBytes WriteBytes;
};

void DaveDel(DaveFree Free, void * Self)
{
    Free(Self);
}

DaveIntfc * DaveNewIntfc(DaveAlloc Alloc, DaveFree Free, DaveDtSrc *DtSrc)
{
    DaveIntfc * Self = (DaveIntfc *)Alloc(sizeof(*Self));
    if ( Self )
    {
        memset(Self, 0, sizeof(*Self));
        if ( DtSrc->File )
        {
            Self->GetLastRecvData = _daveGetLastRecvData;
            switch ( DtSrc->Protocol )
            {
                case daveProtoISOTCP:
                case daveProtoISOTCP243:
                    Self->GetResponse = _daveGetResponseISOInTCP;
                    Self->CnctPLC = _daveCnctPLCInTCP;
                    Self->StartPLC = _daveStartS7;
                    Self->StopPLC = _daveStopS7;
                    Self->ReadBytes = _daveReadS7Bytes;
                    Self->WriteBytes = _daveWriteS7Bytes;
                    DtSrc->SwapData = _daveSwapTCP;
                    Self->DtSrc = DtSrc;
                    break;
                default:
                    Self->DtSrc = NULL;
                    break;
            }
            return Self;
        }
        DaveDel(Free, Self), Self = NULL;
    }
    return Self;
}

int32_t DaveIntfcCnctPLC(DaveIntfc * Intfc)
{
    return Intfc->CnctPLC(Intfc->DtSrc);
}

int32_t DaveIntfcStartPLC(DaveIntfc * Intfc)
{
    return Intfc->StartPLC(Intfc->DtSrc);
}

int32_t DaveIntfcStopPLC(DaveIntfc * Intfc)
{
    return Intfc->StopPLC(Intfc->DtSrc);
}

uint8_t DaveSwapIedOneByte(uint8_t Src)
{
    return daveGetOneBytefrom(&Src);
}

uint16_t DaveSwapIedTwoByte(uint16_t Src)
{
    return daveGetTwoBytefrom(&Src);
}

uint32_t DaveSwapIedFourByte(uint32_t Src)
{
    return daveGetFourBytefrom(&Src);
}

void DaveIntfcDumpHex(const char *name, uint8_t *buf, int32_t len)
{
    _daveDumpHex(name, buf, len);
}

int32_t DaveIntfcGetLastRecvData(DaveIntfc *Intfc, uint8_t * buf, int32_t len)
{
    return Intfc->GetLastRecvData(Intfc->DtSrc, buf, len);
}

int32_t DaveIntfcReadBytes(DaveIntfc * Intfc, int32_t area, int32_t DBnum, int32_t start, int32_t len, int32_t isBit, void * buffer)
{
    return Intfc->ReadBytes(Intfc->DtSrc, area, DBnum, start, len, isBit, buffer);
}

int32_t DaveIntfcWriteBytes(DaveIntfc * Intfc, int32_t area, int32_t DB, int32_t start, int32_t len, void * buffer)
{
    return Intfc->WriteBytes(Intfc->DtSrc, area, DB, start, len, buffer);
}
