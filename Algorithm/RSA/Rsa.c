
#include "Rsa.h"

RsaVar RsaCrypt(RsaVar Data, RsaVar Crypt, RsaVar Max)
{
	return CryptFastPowMod(Data, Crypt, Max);
}

#ifdef UNIT_TEST

#include "stdio.h"

// ��ʼ��RsaVarģ�飬���ɶԳ���Կ����Կ�������ⲿ�ṩ��
static void RsaCryptInit(RsaVar Max, RsaVar * Encrypt, RsaVar * Decrypt)
{
	// N ���ݹ�ģ�� En = ��(N)��ŷ������������� Encrypt �� Decrypt �ֱ�Ϊ������Կ
	RsaVar N = Max, En = CryptEular(N);
	if (*Encrypt < 2) *Encrypt = 2;
	// ����__RSA_H__����
	do
	{
		// ѡȡ Encrypt �� En ���ʵ���
		while (1 != CryptGcd(En, *Encrypt))
			*Encrypt += 1;
		// �����˷���Ԫ���� Decrypt == -1 ���ʾ En �� Encrypt ������
		*Decrypt = CryptEgcdInverse(*Encrypt, En);
	} while (-1 == *Decrypt);
	// ��ӡ��Կ����
	printf("En:%llu\n", En);
	printf("Encrypt:%llu\n", *Encrypt);
	printf("Decrypt:%llu\n", *Decrypt);
}

#include <stdlib.h>
#include <assert.h>

#include <stdio.h>

int main()
{
	// N ���ݹ�ģ�� En = ��(N)��ŷ�������������Encrypt �� Decrypt �ֱ�Ϊ������Կ
	RsaVar N = UINT8_MAX, Encrypt = 7, Decrypt;
	RsaCryptInit(N, &Encrypt, &Decrypt);

	// ��������ѭ����Ȼ����Ϊû�ܻ�ԭ��ԭ���ݵ����޷�������һ����(++)�������в��Դ���N�����ݹ�ģ
	// Test Encrypt Key
	RsaVar src, tmp;
	for (src = 0; src != N; src++)
	{
		tmp = RsaCrypt(src, Encrypt, N);
		tmp = RsaCrypt(tmp, Decrypt, N);
		assert(src == tmp);
		printf("%02hhX\n", src);
	}
	// Test Decrypt Key
	for (src = 0; src != N; src++)
	{
		tmp = RsaCrypt(src, Decrypt, N);
		tmp = RsaCrypt(tmp, Encrypt, N);
		assert(src == tmp);
		printf("%02hhX\n", src);
	}
	return 0;
}

#endif
