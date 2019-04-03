
#ifndef __CRYPT_H__
#define __CRYPT_H__

#ifndef __IAR_SYSTEMS_ICC__
#include <stdint.h>
#else
#include "../HardWare.h"
#endif

// Rsa ����һ�����޷��ŵ�������
// Ecc ����һ�����з��ŵģ���Ϊ�и�Ԫ����
// ע��ģ�ݺ���ֻ֧���޷��ŵġ�
// �������з��ţ���֮Ϊ�޷��š�
// ���루���㣩��ȫ�궨���еĻ�಻һ��ͨ�ã�������64λ������Ҫ��Ӧ��ֲ��
#define ECC_RSA_8
#ifdef ECC_RSA_8
// Ŀǰʹ�õ� RSA ����
typedef int8_t CryptInt;
typedef uint8_t CryptUint;
#elif defined(ECC_RSA_64)
typedef int64_t CryptInt;
typedef uint64_t CryptUint;
// #define CRYPT_SUB_SAFE -64
// #define CRYPT_ADD_SAFE 64
#else
#error "select a config"
#endif

// Montgomery���㺯��ָ��
typedef CryptUint(*CryptMontgomeryPointer)(CryptUint, CryptUint, CryptUint);

// return (a - b) % mod ��������ȷ�����а�ȫ��
static CryptInt CryptSafeSubMod(CryptInt a, CryptInt b, CryptInt mod)
{
#ifdef CRYPT_SUB_SAFE
	// ȡ����ǼĴ���
	unsigned short flag = 0;
	// ������
	CryptInt result;
	// ��ȡ����������
	// result = (a - b);
#if (CRYPT_SUB_SAFE == -32) // �з���32λ����
	__asm
	{
		mov         eax, dword ptr[a]
		sub         eax, dword ptr[b]
		pushf
		pop flag
		mov         dword ptr[result], eax
	}
#elif  (CRYPT_SUB_SAFE == -64)
	// /Z7��/Zi��/ZI��������Ϣ��ʽ����ͬ�������ͬ�Ļ����룬�ⲿ�ֻ�಻ͨ�á�
	// result = (a - b);
	__asm
	{
		mov         ecx, dword ptr[a]
		sub         ecx, dword ptr[b]
		mov         edx, dword ptr[ebp + 0Ch]
		sbb         edx, dword ptr[ebp + 14h]
		pushf
		pop			flag
		mov         dword ptr[result], ecx
		mov         dword ptr[ebp - 8], edx
	}
	//__asm
	//{
	//	mov         eax, dword ptr[a]
	//	sub         eax, dword ptr[b]
	//	mov         ecx, dword ptr[ebp + 0Ch]
	//	sbb         ecx, dword ptr[ebp + 14h]
	//	pushf
	//	pop			flag
	//	mov         dword ptr[result], eax
	//	mov         dword ptr[ebp - 14h], ecx
	//}
#else
	result = (a - b);
#endif

	// �ж��Ƿ�������ǼĴ��������ʽ���ж���������λ��
	// a > 0 && b < 0 && res < 0(��ֱ�Ӽ�����λ) => �����
	// a < 0 && b > 0 && res > 0(��ֱ�Ӽ�����λ) => �����
	// ��״̬�Ĵ�������OF��λ��ǵ�ʱ��0x800 == (flag & 0x800)��
	if (0x800 == (flag & 0x800))
	{
		// a != 0���������������
		// a - b ���� ������������
		// �ƶ� a > 0 => b < 0 ���������
		// �ƶ� a < 0 => b > 0 ���������
		b += (a > 0) ? mod : -mod;
		result = (a - b);
	}
	// �з��ŵ�С����������������,��Ϊ�ⲿPowMod�ĵ�������ָ����Ϊ������
	while (result < 0) result += mod;
	return result;

#else

	CryptInt result = (a - b) % mod;
	while (result < 0) result += mod;
	return result % mod;

#endif
}

static CryptUint CryptSafeAddMod(CryptUint a, CryptUint b, CryptUint mod)
{
#ifdef CRYPT_ADD_SAFE
#if (CRYPT_ADD_SAFE < 0)
	// ������֪���з��Ų���
	// ��������c >= a, c >= b => a + b <= 2*c
	// ����2*c��Ϊ�޷����������ֵ������λ����
	CryptUint m = (a + b);
	while (m > mod) m -= mod;
	return m;
#else  // ������֪���з��Ų�������ȡFLAG�Ĵ���CFֵ�ж��Ƿ���������������
	// ����(a + a)��aΪ���ֵ���ʱ�ᵼ��������������ֵ��������Ϊ0��λ״̬������CF�Ĵ�����
	// ��ʵ����ͨ��mod������Ӧ�ûع�С�������ֵ�����Ըüӷ�����һ��������е�����
	// ȡ����ǼĴ���
	unsigned short flag = 0;
	// ������
	CryptInt result;
	// result = (a + b);
#if (CRYPT_ADD_SAFE == 32)
#define TypeMax UINT32_MAX
	__asm
	{
		mov         eax, dword ptr[a]
			add         eax, dword ptr[b]
			pushf
			pop flag
			mov         dword ptr[result], eax
	}
#elif (CRYPT_ADD_SAFE == 64)
#define TypeMax UINT64_MAX
	// result = (a + b);
	__asm
	{
		mov         ecx, dword ptr[a]
		add         ecx, dword ptr[b]
		mov         edx, dword ptr[ebp + 0Ch]
		adc         edx, dword ptr[ebp + 14h]
		pushf
		pop			flag
		mov         dword ptr[result], ecx
		mov         dword ptr[ebp - 8], edx
	}
	
#elif ((CRYPT_ADD_SAFE == 16) || (CRYPT_ADD_SAFE == 8))
#define TypeMax 0
	// ʵ������״̬�Ĵ���CFλ��δ�仯
	// �޷���Ҳ�޷��жϷ���λ����ʱֻ��ǿת���������������ݡ�
	result = ((size_t) a + b) % mod;
	return result;
#else 
#error "set a type W"
#endif
	// ��״̬�Ĵ�������CF��λ��ǵ�ʱ��1 == (flag & 1)��
	if (1 == (flag & 1))
	{
		// a + b > TypeMax, (TypeMax - a - b) < 0, ����Ϊת������
		return -(TypeMax - a - b);
	}
	return result % mod;
#endif
#else 
	// Ĭ�ϲ���
	return ((a + b) % mod);
#endif
}

// Montgomery �������㷨���
static void CryptMontgomeryFrame(CryptUint *ans, CryptMontgomeryPointer Func, CryptUint a, CryptUint b, CryptUint mod)
{
WHILE:
	if (b & 1)// LSBλΪ 1
	{
		b--, *ans = Func(*ans, a, mod);
	}
	b >>= 1;
	if (0 == b)
	{
		return ;
	}
	a = Func(a, a, mod);
	goto WHILE;
}

static CryptUint CryptFastMulMod(CryptUint a, CryptUint b, CryptUint mod)
{
	CryptUint ans = 0;
	CryptMontgomeryFrame(&ans, CryptSafeAddMod, a, b, mod);
	return ans;
}

// ��ģ����a^b%k������棩
// ͨ��˼ά�������汾���벻Ҫʹ�øð汾����Ȼ��޵ģ�
static CryptUint NormalPowMod(CryptUint a, CryptUint b, CryptUint mod)
{
	CryptUint ans = 1;
	while (b--) ans *= a;
	return ans % mod;
}

// ��ģ����a^b%k������棩
// ��Ҳ�벻Ҫʹ�øð汾����ȻҲ��޵ģ��ݹ������
// �ݹ�汾 a ^ b (mod mod)
// ģ�������ɣ�(a *a) mod mod =( (a mod mod) *a ) mod mod
// C ���Ա�((a *b) % p = (a % p *b) % p)
static CryptUint RecursionPowMod(CryptUint a, CryptUint b, CryptUint mod)
{
	return (b ? (a *RecursionPowMod(a, b - 1, mod)) : 1) % mod;
}

static CryptUint CryptFastPowMod(CryptUint a, CryptUint b, CryptUint mod)
{
	CryptUint ans = 1;
	CryptMontgomeryFrame(&ans, CryptFastMulMod, a, b, mod);
	return ans;
}

//  ����С�������˷���Ԫ������p����������gcd(a,p)=1����ô a^(p?1) �� 1 (mod p)��
// ���Զ���a����Ԫx����ax��1(mod p)����a��p����ʱ���Ƴ�x = a^(p-2),��Ϊ��˷���Ԫ��
// ʵ���з�����ʵ�ֵ�������չŷ��������������Ƚ���Ч�ʲ����ߣ�������n�����ʱ��
static CryptInt CryptFermatInverse(CryptInt a, CryptInt n)
{
	return CryptFastPowMod(a, n - 2, n);
}

#include <stdio.h>

// ���������棺ǰ���� p ���������Ҵ��ڶ���a ����[1, p-1]��
// ��֪ǰ�᣺p Ϊ���������ڶ�������Ϊһ����������
// �⣺a * x1 + p * y1 = u mod p ��
// 	  a * x2 + p * y2 = v mod p ��
// �� u = 1 �� v = 1 ʱ��x1 �� y1 ��Ϊ��Ԫ��
// ��֪ �� �� �� �� u �� p ����ż��ʱ����ʽ�ɳ�����
// ���磺�� u Ϊż��������ֱ����Ϊһʱ��a * x1 ���ɳ��������x1Ϊż�������
// ���x1Ϊ�����������p*(y1 - 1)�������ϲ�Ϊż�����г�����ֱ��uΪ������
// ͬ���ģ�vҲ��������ת����ֱ��vΪ������
// �� u �� v ��Ϊ����ʱ��������ı�ϴ�ֵһ��Ϊż������ʱ�ֿ��Իص���������ż��ת��Ϊ����
// ֱ��ĳһ�������˵���һ����˵����ʱ��x�Ѿ����� a * x + p * y = 1, y ����Ϊ������������a * x = 1 mod p��
// �������Ԫx��
static CryptInt CryptBinaryInverse(CryptInt a, CryptInt n)
{
	CryptInt u = a, v = n;
	CryptInt x1 = 1, x2 = 0;
	while (1 != u && 1 != v)
	{
		while ((u & 1) == 0)
		{
			u >>= 1;
			x1 = ((x1 & 1) == 0) ? x1 >> 1 : (x1 + n) >> 1;
		}
		while ((v & 1) == 0)
		{
			v >>= 1;
			x2 = ((x2 & 1) == 0) ? x2 >> 1 : (x2 + n) >> 1;
		}
		if (u >= v)
		{
			u = u - v, x1 = x1 - x2;
		}
		else
		{
			v = v - u, x2 = x2 - x1;
		}
	}
	if (1 == u)
	{
		while (x1 < 0) x1 += n;
		return x1;
	}
	else
	{
		while (x2 < 0) x2 += n;
		return x2;
	}
	// return (1 == u) ? x1 % n : x2 % n;
}

// ŷ����������(x) = x �� (1-1/p)��P����N����������
static CryptUint CryptEular(CryptUint n)
{
	CryptUint ret = 1;
	for (CryptUint i = 2; i * i <= n; i++)
	{
		if (0 == n % i)
		{
			n /= i, ret *= i - 1;
			while (0 == n % i)
			{
				n /= i, ret *= i;
			}
		}
	}
	if (n > 1)
	{
		ret *= n - 1;
	}
	return ret;
}

// ŷ����κ�����Gcd(a, b��= Gcd(b, a%b)
static CryptUint CryptGcd(CryptUint a, CryptUint b)
{
	return 0 == b ? a : CryptGcd(b, a % b);
}

// ��չŷ����κ�����a*x + b*y = gcd(a, b)
//	  gcd(a, b) = gcd(b, a%b)
//    a%b = a-(a/b)*b
//    a*x + b*y = gcd(a, b) = gcd(b, a%b) = b*x1 + (a-(a/b)*b)*y1
//        = b*x1 + a*y1�C(a/b)*b*y1
//        = a*y1 + b*(x1�Ca/b*y1)
static CryptInt CryptEgcd(CryptInt a, CryptInt b, CryptInt *x, CryptInt *y)
{
	CryptInt result, tmp;
	// �ݹ���ֹ����
	if (0 == b)
	{
		*x = 1, *y = 0;
		return a;
	}
	result = CryptEgcd(b, a % b, x, y);
	tmp = *x, *x = *y;
	// ���Կ��ǳ˷��Ż�
	*y = tmp - (a / b) * (*y);
	return result;
}

// ���˷���Ԫ���� a * (return value) �� 1 (mod n)��
// ǰ�������߻�Ϊ���أ�ͨ����չŷ��������㡣
// ������Ч�Ľ���a��n��������������n�ϴ��ʱ��ȷ���С�������Ч��
static CryptInt CryptEgcdInverse(CryptInt a, CryptInt n)
{
	CryptInt x = 0, y = 0;
	if (CryptEgcd(n, a, &x, &y) != 1)
	{
		return 0;
	}
	// ȷ�������뱻��������һ��
	// if (a < 0) y = -y; // ����ȡģ������ģ��Ĭ��Ϊ�޷���ȡģ��
	// while (y > n) y -= n;// y %= n; // ����yֵ��ģ��
	if (y > n) y %= n; // ����yֵ��ģ��
	if (y < 0) y += n;
	return y;
}

#include <stdlib.h>

//����[ 0 , n ]�������
static CryptUint CryptRandom(CryptUint n)
{
	return (CryptUint) ((double) rand() / RAND_MAX * n + 0.5);
}

//miller_rabin�㷨���ж�Ԫ��
static uint8_t CryptWitness(CryptUint a, CryptUint n)
{
	//�ü�������a������n�ǲ�������
	CryptUint tmp = n - 1;
	uint8_t len = 0;

	while (tmp % 2 == 0)
	{
		tmp /= 2, len++;
	}

	//��n-1���Ϊa^r * s
	CryptUint x = CryptFastPowMod(a, tmp, n); //�õ�a^r mod n
	//����Ϊ1��Ϊ����
	if (x == 1 || x == n - 1)
	{
		return 1;
	}
	//������������2���Ƿ�������� j
	while (len--)
	{
		x = CryptFastMulMod(x, x, n);
		if (x == n - 1)
		{
			return 1;
		}
	}
	return 0;
}

//����n�Ƿ�������
static uint8_t CryptMillerRabin(CryptUint n)
{
	if (n == 2)
	{
		return 1;
	}

	//�����2�������������<2������>2��ż����������
	if (n < 2 || n % 2 == 0)
	{
		return 0;
	}

	//	��times���������
	for (uint8_t i = 1; i != 20; i++)
	{
		//�õ������������ a
		CryptUint a = CryptRandom(n - 2) + 1;
		//��a����n�Ƿ�������
		if (!CryptWitness(a, n))
		{
			return 0;
		}
	}
	return 1;
}

static CryptUint CryptGetPrime(CryptUint n)
{
	while (0 == CryptMillerRabin(n)) n--;
	return n;
}

#endif // __CRYPT_H__
