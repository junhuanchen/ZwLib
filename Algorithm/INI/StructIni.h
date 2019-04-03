#ifndef STRUCT_INI_H
#define STRUCT_INI_H

#include <stdint.h>
#include <stdbool.h>

typedef void(*ExternRead) (uint16_t Pos, uint8_t * Dest);
typedef void(*ExternWrite) (uint16_t Pos, uint8_t Src);

typedef struct struct_ini
{
    uint32_t Max, Min;
    ExternRead Read;
    ExternWrite Write;
}StructIni;

static void StructIniInit(StructIni * Self, uint32_t Min, uint32_t Max, ExternWrite Write, ExternRead Read)
{
    Self->Min = Min, Self->Max = Max, Self->Write = Write, Self->Read = Read;
}

static void StructIniReadOne(StructIni * Self, uint16_t Pos, uint8_t * Dest)
{
    Self->Read(Pos + Self->Min, Dest);
}

static bool StructIniWriteOne(StructIni * Self, uint16_t Pos, uint8_t Src)
{
	uint8_t bak = 0;
    Self->Write(Pos + Self->Min, Src);
    Self->Read(Pos + Self->Min, &bak);
	return bak == Src;
}

bool StructIniWriteStruct(StructIni * Self, uint16_t Pos, uint8_t * Struct, uint16_t Size)
{
	// 验证哈希 地址索引
	uint8_t verify, i = (Size + sizeof(verify));
	uint16_t start = Pos * i; // 结构体索引 
    if (start + i <= Self->Max)
	{
		for (i = verify = 0; i != Size; i++)
		{
			verify += Struct[i];
            if (false == StructIniWriteOne(Self, start + i, Struct[i]))
			{
				return false;
			}
		}
        return StructIniWriteOne(Self, start + i, verify);
	}
	return false;
}

uint8_t StructIniReadStruct(StructIni * Self, uint16_t Pos, uint8_t * Struct, uint16_t Size)
{
	// 验证哈希 地址索引
	uint8_t verify, i = (Size + sizeof(verify));
	uint16_t start = Pos * i; // 结构体索引 
    if (start + i <= Self->Max)
	{
		for (i = verify = 0; i != Size; i++)
		{
            StructIniReadOne(Self, start + i, &Struct[i]);
			verify += Struct[i];
		}
		uint8_t tmp = 0;
        StructIniReadOne(Self, start + i, &tmp);
		return tmp == verify;
	}
	return false;

}

#endif

#ifdef UNIT_TEST

#include <stdio.h>
#include <assert.h>

StructIni ArrayIni;

static uint8_t Array[27];

static void ArrayRead(uint16_t Pos, uint8_t * Dest)
{
    *Dest = Array[Pos];
}

static void ArrayWrite(uint16_t Pos, uint8_t Src)
{
    Array[Pos] = Src;
}

struct config
{
    int a;
	int b;
};

void Put(struct config * cfg)
{
	printf("Px%p 0x%X 0x%X\n", cfg, cfg->a, cfg->b);
}

int main()
{
    StructIniInit(&ArrayIni, 0, sizeof(Array), ArrayWrite, ArrayRead);

	struct config a = { -1, -2 }, b = { -3, -4 }, c = { -5, -6 };

	Put(&a), Put(&b), Put(&c);

    assert(true == StructIniWriteStruct(&ArrayIni, 0, &a, sizeof(a)));
    assert(true == StructIniWriteStruct(&ArrayIni, 1, &b, sizeof(b)));
    assert(true == StructIniWriteStruct(&ArrayIni, 2, &c, sizeof(c)));

    assert(true == StructIniReadStruct(&ArrayIni, 2, &a, sizeof(a)));
    assert(true == StructIniReadStruct(&ArrayIni, 0, &b, sizeof(b)));
    assert(true == StructIniReadStruct(&ArrayIni, 1, &c, sizeof(c)));

	Put(&a), Put(&b), Put(&c);

    assert(false == StructIniWriteStruct(&ArrayIni, sizeof(Array), &a, sizeof(a)));

    assert(false == StructIniWriteStruct(&ArrayIni, sizeof(Array) / (sizeof(a) + 1), &a, sizeof(a)));

    assert(true == StructIniReadStruct(&ArrayIni, 2, &a, sizeof(a)));

    Array[2 * (sizeof(a) + 1) + 2] = 0;

    assert(false == StructIniReadStruct(&ArrayIni, 2, &a, sizeof(a)));

	return 0;
}

#endif
