
#include "IoCp.hpp"

// IoData类成员静态函数指针类外声明
LPFN_ACCEPTEX IoData::Accept;
LPFN_DISCONNECTEX IoData::DisconnectEx;
LPFN_GETACCEPTEXSOCKADDRS IoData::GetAcceptSockAddrs;

BOOL IoCp::CheckCnct( IoData * Data )
{
    PutInfo( "客户端连接通过验证......" );
    return TRUE;
}

void IoCp::Business( IoData * Data )
{
    PutInfo( "服务端接收到了数据......" );
}

void IoCp::Worker( void )
{
    DWORD Byte = 0, Flag = 0;	// 存储GetQueuedCompletionStatus函数返回的结果
    OVERLAPPED * Overlap = NULL;// IoData 数据中的重叠IO指针
    PULONG_PTR CompleKey = NULL;// 完成端口队列绑定的外部数据指针，可传递外部变量
    while ( TRUE )
    {
        // 阻塞等待IoCp队列返回数据, 调用失败返回 0 (FASLE), 此时对方可能已经断开
        Flag = GetQueuedCompletionStatus( ComplePort, &Byte, (PULONG_PTR) &CompleKey, &Overlap, INFINITE );
        // 关闭IoCp端口导致获取的重叠IO与Host均为NULL, 表明发生了结束服务器事件
        if ( NULL == Overlap || FlagExit == ClientNum )
        {
            break; // 退出线程
        }
        // 根据结构体成员指针偏移量获取结构体指针(offset宏)
        IoData * ioData = CONTAINING_RECORD( Overlap, IoData, OverlapIO );
        // 连接异常的scoket情况有：
        // 1.Flag = 0 与对方的连接异常, 未连接即断开的scoket
        // 2.Byte = 0 对方正常断开连接后会发送空数据包
        if ( !Byte || !Flag )
        {
            if ( IoData::StateRecv == ioData->State )
            {
                PutInfo( "\t客户端被动断开!\n" );
                InterlockedDecrement( &ClientNum );
                ioData->Reuse( ListenPort );
            }
            else
            {
                PutInfo( "\t客户端主动断开!\n" );
            }
        }
        else if ( IoData::StateRecv == ioData->State )
        {
            PutInfo( "\t客户端接收数据!\n" );
            // 存储接收到的数据字节总数
            ioData->RecvByte = Byte;
            // 执行处理数据的逻辑函数
            Business( ioData );
            // 投递异步接收信息请求
            ioData->Recv( BusinessCmdLen );
        }
        // 目标客户端的连接后投递的消息进入连接态验证分支
        else if ( IoData::StateAccept == ioData->State )
        {
            // 验证投递的消息
            if ( CheckCnct( ioData ) )
            {
                PutInfo( "\t客户端建立连接!\n" );
                // 重新关联到IoCp队列中以继续接受异步接受请求（WSARecv）
                CreateIoCompletionPort( (HANDLE) ioData->Socket, ComplePort, (ULONG_PTR) NULL, 0 );
                // IoData进入接收态并投递一个预设的指令字节长度的异步接收数据请求
                ioData->State = IoData::StateRecv, ioData->Recv( BusinessCmdLen );
                // 用户连接数计数器增加（原子操作）,满了则生产。
                InterlockedIncrement( &ClientNum );
                if ( ClientNum == ClientSum )
                {
                    SetEvent( ProducerEvent );
                }
            }
            // 验证不通过则手动投递空包以表示主动断开与目标客户端的连接
            else
            {
                PostQueuedCompletionStatus( ComplePort, 0, (ULONG_PTR) NULL, &ioData->OverlapIO );
            }
        }
        else
        {
            PutError( "\t未知错误" );
        }
#ifdef LOG_OUT
        // 输出本线程执行对象和信息
        PSOCKADDR_IN clientAddr, localAddr;
        ioData->GetAddr(clientAddr, localAddr);
        PutInfo("ThreadID: %d P: %p IP:%s Port: %hu ClientNum: %d Check Error %d\n",
            GetCurrentThreadId(), ioData, inet_ntoa(clientAddr->sin_addr),
            htons(clientAddr->sin_port), ClientNum, WSAGetLastError());
#endif
    }
}

void IoCp::Producer( void )
{
    while ( FlagExit != ClientNum )	// 客户端数目变成退出指令时结束生产者线程,开始回收资源.
    {
        // 等待（阻塞）生产线程的ProducerEvent事件直到超时
        switch ( WaitForSingleObject( ProducerEvent, IoData::AliveCycle ) )
        {
            // 当工作线程执行SetEvent()时信号量亮起后进入该分支
            case WAIT_OBJECT_0:
            {
                if ( ClientNum == ClientSum )
                {
                    while ( ClientNum + PreBufClient > ClientSum )
                    {
                        IoData * ioData = ClientQueue.Get( );
                        if ( NULL != ioData && TRUE == ioData->Init( ListenPort ) )
                        {
                            InterlockedIncrement( &ClientSum );
                            continue;
                        }
                        break;
                    }
                }
                break;
            }
            // 等待超时后直接进入该分支
            case WAIT_TIMEOUT:
            {
                // 遍历循环队列检查IODATA的上次操作的时间是否超过生存周期，如果是则投递断开信号，防止恶意连接的客户端
                IoData * ioData = ClientQueue.Iterator( ), *End = ioData;
                do
                {
                    // 获取连接时长用以检查恶意连接的客户端（什么也不做的）
                    if ( IoData::StateAccept == ioData->State && ioData->OverTimeConnect( ) )
                    {
                        PutInfo( "\t生产者处理野生客户端的超时连接!\n" );
                        ioData->Reuse( ListenPort );
                    }
                    // 是接收态且最后一次操作超时，认为掉线出了故障（ACK 机制）
                    else if ( IoData::StateRecv == ioData->State && ioData->OverTimeLastExec( ) )
                    {
                        PutInfo( "\t生产者处理已知客户端的心跳连接!\n" );
                        InterlockedDecrement( &ClientNum );
                        ioData->Reuse( ListenPort );
                    }
                    else
                    {
                        ioData = ClientQueue.Iterator( );
                    }
                } while ( End != ioData );
                break;
            }
        }
    }
    // CloseHandle在win7以上相当于终止所有等待IoCp队列的事件,接着等待所有工作队列退出，然后才销毁IODATA资源
    CloseHandle( ComplePort ), WaitForMultipleObjects( ThreadSum, Threads, TRUE, INFINITE );
}

BOOL IoCp::StartUp( LONG threadI, USHORT port )
{
    ClientSum = ClientNum = FlagExit;
    // 初始化主机的sockaddr_in记录套接字类型和端口
    GUID GuidAcceptEx, GuidGetAcceptExSockAddrs, GuidDisconnectEx;
    DWORD dwBytes;									// WSAIoctl 执行结果
    Addr.sin_family = AF_INET;						// 使用的协议, 默认使用IPv4进行通信
    Addr.sin_port = ntohs( port );					// 监听的端口
    Addr.sin_addr.S_un.S_addr = htonl( INADDR_ANY );	// 监听的范围, Internet（TCP/IP）地址族
    // 主机的socket监听主机地址并绑定地址与端口
    ListenPort = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
    if ( INVALID_SOCKET != ListenPort )
    {
        if ( SOCKET_ERROR != bind( ListenPort, (sockaddr *) &Addr, sizeof( sockaddr ) ) )
        {
            if ( SOCKET_ERROR != listen( ListenPort, SOMAXCONN ) )
            {
                if ( TRUE == ClientQueue.New( ) )
                {
                    // 创建生产线程信号事件
                    ProducerEvent = CreateEvent( NULL, FALSE, TRUE, NULL );
                    if ( NULL != ProducerEvent )
                    {
                        // 创建IoCp队列
                        ComplePort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 0 );
                        if ( INVALID_HANDLE_VALUE != ComplePort )
                        {
                            // ListenPort关联到主机的IoCp队列
                            if ( NULL != CreateIoCompletionPort( (HANDLE) ListenPort, ComplePort, (ULONG_PTR) NULL, 0 ) )
                            {
                                // 根据ListenPort获取拓展的AcceptEx和GetAcceptExSockAddrs和DisconnectEx函数指针, 目的为避免切换上下文消耗资源
                                GuidDisconnectEx = WSAID_DISCONNECTEX;
                                if ( 0 == WSAIoctl( ListenPort, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidDisconnectEx, sizeof( GuidDisconnectEx ), &IoData::DisconnectEx, sizeof( IoData::DisconnectEx ), &dwBytes, NULL, NULL ) )
                                {
                                    GuidAcceptEx = WSAID_ACCEPTEX;
                                    if ( 0 == WSAIoctl( ListenPort, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof( GuidAcceptEx ), &IoData::Accept, sizeof( IoData::Accept ), &dwBytes, NULL, NULL ) )
                                    {
                                        GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
                                        if ( 0 == WSAIoctl( ListenPort, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs, sizeof( GuidGetAcceptExSockAddrs ), &IoData::GetAcceptSockAddrs, sizeof( IoData::GetAcceptSockAddrs ), &dwBytes, NULL, NULL ) )
                                        {
                                            // 初始化线程数组索引并申请线程句柄数组
                                            ThreadSum = threadI, Threads = new HANDLE[ThreadSum + 2];
                                            // 初始化主机连接客户端数目和缓存最大数目
                                            ClientSum = ClientNum = 0;
                                            if ( NULL != Threads )
                                            {
                                                // 创建工作线程
                                                for ( threadI = 0; threadI < ThreadSum; threadI++ )
                                                {
                                                    Threads[threadI] = CreateThread( NULL, 0, TaskWorker, (LPVOID)this, 0, 0 );
                                                    if ( NULL == Threads[threadI] ) break;
                                                }
                                                if ( threadI == ThreadSum )
                                                {
                                                    // 创建生产线程
                                                    Threads[ThreadSum] = CreateThread( NULL, 0, TaskProducer, (LPVOID)this, 0, 0 );
                                                    if ( NULL != Threads[ThreadSum] )
                                                    {
                                                        PutInfo( "IoCp StartUp Success!\n" );
                                                        return TRUE;
                                                    }
                                                }
                                                // 回收生成到一半的线程池句柄资源
                                                while ( --threadI >= 0 ) CloseHandle( Threads[threadI] );
                                                delete[] Threads;
                                            }
                                            PutInfo( "Get WSAID_GETACCEPTEXSOCKADDRS Function Failed! " );
                                        }
                                        PutInfo( "Get WSAID_ACCEPTEX Function Failed! " );
                                    }
                                    PutInfo( "Get WSAID_DISCONNECTEX Function Failed! " );
                                }
                                PutInfo( "Create Producer Thread Failed! " );
                            }
                            if ( NULL != ComplePort ) CloseHandle( ComplePort );
                        }
                        PutInfo( "CreateIoCompletionPort Failed! " );
                    }
                    PutInfo( "KernelEvent Failed! " );
                    ClientQueue.Del( );
                }
            }
            PutInfo( "Bind Failed! " );
        }
        closesocket( ListenPort );
        PutInfo( "Listen Failed!\n" );
    }
    PutInfo( "Check Error: %d\n", WSAGetLastError( ) );
    return FALSE;
}

void IoCp::CleanUp( )
{
    // 远程调用和本地调用只能执行一次
    if ( ClientNum != FlagExit )
    {
        // 设置终止生产线程的结束标记
        ClientNum = FlagExit;
        // 避免生产线程被挂起
        SetEvent( ProducerEvent );
        // 等待全部生产线程的结束
        WaitForSingleObject( Threads[ThreadSum], INFINITE );
        // 回收线程池句柄的资源
        for ( LONG i = ThreadSum; i >= 0; i-- )
        {
            CloseHandle( Threads[i] );
        }
        delete[] Threads;
        CloseHandle( ProducerEvent );
        closesocket( ListenPort );
        // 销毁队列，回收队列内所有IoData
        ClientQueue.Del( );
        PutInfo( "IoCp CleanUp Success!\n" );
    }
}

#ifdef UNIT_TEST

// 解析命令声明
enum CMD
{
    FILE_TYPE = 'F',
    RECV_TYPE = 'R',
    ERROR_TYPE = 'G',
    KEY_TYPE = 'E'
};
class HostIocp : public IoCp
{
    void Business(IoData * ioData)
    {
        PutInfo("处理业务操作: ");
        switch ( ioData->Buffer.buf[0] )
        {
            case FILE_TYPE:
            {
                ioData->Buffer.buf[ioData->RecvByte] = '\0';
                PutInfo("接收到的数据: %s\n", ioData->Buffer.buf);
                break;
            }
            case RECV_TYPE:
            {
                ioData->Buffer.buf[ioData->RecvByte] = '\0';
                PutInfo("接收到的数据: %s\n", ioData->Buffer.buf);
                break;
            }
            case ERROR_TYPE:
            {
                PutInfo("请求获取错误!\n");
                INT error = WSAGetLastError( );
                send(ioData->Sock, (CHAR *)&error, sizeof(INT), 0);
                break;
            }
            default:
            {
                ioData->Buffer.buf[ioData->RecvByte] = '\0';
                PutInfo("%d个无法识别的字节数据: %s\n", ioData->RecvByte, ioData->Buffer.buf);
                break;
            }
        }

        PutInfo("操作结果 : %d\n", WSAGetLastError( ));
    }
    BOOL CheckCnct(IoData * ioData)
    {
        PutInfo("登录验证%s!\n", (ioData->Buffer.buf[0] == KEY_TYPE) ? "成功" : "失败");
        return ioData->Buffer.buf[0] == KEY_TYPE;
    }
};

int main( )
{
    WSADATA wsaData;
    //Sleep(100);
    if ( 0 == WSAStartup(MAKEWORD(0x02, 0x02), &wsaData) )
    {
        while ( TRUE )
        {
            HostIocp Localhost;
            system("pause");
            //Sleep(100);
            if ( Localhost.StartUp(0x4, 8087) )
            {
                //Sleep(100);
                system("pause");
                Localhost.CleanUp( );
            }
        }
    }
    WSACleanup( );
    return 0;
}
#endif

// TODO
#ifdef UNIT_TEST

#include "TcpClient.hpp"

#pragma comment(lib, "Mswsock.lib ")

class UnitTestFileServer : public IoCp
{

protected:

    BOOL CheckCnct(IoData * Data)
    {
        PutInfo("客户端连接通过验证......");
        return TRUE;
    }

    void Business(IoData * Data)
    {
        PutInfo("服务端接收到了数据......");
        switch (Data->State)
        {
            default:

                HANDLE hFile = CreateFile("test.log", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

                TransmitFile(Data->Socket, hFile, 0, 0, NULL, NULL, TF_DISCONNECT);

                CloseHandle(hFile);

                break;
        }
    }

public:

    void Main()
    {
        WSADATA wsaData;
        if (0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
            if (1 == StartUp(1, 9954))
            {
                UnitTestFileClient FileClient;
                if (FileClient.Startup())
                {
                    FileClient.Connect("127.0.0.1", 9954);

                    while (true)
                    {
                        if (FileClient.ConnectAlive())
                        {
                            // FileClient.Send("Request File", sizeof("Request File"), -1);
                        }
                    }
                }
                system("pause > nul");
            }
            WSACleanup();
        }
    }
};

int main()
{
    UnitTestFileServer Ts;
    Ts.Main();
    return 0;
}

#endif