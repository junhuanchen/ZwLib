//-------------------------------------------------------------------------
//代码来源于linux内核
//以下为函数使用说明文档:

/** 
 * @file
 * @brief 算法：AES
 * @author Linux kernel
 * @date 2016-11-03 16:37:15
 * @version 1.0
 * @remark 提供加密解密16字节的AES算法，真正应用AES须另外提供填充手段，例如：NoPadding，PKCS5Padding，ISO10126Padding。
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
 * @brief AES 私有数据
 * @remark 提供私有的变量和缓冲区供 AES 函数调用。
 */
struct aes_ctx
{
    int key_length;	/**< 私有密钥长度  */
    uint32_t buf[120];	/**< 执行AES缓冲区 */
};

/**
 * @brief 初始化 AES 静态变换表资源
 * @remark 使用 AES 其他函数之前请调用该函数进行算法环境初始化。
 * @return 无
 */
void aes_gen_tabs(void);

/**
 * @brief 设定一个私有密钥
 * @remark 这是调用 aes_encrypt 和 aes_decrypt 的前提。
 * @param[in] ctx AES 算法私有数据结构体
 * @param[in] in_key 传递使用者所用私有密钥字符串，长度为key_len。
 * @param[in] key_len 指明私有密钥字符串长度，长度须在
 * @return 返回 -1 表示设定失败。
 * @note
 * 对于每一个aes_ctx变量都需要设定一个密钥(字符串)，
 * 密钥字符串长度要求指定在区间[16, 32]且能整除8,故只有16 24 32三种长度可选。
 * 不同密钥值产生的数据是无法解开的, 所以双方须拥有唯一私钥且必须是指定长度相同的字符串。
 * @see aes_encrypt aes_decrypt
 */
int aes_set_key(struct aes_ctx *ctx, uint8_t *in_key, uint32_t key_len);

/**
 * @brief 数据加密
 * @remark 本函数一次操作一块16字节数据，不满足16倍数数据需另执行padding（填充）
 * @param[in] ctx AES 算法私有数据结构体
 * @param[out] out 存储密文的缓冲区
 * @param[in] in 需加密的数据
 * @return 无
 */
void aes_encrypt(struct aes_ctx *ctx, uint8_t *out, uint8_t *in);

/**
 * @brief 数据解密
 * @remark 本函数一次操作一块16字节数据，不满足16倍数数据需另执行padding（填充）
 * @param[in] ctx AES 算法私有数据结构体
 * @param[out] out 存储明文的缓冲区
 * @param[in] in 需解密的数据
 * @return 无
 */
void aes_decrypt(struct aes_ctx *ctx, uint8_t *out, uint8_t *in);

#endif

