
#include "ZwParse.h"

#include "ZwData.h"

void KvNanoSourceNext( ZwSource *Source ) // 目前仅 进行 十进制 的数据源尾部数值自加
{
	uint8_t pos = Source->Len - 1;
	for ( Source->Data[pos]++; Source->Data[pos] > '9'; Source->Data[--pos]++ ) Source->Data[pos] = '0';
}

bool KvNanoGetSource( uint8_t *Buffer, ZwSource *Source)
{
	uint8_t pos = 0;
	while ( '\0' != Buffer[pos] && Buffer[pos] != '\r' && Buffer[pos] != '.' && Buffer[pos] != ' ' && pos <= ZwDataMax )
	{
		Source->Data[pos] = Buffer[pos], pos++;
	}
	Source->Len = pos;
	return (bool)pos;
}

typedef char KvNanoType;
typedef uint8_t KvNanoTypeLen;

KvNanoType KvNanoGetDefaultType( ZwSource *Source )
{
	uint8_t *pos = Source->Data;
    switch (*pos++)
    {
        case 'D':
			switch ( *pos++ )
            {
                case 'M':
                    return 'U';
                default:
                    return '\0';
            }
        case 'Z':
            return 'U';
        case 'T':
			switch ( *pos++ )
            {
                case 'M':
                    return 'U';
                case 'C':
                case 'S':
                default:
                    return 'D';
            }
        case 'W':
            return 'U';
        case 'V':
			switch ( *pos++ )
            {
                case 'B':
                    return 'B';
                case 'M':
                    return 'U';
                default:
                    return '\0';
            }
        case 'C':
			switch ( *pos++ )
            {
                case 'R':
                    return 'B';
                case 'M':
                    return 'U';
                case 'T':
					switch ( *pos++ )
                    {
                        case 'C':
                        case 'H':
                            return 'D';
                        default:
                            return '\0';
                    }
                case 'C':
                case 'S':
                default:
                    return 'D';
            }
        case 'M':
			switch ( *pos++ )
            {
                case 'R':
                    return 'B';
                default:
                    return '\0';
            }
        case 'L':
			switch ( *pos++ )
            {
                case 'R':
                    return 'B';
                default:
                    return '\0';
            }
        case 'R':
        case 'B':
        default:
            return 'B';
    }
}

KvNanoTypeLen KvNanoGetTypeLen( const KvNanoType Type )
{
	switch ( Type )
	{
		case 'B':  return 1;
		case 'H':  return 4;
		case 'U':  return 5;
		case 'S':  return 6;
		case 'D': return 10;
		case 'L': return 11;
		default:   return 0;
	}
}

enum ZwParseFlag KvNanoRD( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen )
{
	ZwMaxSource source;
	if ( true == KvNanoGetSource( Buffer, &source.Self ) ) // 获得当前软元件源编号
	{
		uint8_t buffer_pos = source.Self.Len;
		uint8_t data_len = KvNanoGetTypeLen( ( '.' == Buffer[buffer_pos] ) ? buffer_pos += 2, Buffer[buffer_pos - 1] : KvNanoGetDefaultType( &source.Self ) );
		if ( data_len && *DataLen >= ZwParseHeadSize + source.Self.Len + data_len )
		{
			if ( Buffer[buffer_pos] == '\r' && Buffer[buffer_pos + 1] == '\n' )
			{
				buffer_pos += 2; // "\r\n"
				uint8_t end_pos = buffer_pos + data_len;
				if ( Buffer[end_pos] == '\r' && Buffer[end_pos + 1] == '\n' )
				{
					Data[ZwParseSumOffset] = 1;
					Data[ZwParseSourceLenOffset] = source.Self.Len;
					Data[ZwParseTypeLenOffset] = data_len;
					uint8_t pos = ZwParseArrayOffset;
					memcpy( Data + pos, source.Self.Data, source.Self.Len ), pos += Data[ZwParseSourceLenOffset];
					memcpy( Data + pos, Buffer + buffer_pos, data_len ), pos += Data[ZwParseTypeLenOffset];
					// printf( "Source %.*s Data %.*s\n", source.Self.Len, source.Self.Data, data_len, Buffer + buffer_pos );
					return ZwParseOk;
				}
			}
			return ZwParseErr;
		}
		*DataLen = ZwParseHeadSize + source.Self.Len + data_len;
	}
	return ZwParseNo;
}

/**
 * @brief 把字符串转换成无符号8位整型
 * @remark 仅支持转换满足0~255的数值字符串，例如："123", "56"
 * @param[in] str 目标字符串起始指针
 * @param[in | out] pos 字符起始索引（整型
 * @param[out] var 存储转换得到的变量
 * @return 无
 * @see atoi
 */
#define zw_parse_atouc(str, pos, var)\
	for (var = 0; str[pos] >= '0' && str[pos] <= '9'; pos++) var = var * 10 + (str[pos] - '0')
 
enum ZwParseFlag KvNanoRDS( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen )
{
	ZwMaxSource source;
	if ( true == KvNanoGetSource( Buffer, &source.Self ) ) // 获得当前软元件源编号
	{
		uint8_t buffer_pos = source.Self.Len;
		uint8_t data_len = KvNanoGetTypeLen( ( '.' == Buffer[buffer_pos] ) ? buffer_pos += 2, Buffer[buffer_pos - 1] : KvNanoGetDefaultType( &source.Self ) );
		if ( data_len && ' ' == Buffer[buffer_pos] )
		{
			buffer_pos += 1;
			uint8_t data_sum = 0;
			zw_parse_atouc( Buffer, buffer_pos, data_sum ); // 十进制字符串数值转二进制原始数据
			if ( *DataLen >= ZwParseHeadSize + data_sum *( source.Self.Len + data_len ) )
			{
				if ( Buffer[buffer_pos] == '\r' && Buffer[buffer_pos + 1] == '\n' )
				{
					buffer_pos += 2;
					uint8_t end_pos = buffer_pos + data_sum * ( data_len + 1 ) - 1;
					if ( Buffer[end_pos] == '\r' && Buffer[end_pos + 1] == '\n' )
					{
						Data[ZwParseSumOffset] = data_sum;
						Data[ZwParseSourceLenOffset] = source.Self.Len;
						Data[ZwParseTypeLenOffset] = data_len;
						uint8_t pos = ZwParseArrayOffset;
						while ( data_sum )
						{
							memcpy( Data + pos, source.Self.Data, source.Self.Len ), pos += Data[ZwParseSourceLenOffset];
							memcpy( Data + pos, Buffer + buffer_pos, data_len ), pos += Data[ZwParseTypeLenOffset];
							// printf( "Source %.*s Data %.*s\n", source.Self.Len, source.Self.Data, data_len, Buffer + buffer_pos );
							KvNanoSourceNext( &source.Self );
							buffer_pos += ( data_len + 1/*' '*/ );
							data_sum--;
						}
						return ZwParseOk;
					}
				}
			}
			*DataLen = ZwParseHeadSize + data_sum *( source.Self.Len + data_len );
			return ZwParseErr;
		}
	}
	return ZwParseNo;
}

enum ZwParseFlag  KvNanoCheckReturn( uint8_t Buffer[], uint8_t BufLen )
{
	uint8_t *pos = Buffer;
	while ( *pos && '\n' != *pos++ );
	switch ( *pos++ )
	{
		case 'O':
		{
			switch ( *pos++ )
			{
				case 'K':
					return ZwParseOk;
			}
			break;
		}
	}
	return ZwParseNo;
}

bool ZwParseCheckKvNano( uint8_t Buffer[], uint8_t BufLen )
{
	return Buffer[BufLen - 1] == '\r' && Buffer[BufLen] == '\n';
}

enum ZwParseFlag ZwParseKvNano( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen )
{
	uint8_t * pos = Buffer;
	switch ( *pos++ )
	{
		case 'R':
		{
			switch ( *pos++ )
			{
				case 'D':
				{
					switch ( *pos++ )
					{
						case ' ':
							return KvNanoRD( Buffer + 3, BufLen, Data, DataLen );
						case 'S':
						{
							switch ( *pos++ )
							{
								case ' ':
									return KvNanoRDS( Buffer + 4, BufLen, Data, DataLen );
							}
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case 'W':
		{
			switch ( *pos++ )
			{
				case 'R':
				{
					switch ( *pos++ )
					{
						case ' ':
							return KvNanoCheckReturn( Buffer, BufLen );
						case 'S':
							switch ( *pos++ )
							{
								case ' ':
									return KvNanoCheckReturn( Buffer, BufLen );
							}
							break;
					}
					break;
				}
			}
			break;
		}
	}
	return ZwParseNo;
}

enum ZwParseFlag ZwParseHostLinkCache(uint8_t Buffer[], uint8_t BufLen, ZwCache *ZwCache)
{
	char *request, *respond;
	request = strstr((const char *)Buffer, "0104") + 4;
	if (NULL != request && NULL != strstr(request, "*\r@"))
	{
		respond = strstr(request, "0104") + 8;
		if (NULL != respond && NULL != strstr(respond, "*\r"))
		{
			uint8_t *source = (uint8_t *)request;
			uint8_t *data = (uint8_t *)respond;

			while (0 == memcmp(source, data, 2))
			{
				if (ZwCacheErr == ZwCacheUpdateEvent(ZwCache, 8, source, 4, data + 2))
				{
					return ZwParseErr;
				}
				source += 8, data += 6;
			}
			return ZwParseOk;
		}
	}
	return ZwParseNo;
}

enum ZwParseFlag HostLinkRDS( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen )
{
	char *request, *respond;
	request = strstr( ( const char * ) Buffer, "0104" ) + 4;
	if ( NULL != request && NULL != strstr( request, "*\r@" ) )
	{
		respond = strstr( request, "0104" ) + 8;
		if ( NULL != respond && NULL != strstr( respond, "*\r" ) )
		{
			uint8_t *source = (uint8_t *)request;
			uint8_t *data = (uint8_t *)respond;

			while ( 0 == memcmp( request, respond, 2 ) )
			{
				Data[ZwParseSumOffset]++, request += 8, respond += 6;
			}

			if ( *DataLen >= ZwParseHeadSize + Data[ZwParseSumOffset] * ( 8 + 4 ) )
			{
				Data[ZwParseSourceLenOffset] = 8;
				Data[ZwParseTypeLenOffset] = 4;
				uint8_t pos = ZwParseArrayOffset;
				while ( 0 == memcmp( source, data, 2 ) )
				{
					// printf( "Source %.*s Data %.*s\n", 8, source, 4, ( ( uint8_t * ) data ) + 2 );
					memcpy( Data + pos, source, 8 ), pos += 8;
					memcpy( Data + pos, data + 2, 4 ), pos += 4;
					source += 8, data += 6;
				}
				return ZwParseOk;
			}
			*DataLen = ZwParseHeadSize + Data[ZwParseSumOffset] * ( 8 + 4 );
			return ZwParseErr;
		}
	}
	return ZwParseNo;
}

bool ZwParseCheckHostLink( uint8_t Buffer[], uint8_t BufLen )
{
	return '@' == Buffer[0] && '\r' == Buffer[BufLen];
}

enum ZwParseFlag ZwParseHostLink( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen )
{
	uint8_t *pos = Buffer;
	switch ( *pos++ )
	{
		case '@':
			return HostLinkRDS(Buffer, BufLen, Data, DataLen);
	}
	return ZwParseNo;
}

#ifdef UNIT_TEST

#include <assert.h>

void UnitTestKvNanoDefaultType()
{

#define S(Value) {sizeof(Value), Value} 

#define AssertDefaultType(KeyData, DefaultType) { MapKey key = S( #KeyData ); assert( DefaultType == KvNanoGetDefaultType( &key ) ); }

	AssertDefaultType( R, 'B' );
	AssertDefaultType( B, 'B' );
	AssertDefaultType( MR, 'B' );
	AssertDefaultType( LR, 'B' );
	AssertDefaultType( CR, 'B' );
	AssertDefaultType( VB, 'B' );
	AssertDefaultType( DM, 'U' );
	AssertDefaultType( W, 'U' );
	AssertDefaultType( TM, 'U' );
	AssertDefaultType( Z, 'U' );
	AssertDefaultType( CM, 'U' );
	AssertDefaultType( VM, 'U' );
	AssertDefaultType( T, 'D' );
	AssertDefaultType( TC, 'D' );
	AssertDefaultType( TS, 'D' );
	AssertDefaultType( C, 'D' );
	AssertDefaultType( CC, 'D' );
	AssertDefaultType( CS, 'D' );
	AssertDefaultType( CTH, 'D' );
	AssertDefaultType( CTC, 'D' );
}

#include <stdio.h>
#include <stdlib.h>

void UnitTestKvNano()
{
	const char *KvNanoTestCase[] =
	{
		"RD DM00510.S\r\n+00100\r\n",
		"RD DM00510\r\n00300\r\n",
		"RD DM0051.L\r\n00000100000\r\n",
		"RD DM00510\r\n00300\r\n",
		"RD DM00510.S\r\n+00300\r\n",
		"RDS DM00510 04\r\n00000 00002 00001 00004\r\n",
		"RDS DM00514.S 02\r\n+00001 -00002\r\n",
		"RDS DM00510 04\r\n00001 00002 00003 00004\r\n",
		"RDS DM00514.S 02\r\n+00001 -00002\r\n",
		"WR DM00510 00000\r\nOK\r\n",
		"WR DM00510 00000\r\nOK\r\n",
		"WRS R100 2 0 1\r\nOK\r\n",
		"WRS R100 2 0 1\r\nOK\r\n",
	};

	for ( size_t i = 0; i < sizeof( KvNanoTestCase ) / sizeof( &KvNanoTestCase[0] ); i++ )
	{
		uint8_t *buffer = KvNanoTestCase[i], buflen = strlen( KvNanoTestCase[i] );
		uint8_t kv_nano_data[64] = { 0 }, kv_nano_data_len = sizeof( kv_nano_data );
		assert( true == ZwParseCheckKvNano( buffer, buflen ) );
		assert( ZwParseOk == ZwParseKvNano( buffer, buflen, kv_nano_data, &kv_nano_data_len ) );
	
		puts( KvNanoTestCase[i] );

		uint8_t array_sum = kv_nano_data[ZwParseSumOffset];

		uint8_t source_len = kv_nano_data[ZwParseSourceLenOffset];

		uint8_t data_len = kv_nano_data[ZwParseTypeLenOffset];

		uint8_t *array_pos = kv_nano_data + ZwParseArrayOffset;

		for ( size_t j = 0; j < array_sum; j++ )
		{
			printf( "parse : source %.*s data %.*s \n", source_len, array_pos, data_len, array_pos + source_len );
			array_pos += data_len + source_len;
		}
	}
}

void UnitTestHostLink()
{
	const char *HostLinkTestCase[] =
	{
		"@00FA0800002000000000000000104B0000000B0006400B10000008200000003*\r@00FA00C000020000000000000001040000B00100B00000B1000082000C38*\r",
	};

	for ( size_t i = 0; i < sizeof( HostLinkTestCase ) / sizeof( &HostLinkTestCase[0] ); i++ )
	{
		uint8_t *buffer = HostLinkTestCase[i], buflen = strlen( HostLinkTestCase[i] );
		uint8_t host_link_data[64] = { 0 }, host_link_data_len = sizeof( host_link_data );
		assert( true == ZwParseCheckHostLink( buffer, buflen ) );
		assert( ZwParseOk == ZwParseHostLink( buffer, buflen, host_link_data, &host_link_data_len ) );

		puts( HostLinkTestCase[i] );
		
		uint8_t array_sum = host_link_data[ZwParseSumOffset];

		uint8_t source_len = host_link_data[ZwParseSourceLenOffset];

		uint8_t data_len = host_link_data[ZwParseTypeLenOffset];

		uint8_t *array_pos = host_link_data + ZwParseArrayOffset;

		for ( size_t j = 0; j < array_sum; j++ )
		{
			printf( "parse : source %.*s data %.*s \n", source_len, array_pos, data_len, array_pos + source_len );
			array_pos += data_len + source_len;
		}
	}

}

#endif
