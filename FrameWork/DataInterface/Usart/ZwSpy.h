
/** @file
 * @brief 监听PLC和HMI通讯模块
 * @author JunHuanChen
 * @date 2016-10-31 12:00:10
 * @version 1.0
 * @remark 监听PLC与PLC之间单字节传输的通信。
 */
#ifndef ZW_SPY_H
#define ZW_SPY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../HardWare.h"

    // ZwSpy Buffer Max Length
#define ZW_SPY_MAX 0x616									/**< ZwLib系列传输协议监听的缓冲区大小  */

    typedef void(*External)(void*);				/**< ZwSpyWaitHmiData 调用外部函数的声明，提供与等待HMI数据事务外与其不相关事务的并行执行。  */

    /**
     * @brief 设置监听一个周期的数据终止符
     * @remark 欧姆龙 hostlink '\x0D' 基恩士 上位链路 '\x0A'
     * @param[in] flag ASCII 终止符
     * @return 无
     */
    void ZwSpySetEndFlag(uint8_t flag);
	
    /**
     * @brief 等待接收HMI与PLC传输完整的数据
     * @remark 接收的是HMI控制与PLC应答的一次来回的完整数据，例如："RD DM00510\r\n00300\r\n"。
     * @param[in] buf 接收数据的缓冲区，其长度应为 ZW_SPY_MAX
     * @param[out] buf 存储传输过程中监听到的数据
     * @param[in] func 调用提供的外部函数，例如：监听过程中将灯亮起等。
     * @return 接收到的数据长度
     */
    uint16_t ZwSpyWaitHmiData(uint8_t *buf, External func, void * param);
	
    /**
     * @brief 缓冲数据到监听模块发送缓冲区
     * @remark 想要主动发送数据到PLC之前需要将字符串缓冲到本模块的缓冲区。
     * @param[in] buf 待发送的字符串（以'\0'字符结尾）
     * @return 无
     * @see ZwSpySend
     */
    void ZwSpySendReady(uint8_t *buf);

    /**
     * @brief 监听模块发送数据
     * @remark 一直阻塞到发送到PLC的缓冲区以及应答结束为止，使用前须缓冲数据到发送缓冲区中。
     * @param[in] buf 接收数据的缓冲区，其长度应为 ZW_SPY_MAX
     * @param[out] buf 存储发送到PLC的数据以及应答数据
     * @return 无
     * @see ZwSpySendReady
     */
    void ZwSpySend(uint8_t *buf);

    /**
     * @brief 监听PLC的传输（由外部中断触发）
     * @remark 提供给外部中断时，PLC的数据过来时触发中断后调用该函数以进一步得到PLC的数据，不清楚的话请进一步查看参见。
     * @return 无
     * @see ZwSpyPlcEvent ZwSpyPlcRecv
     */
    void ZwSpyPlc(void);

    /**
     * @brief 检查是否触发了PLC传输数据的事件（由外部提供）
     * @remark 触发PLC的事件不仅是传输数据。
     * @return 若是数据传输则返回真，否则返回假
     * @see ZwSpyPlc ZwSpyPlcRecv
     */
    extern bool ZwSpyPlcEvent(void);

    /**
     * @brief 从PLC处接收一个字节（由外部提供）
     * @remark 前提是触发了ZwSpyPlc和通过了ZwSpyPlcSend。
     * @return 无
     * @see ZwSpyPlc ZwSpyPlcSend
     */
    extern uint8_t ZwSpyPlcRecv(void);

    /**
     * @brief 发送一个字节给PLC（由外部提供）
     * @return 无
     */
    extern void ZwSpyPlcSend(uint8_t data);

    /**
     * @brief 监听HMI的传输（由外部中断触发）
     * @remark 提供给外部中断时，HMI的数据过来时触发中断后调用该函数以进一步得到HMI的数据，不清楚的话请进一步查看参见。
     * @return 无
     * @see ZwSpyHmiEvent ZwSpyHmiRecv
     */
    void ZwSpyHmi(void);

    /**
     * @brief 检查是否触发了HMI传输数据的事件（由外部提供）
     * @remark 触发HMI的事件不仅是传输数据。
     * @return 若是数据传输则返回真，否则返回假
     * @see ZwSpyHmi ZwSpyHmiRecv
     */
    extern bool ZwSpyHmiEvent(void);

    /**
     * @brief 从HMI处接收一个字节（由外部提供）
     * @remark 前提是触发了ZwSpyHmi和通过了ZwSpyHmiSend。
     * @return 无
     * @see ZwSpyHmi ZwSpyHmiSend
     */
    extern uint8_t ZwSpyHmiRecv(void);

    /**
     * @brief 发送一个字节给HMI（由外部提供）
     * @return 无
     */
    extern void ZwSpyHmiSend(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // ZW_SPY_H
