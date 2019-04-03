
#include "ZwSpy.h"

// Hmi's UsartRecvBuffer And Position 																						 
static uint8_t HmiBuf[ZW_SPY_MAX];		/**< Hmi 模式缓冲区  */
static uint8_t HmiBufPos;				/**< Hmi 模式缓冲区索引  */

// Spy's UsartRecvBuffer And Position
static uint8_t SpyBuf[ZW_SPY_MAX];	/**< Spy 模式缓冲区  */
static uint8_t SpyBufPos;			/**< Spy 模式缓冲区索引  */
static uint8_t SpySendCount;			/**< Spy 模式发送计数器  */

// DataPortMode Parameter            
#define HMI		1					/**< 标记：Hmi 模式  		*/
#define SPY		0					/**< 标记：监控模式  		*/

// DataPortMode Used Switch Data's Buffer And Position (init to default value)      
static uint8_t SpyFlagEnd = '\x0A';     /**< 标记：协议终止符 （默认值：'\x0A'）*/
static uint8_t DataPortMode = HMI;		/**< 标识监控模式（默认值：人机接口 HMI）  */
static uint8_t *Data = HmiBuf;			/**< 监控缓冲区接口（默认值：Hmi 接收缓冲区）  */
static uint8_t *Pos = &HmiBufPos;		/**< 监控缓冲区索引（默认值：Hmi 缓冲区索引）  */
static uint16_t DataLen = 0;

void ZwSpySetEndFlag(uint8_t flag)
{
    SpyFlagEnd = flag;
}

// Wait Recv Hmi And Plc Data
uint16_t ZwSpyWaitHmiData(uint8_t *buf, External func, void * param)
{
    // wait data recv begin
    while ( 0 == *Pos ) func(param);
    // wait data recv end
    while ( *Pos ) func(param);
    // fast move data to an external buffer
    strncpy((char *) buf, (char *) HmiBuf, ZW_SPY_MAX);
    return DataLen;
}

void ReadySpySend(uint8_t *buf)
{
    for ( SpySendCount = 0; '\0' != buf[SpySendCount]; SpySendCount++ )
    {
        SpyBuf[SpySendCount] = buf[SpySendCount];
    }
}

void ZwSpySend(uint8_t *buf)
{
    // Ready Data To Buffer
    ReadySpySend(buf);
    // Set Time Out Conut
    uint32_t time = 0;
    // Switch DataPort To Spy
    DataPortMode = SPY;
    // Wait DataPort Switch Success 
    while ( Data != SpyBuf && ++time );
    // Set Next Recv DataPort To Hmi
    DataPortMode = HMI;
    // Send Spy's Buffer
    for ( SpyBufPos = 0; SpyBufPos < SpySendCount; SpyBufPos++ )
    {
        ZwSpyPlcSend(SpyBuf[SpyBufPos]);
    }
    // Wait Hmi DataPort Switch Success
    while ( Data != HmiBuf && ++time );
    // Copy Second half Result
    // strcpy((char *)Buf+SpySendCount, (char *)SpyBuf+SpySendCount);
    // Copy All Result
    strcpy((char *) buf, (char *) SpyBuf);
}

// Record And Relay Hmi Code
void ZwSpyPlc(void)
{
    if ( ZwSpyPlcEvent( ) )
    {
        if ( Data == HmiBuf )
        {
            HmiBuf[HmiBufPos] = ZwSpyPlcRecv( );
            ZwSpyPlcSend(HmiBuf[HmiBufPos]);
            HmiBufPos++;
        }
    }
}

// Record And Relay Plc Code
void ZwSpyHmi(void)
{
    if ( ZwSpyHmiEvent( ) )
    {
        Data[*Pos] = ZwSpyHmiRecv( );
        if ( SpyFlagEnd != Data[*Pos] )
        {
            if ( Data == HmiBuf )
            {
                ZwSpyHmiSend(Data[*Pos]);
            }
            (*Pos)++;
        }
        else
        {
            DataLen = *Pos, Data[*Pos + 1] = '\0', *Pos = 0;
            if ( DataPortMode )
            {
                Pos = &HmiBufPos;
                Data = HmiBuf;
                ZwSpyHmiSend(SpyFlagEnd);
            }
            else
            {
                Pos = &SpyBufPos;
                Data = SpyBuf;
            }
        }
    }
}

#ifndef STM32

bool ZwSpyPlcEvent(void)
{
	return true;
}

void ZwSpyPlcSend(uint8_t data)
{
	;
}

uint8_t ZwSpyPlcRecv(void)
{
	return '\0';
}

bool ZwSpyHmiEvent(void)
{
	return true;
}

void ZwSpyHmiSend(uint8_t data)
{
	;
}

uint8_t ZwSpyHmiRecv(void)
{
	return '\0';
}
#endif
