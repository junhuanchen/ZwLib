
#include "Dave.h"

uint8_t DAVE_DECL daveGetOneBytefrom(void *buffer)
{
    uint8_t *buf = (uint8_t *)buffer;
    return (*buf << 4 & *buf >> 4);
}

uint16_t DAVE_DECL daveGetTwoBytefrom(void *buffer)
{
    uint8_t *buf = (uint8_t *)buffer;
    union
    {
        uint16_t res;
        uint8_t buf[sizeof(uint16_t)];
    } obj;
#ifdef DAVE_LITTLE_ENDIAN    
    obj.buf[1] = *buf, buf++, obj.buf[0] = *buf;
#else
    obj.buf[0] = *buf, buf++, obj.buf[1] = *buf;
#endif    
    return obj.res;
}

uint32_t DAVE_DECL daveGetFourBytefrom(void *buffer)
{
    uint8_t *buf = (uint8_t *)buffer;
    union
    {
        uint32_t res;
        uint8_t buf[sizeof(uint32_t)];
    } obj;
#ifdef DAVE_LITTLE_ENDIAN
    obj.buf[3] = *buf, buf++, obj.buf[2] = *buf, buf++;
    obj.buf[1] = *buf, buf++, obj.buf[0] = *buf;
#else
    obj.buf[0] = *buf, buf++, obj.buf[1] = *buf, buf++;
    obj.buf[2] = *buf, buf++, obj.buf[3] = *buf;
#endif
    return obj.res;
}

/*
    error code to message string conversion:
*/
char * DAVE_DECL daveStrerror(int32_t code)
{
    switch ( code )
    {
        case daveResOK: return "ok";
        case daveResMultipleBitsNotSupported:return "the CPU does not support reading a bit block of length<>1";
        case daveResItemNotAvailable: return "the desired item is not available in the PLC";
        case daveResItemNotAvailable200: return "the desired item is not available in the PLC (200 family)";
        case daveAddressOutOfRange: return "the desired address is beyond limit for this PLC";
        case daveResCPUNoData: return "the PLC returned a packet with no result data";
        case daveUnknownError: return "the PLC returned an error code not understood by this library";
        case daveEmptyResultError: return "this result contains no data";
        case daveEmptyResultSetError: return "cannot work with an undefined result set";
        case daveResCannotEvaluatePDU: return "cannot evaluate the received PDU";
        case daveWriteDataSizeMismatch: return "Write data size error";
        case daveResNoPeripheralAtAddress: return "No data from I/O module";
        case daveResUnexpectedFunc: return "Unexpected function code in answer";
        case daveResUnknownDataUnitSize: return "PLC responds with an unknown data type";

        case daveResShortPacket: return "Short packet from PLC";
        case daveResTimeout: return "Timeout when waiting for PLC response";
        case daveResNoBuffer: return "No buffer provided";
        case daveNotAvailableInS5: return "Function not supported for S5";

        case 0x8000: return "function already occupied.";
        case 0x8001: return "not allowed in current operating status.";
        case 0x8101: return "hardware fault.";
        case 0x8103: return "object access not allowed.";
        case 0x8104: return "context is not supported. Step7 says:Function not implemented or error in telgram.";
        case 0x8105: return "invalid address.";
        case 0x8106: return "data type not supported.";
        case 0x8107: return "data type not consistent.";
        case 0x810A: return "object does not exist.";
        case 0x8301: return "insufficient CPU memory ?";
        case 0x8402: return "CPU already in RUN or already in STOP ?";
        case 0x8404: return "severe error ?";
        case 0x8500: return "incorrect PDU size.";
        case 0x8702: return "address invalid.";;
        case 0xd002: return "Step7:variant of command is illegal.";
        case 0xd004: return "Step7:status for this command is illegal.";
        case 0xd0A1: return "Step7:function is not allowed in the current protection level.";
        case 0xd201: return "block name syntax error.";
        case 0xd202: return "syntax error function parameter.";
        case 0xd203: return "syntax error block type.";
        case 0xd204: return "no linked block in storage medium.";
        case 0xd205: return "object already exists.";
        case 0xd206: return "object already exists.";
        case 0xd207: return "block exists in EPROM.";
        case 0xd209: return "block does not exist/could not be found.";
        case 0xd20e: return "no block present.";
        case 0xd210: return "block number too big.";
            //	case 0xd240: return "unfinished block transfer in progress?";  // my guess
        case 0xd240: return "Coordination rules were violated.";
            /*  Multiple functions tried to manipulate the same object.
            Example: a block could not be copied,because it is already present in the target system
            and
            */
        case 0xd241: return "Operation not permitted in current protection level.";
            /**/	case 0xd242: return "protection violation while processing F-blocks. F-blocks can only be processed after password input.";
            case 0xd401: return "invalid SZL ID.";
            case 0xd402: return "invalid SZL index.";
            case 0xd406: return "diagnosis: info not available.";
            case 0xd409: return "diagnosis: DP error.";
            case 0xdc01: return "invalid BCD code or Invalid time format?";
            default: return "no message defined!";
    }
}

char * DAVE_DECL daveAreaName(uint8_t n)
{
    LogPut("daveAreaName(n:%d)\n", n);
    switch ( n )
    {
        case daveSysInfo:	return "System info mem.area of 200 family";
        case daveSysFlags:	return "System flags of 200 family";
        case daveAnaIn:		return "analog inputs of 200 family";
        case daveAnaOut:	return "analog outputs of 200 family";

        case daveP:		return "Peripheral I/O";
        case daveInputs:	return "Inputs";
        case daveOutputs:	return "Outputs";
        case daveDB:		return "DB";
        case daveDI:		return "DI (instance data)";
        case daveFlags:		return "Flags";
        case daveLocal:		return "local data";
        case daveV:		return "caller's local data";
        case daveCounter:	return "S7 counters";
        case daveTimer:		return "S7 timers";
        case daveCounter200:	return "IEC counters";
        case daveTimer200:	return "IEC timers";
        default:return "unknown area!";
    }
}

/*
    Hex dump:
*/
#define DUMPLIMIT 11132
void DAVE_DECL _daveDumpHex(const char *name, uint8_t *buf, int32_t len)
{
    LogPut("%s:\t", name);
    if ( len > daveMaxRawLen )
    {
        len = daveMaxRawLen; 	/* this will avoid to dump zillions of chars */
    }
    if ( len > DUMPLIMIT )
    {
        len = DUMPLIMIT; 		/* this will avoid large dumps */
    }

    for ( int32_t i = 0; i < len; i++ )
    {
        if ( (i & 0xf) == 0 )
        {
            LogPut("\n\t%x:", i);
        }
        LogPut("0x%02X, ", ((uint8_t*)(buf))[i]);
    }
    LogPut("\n");
}
