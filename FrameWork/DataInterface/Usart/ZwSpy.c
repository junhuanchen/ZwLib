
#include "ZwSpy.h"

// Hmi's UsartRecvBuffer And Position 																						 
static uint8_t HmiBuf[ZW_SPY_MAX];		/**< Hmi ģʽ������  */
static uint8_t HmiBufPos;				/**< Hmi ģʽ����������  */

// Spy's UsartRecvBuffer And Position
static uint8_t SpyBuf[ZW_SPY_MAX];	/**< Spy ģʽ������  */
static uint8_t SpyBufPos;			/**< Spy ģʽ����������  */
static uint8_t SpySendCount;			/**< Spy ģʽ���ͼ�����  */

// DataPortMode Parameter            
#define HMI		1					/**< ��ǣ�Hmi ģʽ  		*/
#define SPY		0					/**< ��ǣ����ģʽ  		*/

// DataPortMode Used Switch Data's Buffer And Position (init to default value)      
static uint8_t SpyFlagEnd = '\x0A';     /**< ��ǣ�Э����ֹ�� ��Ĭ��ֵ��'\x0A'��*/
static uint8_t DataPortMode = HMI;		/**< ��ʶ���ģʽ��Ĭ��ֵ���˻��ӿ� HMI��  */
static uint8_t *Data = HmiBuf;			/**< ��ػ������ӿڣ�Ĭ��ֵ��Hmi ���ջ�������  */
static uint8_t *Pos = &HmiBufPos;		/**< ��ػ�����������Ĭ��ֵ��Hmi ������������  */
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
