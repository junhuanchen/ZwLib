#ifndef ADO_MSSQL_HPP
#define ADO_MSSQL_HPP

// 定义传输采集中间件各项参数
extern "C"
{
    #include "../../Core/ZwTransit.h"
}

#define		IP_V4_LEN	16   // 采集端IPv4标识长度 127.000.000.001
#define		TIME_LEN	23   // 时间字符串长度 2017-01-02 03:04:05.300

#include "ado2.h"

#include "../../FrameWork.hpp"

struct ZwAdoMsSql : public CADODatabase
{
	ZwAdoMsSql()
	{

	}

#include "ZwAdoMsSql.hpp"

    bool ZwAdoMsSql::KvDataInsert( SHORT EntID, CString & DevName, SHORT DevPort, CString & DtSrcName, CString Data, CString & Time )
    {
        if ( CheckConnect( ) )
        {
            CADOCommand Cmd = CADOCommand( this, "KvDataInsert" );

            CADOParameter Result( DataTypeEnum::adInteger, sizeof( INT ), CADOParameter::paramReturnValue );
            Cmd.AddParameter( &Result );

            CADOParameter ParamDevName( DataTypeEnum::adVarChar );
            ParamDevName.SetValue( DevName ), Cmd.AddParameter( &ParamDevName );

            CADOParameter ParamDevPort( DataTypeEnum::adSmallInt );
            ParamDevPort.SetValue( DevPort ), Cmd.AddParameter( &ParamDevPort );

            CADOParameter ParamDtSrcName( DataTypeEnum::adVarChar );
            ParamDtSrcName.SetValue( DtSrcName ), Cmd.AddParameter( &ParamDtSrcName );

            CADOParameter ParamDataLen( DataTypeEnum::adSmallInt );
            ParamDataLen.SetValue( ((short) Data.GetLength( )) ), Cmd.AddParameter( &ParamDataLen );

            CADOParameter ParamData( DataTypeEnum::adVarChar );
            ParamData.SetValue( Data ), Cmd.AddParameter( &ParamData );

            CADOParameter ParamTime( DataTypeEnum::adVarChar );
            ParamTime.SetValue( Time ), Cmd.AddParameter( &ParamTime );

            if ( Cmd.Execute( ) )
            {
                int result = 0;
                Result.GetValue( result );
                PutInfo( "KvDataInsert Result:%d\n", result );
                return result;
            }
        }
        PutInfo( "Error:%s\n", GetLastErrorString( ).GetString( ) );
        return false;
    }

    bool ZwAdoMsSql::ZwDataInsert(UCHAR EntID, CString & DevName, UCHAR DevPort, CString & DtSrcName, CString Data, CString & Time )
    {
        if ( CheckConnect( ) )
        {
            CADOCommand Cmd = CADOCommand( this, "ZwDataInsert" );

            CADOParameter Result( DataTypeEnum::adInteger, sizeof( INT ), CADOParameter::paramReturnValue );
            Cmd.AddParameter( &Result );

            CADOParameter ParamEntID( DataTypeEnum::adSmallInt );
            ParamEntID.SetValue( EntID ), Cmd.AddParameter( &ParamEntID );

            CADOParameter ParamDevName( DataTypeEnum::adVarChar );
            ParamDevName.SetValue( DevName ), Cmd.AddParameter( &ParamDevName );

            CADOParameter ParamDevPort( DataTypeEnum::adSmallInt );
            ParamDevPort.SetValue( DevPort ), Cmd.AddParameter( &ParamDevPort );

            CADOParameter ParamDtSrcName( DataTypeEnum::adVarChar );
            ParamDtSrcName.SetValue( DtSrcName ), Cmd.AddParameter( &ParamDtSrcName );

            CADOParameter ParamData( DataTypeEnum::adVarChar );
            ParamData.SetValue( Data ), Cmd.AddParameter( &ParamData );

            CADOParameter ParamTime( DataTypeEnum::adVarChar );
            ParamTime.SetValue( Time ), Cmd.AddParameter( &ParamTime );

            if ( Cmd.Execute( ) )
            {
                int result = 0;
                Result.GetValue( result );
                PutInfo( "ZwDataInsert Result:%d\n", result );
                return !result;
            }
        }
        PutInfo( "Error:%s\n", GetLastErrorString( ).GetString( ) );
        return false;
    }

	void ExitConnect()
	{
		CADODatabase::Close();
	}

	LPCSTR SqlConfig;

	bool StartConnect(LPCSTR SqlConfig)
	{
		this->SqlConfig = SqlConfig;
		return CADODatabase::Open(SqlConfig);
	}

	bool CheckConnect()
	{
		if (!IsOpen())
		{
			return CADODatabase::Open(SqlConfig);
		}
		return true;
	}

#ifdef UNIT_TEST

    static void UnitTest( )
    {
        ZwAdoMsSql sql;
        if ( sql.Open( "File Name=SqlConfig.udl" ) )
        {
            time_t tt = time( NULL );
            tm * t = localtime( &tt );
            CString Time;
            Time.Format( "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, GetTickCount( ) % 1000 );
            sql.ZwDataInsert( 1, CString( "zw01" ), 0, CString( "DM123456" ), CString( "12345" ), Time );
            return 0;
        }
        else
        {
            PutInfo( "Error:%s\n", sql.GetLastErrorString( ) );
        }
        return FALSE;
    }

#endif // !UNIT_TEST

};

#endif
