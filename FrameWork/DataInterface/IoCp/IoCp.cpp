
#include "IoCp.hpp"

// IoData���Ա��̬����ָ����������
LPFN_ACCEPTEX IoData::Accept;
LPFN_DISCONNECTEX IoData::DisconnectEx;
LPFN_GETACCEPTEXSOCKADDRS IoData::GetAcceptSockAddrs;

BOOL IoCp::CheckCnct( IoData * Data )
{
    PutInfo( "�ͻ�������ͨ����֤......" );
    return TRUE;
}

void IoCp::Business( IoData * Data )
{
    PutInfo( "����˽��յ�������......" );
}

void IoCp::Worker( void )
{
    DWORD Byte = 0, Flag = 0;	// �洢GetQueuedCompletionStatus�������صĽ��
    OVERLAPPED * Overlap = NULL;// IoData �����е��ص�IOָ��
    PULONG_PTR CompleKey = NULL;// ��ɶ˿ڶ��а󶨵��ⲿ����ָ�룬�ɴ����ⲿ����
    while ( TRUE )
    {
        // �����ȴ�IoCp���з�������, ����ʧ�ܷ��� 0 (FASLE), ��ʱ�Է������Ѿ��Ͽ�
        Flag = GetQueuedCompletionStatus( ComplePort, &Byte, (PULONG_PTR) &CompleKey, &Overlap, INFINITE );
        // �ر�IoCp�˿ڵ��»�ȡ���ص�IO��Host��ΪNULL, ���������˽����������¼�
        if ( NULL == Overlap || FlagExit == ClientNum )
        {
            break; // �˳��߳�
        }
        // ���ݽṹ���Աָ��ƫ������ȡ�ṹ��ָ��(offset��)
        IoData * ioData = CONTAINING_RECORD( Overlap, IoData, OverlapIO );
        // �����쳣��scoket����У�
        // 1.Flag = 0 ��Է��������쳣, δ���Ӽ��Ͽ���scoket
        // 2.Byte = 0 �Է������Ͽ����Ӻ�ᷢ�Ϳ����ݰ�
        if ( !Byte || !Flag )
        {
            if ( IoData::StateRecv == ioData->State )
            {
                PutInfo( "\t�ͻ��˱����Ͽ�!\n" );
                InterlockedDecrement( &ClientNum );
                ioData->Reuse( ListenPort );
            }
            else
            {
                PutInfo( "\t�ͻ��������Ͽ�!\n" );
            }
        }
        else if ( IoData::StateRecv == ioData->State )
        {
            PutInfo( "\t�ͻ��˽�������!\n" );
            // �洢���յ��������ֽ�����
            ioData->RecvByte = Byte;
            // ִ�д������ݵ��߼�����
            Business( ioData );
            // Ͷ���첽������Ϣ����
            ioData->Recv( BusinessCmdLen );
        }
        // Ŀ��ͻ��˵����Ӻ�Ͷ�ݵ���Ϣ��������̬��֤��֧
        else if ( IoData::StateAccept == ioData->State )
        {
            // ��֤Ͷ�ݵ���Ϣ
            if ( CheckCnct( ioData ) )
            {
                PutInfo( "\t�ͻ��˽�������!\n" );
                // ���¹�����IoCp�������Լ��������첽��������WSARecv��
                CreateIoCompletionPort( (HANDLE) ioData->Socket, ComplePort, (ULONG_PTR) NULL, 0 );
                // IoData�������̬��Ͷ��һ��Ԥ���ָ���ֽڳ��ȵ��첽������������
                ioData->State = IoData::StateRecv, ioData->Recv( BusinessCmdLen );
                // �û����������������ӣ�ԭ�Ӳ�����,������������
                InterlockedIncrement( &ClientNum );
                if ( ClientNum == ClientSum )
                {
                    SetEvent( ProducerEvent );
                }
            }
            // ��֤��ͨ�����ֶ�Ͷ�ݿհ��Ա�ʾ�����Ͽ���Ŀ��ͻ��˵�����
            else
            {
                PostQueuedCompletionStatus( ComplePort, 0, (ULONG_PTR) NULL, &ioData->OverlapIO );
            }
        }
        else
        {
            PutError( "\tδ֪����" );
        }
#ifdef LOG_OUT
        // ������߳�ִ�ж������Ϣ
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
    while ( FlagExit != ClientNum )	// �ͻ�����Ŀ����˳�ָ��ʱ�����������߳�,��ʼ������Դ.
    {
        // �ȴ��������������̵߳�ProducerEvent�¼�ֱ����ʱ
        switch ( WaitForSingleObject( ProducerEvent, IoData::AliveCycle ) )
        {
            // �������߳�ִ��SetEvent()ʱ�ź�����������÷�֧
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
            // �ȴ���ʱ��ֱ�ӽ���÷�֧
            case WAIT_TIMEOUT:
            {
                // ����ѭ�����м��IODATA���ϴβ�����ʱ���Ƿ񳬹��������ڣ��������Ͷ�ݶϿ��źţ���ֹ�������ӵĿͻ���
                IoData * ioData = ClientQueue.Iterator( ), *End = ioData;
                do
                {
                    // ��ȡ����ʱ�����Լ��������ӵĿͻ��ˣ�ʲôҲ�����ģ�
                    if ( IoData::StateAccept == ioData->State && ioData->OverTimeConnect( ) )
                    {
                        PutInfo( "\t�����ߴ���Ұ���ͻ��˵ĳ�ʱ����!\n" );
                        ioData->Reuse( ListenPort );
                    }
                    // �ǽ���̬�����һ�β�����ʱ����Ϊ���߳��˹��ϣ�ACK ���ƣ�
                    else if ( IoData::StateRecv == ioData->State && ioData->OverTimeLastExec( ) )
                    {
                        PutInfo( "\t�����ߴ�����֪�ͻ��˵���������!\n" );
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
    // CloseHandle��win7�����൱����ֹ���еȴ�IoCp���е��¼�,���ŵȴ����й��������˳���Ȼ�������IODATA��Դ
    CloseHandle( ComplePort ), WaitForMultipleObjects( ThreadSum, Threads, TRUE, INFINITE );
}

BOOL IoCp::StartUp( LONG threadI, USHORT port )
{
    ClientSum = ClientNum = FlagExit;
    // ��ʼ��������sockaddr_in��¼�׽������ͺͶ˿�
    GUID GuidAcceptEx, GuidGetAcceptExSockAddrs, GuidDisconnectEx;
    DWORD dwBytes;									// WSAIoctl ִ�н��
    Addr.sin_family = AF_INET;						// ʹ�õ�Э��, Ĭ��ʹ��IPv4����ͨ��
    Addr.sin_port = ntohs( port );					// �����Ķ˿�
    Addr.sin_addr.S_un.S_addr = htonl( INADDR_ANY );	// �����ķ�Χ, Internet��TCP/IP����ַ��
    // ������socket����������ַ���󶨵�ַ��˿�
    ListenPort = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
    if ( INVALID_SOCKET != ListenPort )
    {
        if ( SOCKET_ERROR != bind( ListenPort, (sockaddr *) &Addr, sizeof( sockaddr ) ) )
        {
            if ( SOCKET_ERROR != listen( ListenPort, SOMAXCONN ) )
            {
                if ( TRUE == ClientQueue.New( ) )
                {
                    // ���������߳��ź��¼�
                    ProducerEvent = CreateEvent( NULL, FALSE, TRUE, NULL );
                    if ( NULL != ProducerEvent )
                    {
                        // ����IoCp����
                        ComplePort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 0 );
                        if ( INVALID_HANDLE_VALUE != ComplePort )
                        {
                            // ListenPort������������IoCp����
                            if ( NULL != CreateIoCompletionPort( (HANDLE) ListenPort, ComplePort, (ULONG_PTR) NULL, 0 ) )
                            {
                                // ����ListenPort��ȡ��չ��AcceptEx��GetAcceptExSockAddrs��DisconnectEx����ָ��, Ŀ��Ϊ�����л�������������Դ
                                GuidDisconnectEx = WSAID_DISCONNECTEX;
                                if ( 0 == WSAIoctl( ListenPort, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidDisconnectEx, sizeof( GuidDisconnectEx ), &IoData::DisconnectEx, sizeof( IoData::DisconnectEx ), &dwBytes, NULL, NULL ) )
                                {
                                    GuidAcceptEx = WSAID_ACCEPTEX;
                                    if ( 0 == WSAIoctl( ListenPort, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof( GuidAcceptEx ), &IoData::Accept, sizeof( IoData::Accept ), &dwBytes, NULL, NULL ) )
                                    {
                                        GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
                                        if ( 0 == WSAIoctl( ListenPort, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs, sizeof( GuidGetAcceptExSockAddrs ), &IoData::GetAcceptSockAddrs, sizeof( IoData::GetAcceptSockAddrs ), &dwBytes, NULL, NULL ) )
                                        {
                                            // ��ʼ���߳����������������߳̾������
                                            ThreadSum = threadI, Threads = new HANDLE[ThreadSum + 2];
                                            // ��ʼ���������ӿͻ�����Ŀ�ͻ��������Ŀ
                                            ClientSum = ClientNum = 0;
                                            if ( NULL != Threads )
                                            {
                                                // ���������߳�
                                                for ( threadI = 0; threadI < ThreadSum; threadI++ )
                                                {
                                                    Threads[threadI] = CreateThread( NULL, 0, TaskWorker, (LPVOID)this, 0, 0 );
                                                    if ( NULL == Threads[threadI] ) break;
                                                }
                                                if ( threadI == ThreadSum )
                                                {
                                                    // ���������߳�
                                                    Threads[ThreadSum] = CreateThread( NULL, 0, TaskProducer, (LPVOID)this, 0, 0 );
                                                    if ( NULL != Threads[ThreadSum] )
                                                    {
                                                        PutInfo( "IoCp StartUp Success!\n" );
                                                        return TRUE;
                                                    }
                                                }
                                                // �������ɵ�һ����̳߳ؾ����Դ
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
    // Զ�̵��úͱ��ص���ֻ��ִ��һ��
    if ( ClientNum != FlagExit )
    {
        // ������ֹ�����̵߳Ľ������
        ClientNum = FlagExit;
        // ���������̱߳�����
        SetEvent( ProducerEvent );
        // �ȴ�ȫ�������̵߳Ľ���
        WaitForSingleObject( Threads[ThreadSum], INFINITE );
        // �����̳߳ؾ������Դ
        for ( LONG i = ThreadSum; i >= 0; i-- )
        {
            CloseHandle( Threads[i] );
        }
        delete[] Threads;
        CloseHandle( ProducerEvent );
        closesocket( ListenPort );
        // ���ٶ��У����ն���������IoData
        ClientQueue.Del( );
        PutInfo( "IoCp CleanUp Success!\n" );
    }
}

#ifdef UNIT_TEST

// ������������
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
        PutInfo("����ҵ�����: ");
        switch ( ioData->Buffer.buf[0] )
        {
            case FILE_TYPE:
            {
                ioData->Buffer.buf[ioData->RecvByte] = '\0';
                PutInfo("���յ�������: %s\n", ioData->Buffer.buf);
                break;
            }
            case RECV_TYPE:
            {
                ioData->Buffer.buf[ioData->RecvByte] = '\0';
                PutInfo("���յ�������: %s\n", ioData->Buffer.buf);
                break;
            }
            case ERROR_TYPE:
            {
                PutInfo("�����ȡ����!\n");
                INT error = WSAGetLastError( );
                send(ioData->Sock, (CHAR *)&error, sizeof(INT), 0);
                break;
            }
            default:
            {
                ioData->Buffer.buf[ioData->RecvByte] = '\0';
                PutInfo("%d���޷�ʶ����ֽ�����: %s\n", ioData->RecvByte, ioData->Buffer.buf);
                break;
            }
        }

        PutInfo("������� : %d\n", WSAGetLastError( ));
    }
    BOOL CheckCnct(IoData * ioData)
    {
        PutInfo("��¼��֤%s!\n", (ioData->Buffer.buf[0] == KEY_TYPE) ? "�ɹ�" : "ʧ��");
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
        PutInfo("�ͻ�������ͨ����֤......");
        return TRUE;
    }

    void Business(IoData * Data)
    {
        PutInfo("����˽��յ�������......");
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