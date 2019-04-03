//-------------------------------------------------------------------------
//������Դ��linux�ں�
//����Ϊ����ʹ��˵���ĵ�:

/** 
 * @file
 * @brief �㷨��AES
 * @author Linux kernel
 * @date 2016-11-03 16:37:15
 * @version 1.0
 * @remark �ṩ���ܽ���16�ֽڵ�AES�㷨������Ӧ��AES�������ṩ����ֶΣ����磺NoPadding��PKCS5Padding��ISO10126Padding��
 */
#ifndef __AES_H__
#define __AES_H__

#define AES_MIN_KEY_SIZE    16
#define AES_MAX_KEY_SIZE    32

#define AES_BLOCK_SIZE      16

#include <stdint.h>

typedef uint16_t        __le16;
typedef uint32_t        __le32;

#define E_KEY    (&ctx->buf[0])
#define D_KEY    (&ctx->buf[60])

#define le32_to_cpu
#define cpu_to_le32

/**
 * @struct aes_ctx
 * @brief AES ˽������
 * @remark �ṩ˽�еı����ͻ������� AES �������á�
 */
struct aes_ctx
{
    int key_length;	/**< ˽����Կ����  */
    uint32_t buf[120];	/**< ִ��AES������ */
};

/**
 * @brief ��ʼ�� AES ��̬�任����Դ
 * @remark ʹ�� AES ��������֮ǰ����øú��������㷨������ʼ����
 * @return ��
 */
void aes_gen_tabs(void);

/**
 * @brief �趨һ��˽����Կ
 * @remark ���ǵ��� aes_encrypt �� aes_decrypt ��ǰ�ᡣ
 * @param[in] ctx AES �㷨˽�����ݽṹ��
 * @param[in] in_key ����ʹ��������˽����Կ�ַ���������Ϊkey_len��
 * @param[in] key_len ָ��˽����Կ�ַ������ȣ���������
 * @return ���� -1 ��ʾ�趨ʧ�ܡ�
 * @note
 * ����ÿһ��aes_ctx��������Ҫ�趨һ����Կ(�ַ���)��
 * ��Կ�ַ�������Ҫ��ָ��������[16, 32]��������8,��ֻ��16 24 32���ֳ��ȿ�ѡ��
 * ��ͬ��Կֵ�������������޷��⿪��, ����˫����ӵ��Ψһ˽Կ�ұ�����ָ��������ͬ���ַ�����
 * @see aes_encrypt aes_decrypt
 */
int aes_set_key(struct aes_ctx *ctx, uint8_t *in_key, uint32_t key_len);

/**
 * @brief ���ݼ���
 * @remark ������һ�β���һ��16�ֽ����ݣ�������16������������ִ��padding����䣩
 * @param[in] ctx AES �㷨˽�����ݽṹ��
 * @param[out] out �洢���ĵĻ�����
 * @param[in] in ����ܵ�����
 * @return ��
 */
void aes_encrypt(struct aes_ctx *ctx, uint8_t *out, uint8_t *in);

/**
 * @brief ���ݽ���
 * @remark ������һ�β���һ��16�ֽ����ݣ�������16������������ִ��padding����䣩
 * @param[in] ctx AES �㷨˽�����ݽṹ��
 * @param[out] out �洢���ĵĻ�����
 * @param[in] in ����ܵ�����
 * @return ��
 */
void aes_decrypt(struct aes_ctx *ctx, uint8_t *out, uint8_t *in);

#endif

