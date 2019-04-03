
#ifndef _WIN_TDP_TO_SQL_
#define _WIN_TDP_TO_SQL_

#include "../DataInterface/Tdp/Tdp.hpp"
#include "../DataInterface/Ado/ZwAdoMsSql.hpp"
#include "WinTran.hpp"

class WinTranTdpToSql : public WinTranTask, public Tdp<WinTranPack>, public ZwAdoMsSql
{
    ZwEncode En;

    static void GetTime( uint32_t *sec, uint16_t *ms )
    {
        *sec = time( NULL ), *ms = GetTickCount64( ) % 1000;
    }

    const uint8_t RsaLocalKey = 3, RsaRemoteKey = 7;

public:

    void WinTranTdpToSql::TdpRecvProcess( TdpFrame & Packet )
    {
        // 应答
        PutInfo( "tdp - ip : %s port : %hu\n",
            inet_ntoa( Packet.Addr.sin_addr ), ntohs( Packet.Addr.sin_port ) );
        ZwDecode PackRecv;
        ZwDecodeInit( &PackRecv, RsaLocalKey );
        switch ( Packet.Var.buf[0] )
        {
            case ZwTranTypeCollect:
            {
                WinTranPack temp( Packet.Var );
                //uint16_t ms = 0, ent_id = 0, dev_ip = 0;
                //uint32_t sec = 0;
                //uint8_t dev_id[sizeof(WTUnPack.Zip.DevID)];
                uint8_t Srclen, SrcId[ZwSourceMax], Datalen, Data[ZwDataMax];
                // 解析协议数据
                if ( 1 == ZwDecodeCollect( &PackRecv, temp.buf, Packet.Len, &Srclen, SrcId, &Datalen, Data ) )
                {
                    PutInfo( "Collect\n" );
                    uint8_t ent_id = *(uint8_t *) PackRecv.Zip.EntID;
                    uint32_t time_ms = *(uint16_t *) PackRecv.Zip.DevMs;
                    time_t time_sec = *(uint32_t *) PackRecv.Zip.DevTm;
                    time_t tt = time( NULL );// time_sec;

                    if ( /*true || */tt - time_sec <= 10 )
                    {
                        // 时间符合要求即可应答。
                        SendTo( Packet.Addr, Packet.Var, Packet.Len );

                        tm * t = localtime( &time_sec );

                        uint8_t dev_ip = *(uint8_t *) PackRecv.Zip.DevIP;
                        CString dev;
                        dev.Format( "%X-%X-%X-%X-%X-%X", PackRecv.Zip.DevID[0], PackRecv.Zip.DevID[1],
                            PackRecv.Zip.DevID[2], PackRecv.Zip.DevID[3], PackRecv.Zip.DevID[4],
                            PackRecv.Zip.DevID[5] );
                        CString src;
                        src.Format( "%.*s", Srclen, SrcId );
                        CString data;
                        data.Format( "%.*s", Datalen, Data );

                        CString Tm;
						Tm.Format( "%04d-%02d-%02d %02d:%02d:%02d.%03hu",
                            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, time_ms );

                        LogPut( "%hhu-%s-%s-%hhu-%s-%s\n", ent_id, Tm.GetString(), dev, dev_ip, src, data );

                        try
                        {
                            ZwAdoMsSql::ZwDataInsert( ent_id, dev, dev_ip, src, data, Tm);
                        }
                        catch ( CADOException e )
                        {
                            break;
                        }
                    }
                    else
                    {
                        // 反馈时间同步指令
                        Packet.Len = ZwEncodeCommand( &this->En, (uint8_t *) Packet.Var.buf, sizeof( "TimeSysn" ) - 1, (uint8_t *)"TimeSysn" );
                        if ( 0 != Packet.Len )
                        {
                            uint32_t time = *(uint32_t *) En.Zip.DevTm;
                            PutInfo( "RequestCommandTimeSysn TimeSysn:%u\n", time );

                            SendTo( Packet.Addr, Packet.Var, Packet.Len );
                        }
                    }
                }
                else
                {
                    // 待解析数据异常，记录之
                }
                break;
            }
            case ZwTranTypeCommand:
            {
                uint8_t Cmd[ZwTranMax - ZwEncodeCoreLen], CmdLen;
                if ( 1 == ZwDecodeCommand( &PackRecv, (uint8_t *) Packet.Var.buf, Packet.Len, &CmdLen, Cmd ) )
                {
                    // 并行
                }
                break;
            }
        }
    }

    WinTranTdpToSql( )
    {
        ZwTranInit( );
        ZwEncodeInit( &En, RsaRemoteKey, 1, (uint8_t *) "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 0, GetTime );
        PutInfo( "WinTranTdpToSql Start!\n" );
    }

    void WinTranTdpToSql::BackgroundProcess( )
    {
        Sleep( 5 );
    }

    bool WinTranTdpToSql::StartUp( LPCSTR SqlConfig, USHORT LocalPort, USHORT ThreadSum )
    {
        if ( ZwAdoMsSql::StartConnect( SqlConfig ) )
        {
#ifdef _TDPCRITICALSECTION_
            // 启动Tdp服务
            if (Tdp::StartServer(LocalPort, ThreadSum))
#else
            if ( Tdp::StartServer( LocalPort ) )
#endif
            {
                // 启动后台线程服务
                if ( WinTranTask::Start( ) )
                {
                    return true;
                }
                else
                {
                    PutError( "后台线程服务启动失败，无法在系统中创建更多的线程" );
                }
                Tdp::StopServer( );
            }
            else
            {
                PutError( "UDP网络服务启动失败" );
            }
            ZwAdoMsSql::ExitConnect( );
        }
        else
        {
            PutError( "数据库服务连接失败" );
        }
        return false;
    }

    void WinTranTdpToSql::CleanUp( )
    {
        WinTranTask::Stop( );
        Tdp::StopServer( );
        ZwAdoMsSql::ExitConnect( );
    }

#ifdef UNIT_TEST

#pragma comment(lib,"user32.lib")

#define debug(_operator) printf("operator: %d\n", _operator)

    void UnitTest( )
    {
        char TmpBuf[0xFF] = { };
        // 加载服务器配置

        // 加载数据库配置

        WSADATA wsaData;
        // 启动网络环境
        if ( 0 == WSAStartup( MAKEWORD( 0x02, 0x02 ), &wsaData ) )
        {
            WinTranTdpToSql Sys;
            if ( Sys.StartUp( "File Name=sqlconfig.udl", 9954, 4 ) )
            {
                PutInfo( "数据采集主机服务启动成功，按 ESC 退出程序！\n" );
                while ( true )
                {
                    // 按下ESC可以退出程序
                    if ( GetAsyncKeyState( VK_ESCAPE ) )
                    {
                        break;
                    }
                    Sleep( 1000 );
                }
                Sys.CleanUp( );
            }
            WSACleanup( );
        }
        else
        {
            PutInfo( "WSAStartup服务环境异常，正尝试重置网络......\n" );
            system( "netsh winsock reset" );
        }
        return 0;
    }

#endif // !UNIT_TEST

};

#endif
