
#ifndef __DHT11_H__
#define __DHT11_H__

#ifndef _WIN32

#include <ioCC2530.h>
#include "OnBoard.h"

#define DATA_PIN P0_7

extern bool DHT11(uint8 *T, uint8 *RH);

//读取传感器数据
static void COM(uint8 *Com)
{   
    uint8 i, FLAG, temp;
    for(i = 0; i != 8; i++)  
    {
        FLAG = 2;
        while((!DATA_PIN) && FLAG++);
        MicroWait(30);
        temp = 0;
        if(DATA_PIN) temp = 1;
        FLAG = 2;
        while((DATA_PIN) && FLAG++);
        if(FLAG == 1) break;
        *Com = (*Com << 1) | temp; 
    }  
}

// 获取数据，当前的传感器无浮点数，故剔除小数参数。
bool DHT11(uint8 *T, uint8 *RH)
{
    uint8 FLAG, HighT, LowT, HighRH, LowRH, Check;
    DATA_PIN = 0;
    MicroWait(19000); //>18MS
    DATA_PIN = 1; 
    P0DIR &= ~0x80; //重新配置IO口方向
    MicroWait(40);
    if(!DATA_PIN)
    {
        FLAG = 2; 
        while((!DATA_PIN) && FLAG++);
        FLAG = 2;
        while((DATA_PIN) && FLAG++); 
        COM(&HighRH);
        COM(&LowRH);
        COM(&HighT);
        COM(&LowT);
        COM(&Check);
        DATA_PIN = 1;
        if(Check == (HighT + LowT + HighRH + LowRH))
        {
            *RH = HighRH;
            *T = HighT;
            FLAG = true;
        }
        else
        {
            // 数据传输异常
            FLAG = false;
        }
    } 
    else
    {
        // 温湿度传感器接口异常
        *T = *RH = 0;
        FLAG = false;
    }
    P0DIR |= 0x80; //IO口需要重新配置
    return FLAG;
}

#endif

#endif // __DHT11_H__