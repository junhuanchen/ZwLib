
/** @file
 * @brief ����PLC��HMIͨѶģ��
 * @author JunHuanChen
 * @date 2016-10-31 12:00:10
 * @version 1.0
 * @remark ����PLC��PLC֮�䵥�ֽڴ����ͨ�š�
 */
#ifndef ZW_SPY_H
#define ZW_SPY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../HardWare.h"

    // ZwSpy Buffer Max Length
#define ZW_SPY_MAX 0x616									/**< ZwLibϵ�д���Э������Ļ�������С  */

    typedef void(*External)(void*);				/**< ZwSpyWaitHmiData �����ⲿ�������������ṩ��ȴ�HMI�������������䲻�������Ĳ���ִ�С�  */

    /**
     * @brief ���ü���һ�����ڵ�������ֹ��
     * @remark ŷķ�� hostlink '\x0D' ����ʿ ��λ��· '\x0A'
     * @param[in] flag ASCII ��ֹ��
     * @return ��
     */
    void ZwSpySetEndFlag(uint8_t flag);
	
    /**
     * @brief �ȴ�����HMI��PLC��������������
     * @remark ���յ���HMI������PLCӦ���һ�����ص��������ݣ����磺"RD DM00510\r\n00300\r\n"��
     * @param[in] buf �������ݵĻ��������䳤��ӦΪ ZW_SPY_MAX
     * @param[out] buf �洢��������м�����������
     * @param[in] func �����ṩ���ⲿ���������磺���������н�������ȡ�
     * @return ���յ������ݳ���
     */
    uint16_t ZwSpyWaitHmiData(uint8_t *buf, External func, void * param);
	
    /**
     * @brief �������ݵ�����ģ�鷢�ͻ�����
     * @remark ��Ҫ�����������ݵ�PLC֮ǰ��Ҫ���ַ������嵽��ģ��Ļ�������
     * @param[in] buf �����͵��ַ�������'\0'�ַ���β��
     * @return ��
     * @see ZwSpySend
     */
    void ZwSpySendReady(uint8_t *buf);

    /**
     * @brief ����ģ�鷢������
     * @remark һֱ���������͵�PLC�Ļ������Լ�Ӧ�����Ϊֹ��ʹ��ǰ�뻺�����ݵ����ͻ������С�
     * @param[in] buf �������ݵĻ��������䳤��ӦΪ ZW_SPY_MAX
     * @param[out] buf �洢���͵�PLC�������Լ�Ӧ������
     * @return ��
     * @see ZwSpySendReady
     */
    void ZwSpySend(uint8_t *buf);

    /**
     * @brief ����PLC�Ĵ��䣨���ⲿ�жϴ�����
     * @remark �ṩ���ⲿ�ж�ʱ��PLC�����ݹ���ʱ�����жϺ���øú����Խ�һ���õ�PLC�����ݣ�������Ļ����һ���鿴�μ���
     * @return ��
     * @see ZwSpyPlcEvent ZwSpyPlcRecv
     */
    void ZwSpyPlc(void);

    /**
     * @brief ����Ƿ񴥷���PLC�������ݵ��¼������ⲿ�ṩ��
     * @remark ����PLC���¼������Ǵ������ݡ�
     * @return �������ݴ����򷵻��棬���򷵻ؼ�
     * @see ZwSpyPlc ZwSpyPlcRecv
     */
    extern bool ZwSpyPlcEvent(void);

    /**
     * @brief ��PLC������һ���ֽڣ����ⲿ�ṩ��
     * @remark ǰ���Ǵ�����ZwSpyPlc��ͨ����ZwSpyPlcSend��
     * @return ��
     * @see ZwSpyPlc ZwSpyPlcSend
     */
    extern uint8_t ZwSpyPlcRecv(void);

    /**
     * @brief ����һ���ֽڸ�PLC�����ⲿ�ṩ��
     * @return ��
     */
    extern void ZwSpyPlcSend(uint8_t data);

    /**
     * @brief ����HMI�Ĵ��䣨���ⲿ�жϴ�����
     * @remark �ṩ���ⲿ�ж�ʱ��HMI�����ݹ���ʱ�����жϺ���øú����Խ�һ���õ�HMI�����ݣ�������Ļ����һ���鿴�μ���
     * @return ��
     * @see ZwSpyHmiEvent ZwSpyHmiRecv
     */
    void ZwSpyHmi(void);

    /**
     * @brief ����Ƿ񴥷���HMI�������ݵ��¼������ⲿ�ṩ��
     * @remark ����HMI���¼������Ǵ������ݡ�
     * @return �������ݴ����򷵻��棬���򷵻ؼ�
     * @see ZwSpyHmi ZwSpyHmiRecv
     */
    extern bool ZwSpyHmiEvent(void);

    /**
     * @brief ��HMI������һ���ֽڣ����ⲿ�ṩ��
     * @remark ǰ���Ǵ�����ZwSpyHmi��ͨ����ZwSpyHmiSend��
     * @return ��
     * @see ZwSpyHmi ZwSpyHmiSend
     */
    extern uint8_t ZwSpyHmiRecv(void);

    /**
     * @brief ����һ���ֽڸ�HMI�����ⲿ�ṩ��
     * @return ��
     */
    extern void ZwSpyHmiSend(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // ZW_SPY_H
