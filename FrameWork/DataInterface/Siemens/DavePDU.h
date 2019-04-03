#ifndef DAVE_PDU_H
#define DAVE_PDU_H

#include "Dave.h"

/*
    Helper struct to manage PDUs. Self is NOT the part of the packet I would call PDU, but
    a set of pointers that ease access to the "private parts" of a PDU.
*/
struct dave_pdu
{
    uint8_t * header;	/* pointer to start of PDU (PDU header) */
    uint8_t * param;		/* pointer to start of parameters inside PDU */
    uint8_t * data;		/* pointer to start of data inside PDU */
    uint8_t * udata;		/* pointer to start of data inside PDU */
    int32_t hlen;		/* header length */
    int32_t plen;		/* parameter length */
    int32_t dlen;		/* data length */
    int32_t udlen;		/* user or result data length */
};

/**
    PDU handling:
    PDU is the central structure present in S7 communication.
    It is composed of a 10 or 12 byte header,a parameter block and a data block.
    When reading or writing values, the data field is itself composed of a data
    header followed by payload data
**/
struct dave_pdu_header
{
    uint8_t P;	    /* allways 0x32 */
    uint8_t type;	/* Header type, one of 1,2,3 or 7. type 2 and 3 headers are two bytes longer. */
    uint8_t a, b;	/* currently unknown. Maybe it can be used for long numbers? */
    uint16_t number;	/* A number. Self can be used to make sure a received answer */
    /* corresponds to the request with the same number. */
    uint16_t plen;	/* length of parameters which follow this header */
    uint16_t dlen;	/* length of data which follow the parameters */
    uint8_t result[2]; /* only present in type 2 and 3 headers. Self contains error information. */
};

/*
    same as above, but made up of single bytes only, so that every single byte can be adressed separately
*/
struct dave_pdu_header2
{
    uint8_t P;	    /* allways 0x32 */
    uint8_t type;	/* Header type, one of 1,2,3 or 7. type 2 and 3 headers are two bytes longer. */
    uint8_t a, b;	/* currently unknown. Maybe it can be used for long numbers? */
    uint8_t numberHi, numberLo;	/* A number. Self can be used to make sure a received answer */
    /* corresponds to the request with the same number. */
    uint8_t plenHi, plenLo;	    /* length of parameters which follow this header */
    uint8_t dlenHi, dlenLo;	    /* length of data which follow the parameters */
    uint8_t result[2];           /* only present in type 2 and 3 headers. Self contains error information. */
};

struct dave_routing_data
{
    int32_t connectionType;
    int32_t destinationType; 	// destinationIsIP=DestinationIsIP;
    int32_t SubnetID1;
    int32_t SubnetID2;
    int32_t SubnetID3;
    int32_t PLCadrsize;
    uint8_t  PLCadr[4];		// currently, IP is maximum. Maybe there could be MAC adresses for Industrial Ethernet?
};

/*
    Get the eror code from a PDU, if one.
*/
int16_t DAVE_DECL daveGetPDUerror(DavePDU * PDU);

/*
    Hex dump PDU:
*/
void DAVE_DECL _daveDumpHexPDU(DavePDU * PDU);

/*
set up the header. Needs valid header pointer
*/
void DAVE_DECL _daveInitPDUheader(DavePDU * PDU, int32_t type);

/*
    add parameters after header, adjust pointer to data.
    needs valid header
*/
void DAVE_DECL _daveAddPDUParam(DavePDU * PDU, uint8_t * Param, uint16_t Len);

/*
    add data after parameters, set dlen
    needs valid header,parameters
*/
void DAVE_DECL _daveAddPDUData(DavePDU * PDU, void * data, int32_t len);

/*
    add values after value header in data, adjust dlen and data count.
    needs valid header,parameters,data,dlen
*/
void DAVE_DECL _daveAddPDUValue(DavePDU * PDU, void * data, int len);

void DAVE_DECL daveAddPDUVarToReadRequest(DavePDU *PDU, int32_t area, int32_t DBnum, int32_t start, int32_t byteCount, int32_t isBit);

#endif /* DAVE_PDU_H */
