
#ifndef USBHID_HPP
#define USBHID_HPP

#include "../../FrameWork.hpp"

// ATL��CString
#include <atlstr.h> 

// UsbHid��ϢWM_DEVICECHANGE��֧��
#include <dbt.h>

// WIN32 DDK
#pragma comment(lib, "cfgmgr32.lib")
#include <cfgmgr32.h>

// HID_LIB
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

//#pragma comment(lib, "UsbHid\\hid.lib")
//#pragma comment(lib, "UsbHid\\setupapi.lib")

extern "C"
{
#ifndef WIN32
#include <hidsdi.h>
#include <setupapi.h>
#else
#include "UsbHid\hidsdi.h"
#include "UsbHid\setupapi.h"
#endif
}

// �趨�Ķ�д�������ܳ���(����ID Ϊ 1 + ��λ��UsbHid������ָ�������� 0x40)

#define	USB_MAX		0x40		// USB ���䳤�� �� Stm32.h ����һ��

#define USB_RX_MAX USB_MAX + 1
#define USB_TX_MAX USB_MAX + 1

class UsbHid
{
private:
    DEVINST DevParent;                              // �ҵ��� UsbHid �豸�ĸ����
    CString DevPath;								// �����ҵ���UsbHid·��
    HANDLE WriteThread, ReadThread;					// UsbHidWrite��Read�̵߳ľ��
    HANDLE WriteHandle, ReadHandle;					// UsbHidWrite��Read�ļ��ľ��
    OVERLAPPED WriteOverlap, ReadOverlap;			// Write��Read���ݵ��ص�IO
    DWORD ReadSize;									// ��һ�ζ�ȡ�������ݳ���
    UCHAR WriteBuf[USB_TX_MAX], ReadBuf[USB_RX_MAX];// Write��Read���ݵĻ�����
    DWORD LogSwitch;                                // ��־�������
    BOOL DevWriting;								// д������״̬
public:
    DWORD DevState;									// ��¼��ǰ��UsbHid״̬
    DWORD DevError;									// ��¼GetLastError����ֵ����UsbHid��Errorֵ����ֵ��
    CString DevDesc;                                // ��¼�豸����
    const DWORD TimeOut = 5000;	// UsbHid�򿪺��߳������ĳ�ʱʱ��

    // ����ΪUsbHid״̬��
    enum State
    {
        Close = 0x00, // δ��
        Find = 0x01, // �ѷ���
        Open = 0x02, // �Ѵ�
        Read = 0x04, // UsbHid�ɶ�
        Write = 0x08, // UsbHid��д
        UnInit = 0x10, // δ��ʼ��
    };

    enum Error
    {
        ErrNo = 0, // û�д���ERROR_SUCCESS��
        ErrMem = -1, // �����ڴ�ʧ�ܣ�����ö��ʧ�ܣ�
        ErrOpen = -2, // ��ʧ�ܣ�UsbHid���ɶ�д��
        ErrWriteTmOut = -3, // д�������ʱ
    };

    enum Log
    {
        Stop = 0x0,  // ͣ��
        Stream = 0x1,  // ��
        File = 0x2,  // �ļ�
    };

    UsbHid( )
    {
        DevState = State::UnInit;
        DevError = Error::ErrNo;
        LogSwitch = Log::Stop;
        DevWriting = FALSE;
    }

    ~UsbHid( )
    {
        if ( State::Close != DevState )
        {
            PutInfo( "���棺û����������UsbHid\n" );
            Cleanup( );
        }
    };

    virtual void UsbHid::UsbRecvProcess( UCHAR Buf[USB_MAX] )
    {
        PutInfo( "debug : %.*s\n", USB_MAX, Buf );
    }

    // ����: ����UsbHid, ����ֵ : DevState
    DWORD32 UsbHid::FindDev( DWORD32 Pid, DWORD32 Vid, DWORD32 Pvn )
    {
        PutInfo( "�������оٵ�ǰ�Ѵ��ڵ�UsbHid\n" );
        if ( State::Close != DevState )
        {
            PutInfo( "���� UsbHid ǰ��ȷ��DevStateΪClose״̬\n" );
            return DevState;	// ȷ��UsbHid״̬Ϊ�ر�
        }
        // ��ʼ����������
        HIDD_ATTRIBUTES DevAttr;							// ����UsbHid������
        DevAttr.Size = sizeof( DevAttr );						// ��DevAttr�ṹ���Size��ʼ��Ϊ�ṹ���С
        SP_DEVICE_INTERFACE_DATA DevInterfaceData;			// ����UsbHid�������ӿ���Ϣ
        DevInterfaceData.cbSize = sizeof( DevInterfaceData );	// ��DevInterfaceData�ṹ���cbSize��ʼ��Ϊ�ṹ���С
        GUID HidGuid;										// ����HIDUsbHid�Ľӿ���GUID
        HidD_GetHidGuid( &HidGuid );							// ����HidD_GetHidGuid������ȡHIDUsbHid��GUID����������HidGuid��

        HDEVINFO DevInfoSet;								// �����ȡ����UsbHid��Ϣ���Ͼ��
        // �ṩ��UsbHid��Ϣ�ṹ��ʹ�õĿ���ѡ�������������ֵ
        // DIGCF_PRESENT - ֻ���ص�ǰ���ڵ�UsbHid
        // DIGCF_ALLCLASSES - ���������Ѱ�װ��UsbHid��������־�����ˣ�ClassGuid������������
        // DIGCF_PROFILE - ֻ���ص�ǰӲ�������ļ��е�UsbHid
        // DIGCF_INTERFACEDEVICE - ��������֧�ֵ�UsbHid
        // DIGCF_DEFAULT - ֻ������ϵͳĬ��UsbHid��ص�UsbHid
        // ָ�� DIGCF_DEVICEINTERFACE | DIGCF_PRESENT ���ص�ǰӲ�������ļ��д��ڵ�UsbHid
        DevInfoSet = SetupDiGetClassDevs( &HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT );

        // ����ָ��UsbHid(ö��UsbHid����)
        if ( INVALID_HANDLE_VALUE != DevInfoSet )
        {
            // ����Ҫ����ȫ�����UsbHid���޸�ѭ���ж�(DevState == DEV_NO)������CString�����Լ���Ӧ�ķ���ȫ������
            DWORD DevIndex, DevRequiredSize;				// ��ǰö�ٵ�UsbHidID �� ����UsbHid��ϸ��Ϣ�Ļ��峤��

            for ( SetLastError( NO_ERROR ), DevIndex = 0; State::Close == DevState && ERROR_NO_MORE_ITEMS != GetLastError( ); DevIndex++ )
            {
                // ����SetupDiEnumDeviceInterfaces��UsbHid��Ϣ�����л�ȡ���ΪDevIndex��UsbHid��Ϣ
                if ( SetupDiEnumDeviceInterfaces( DevInfoSet, NULL, &HidGuid, DevIndex, &DevInterfaceData ) )
                {
                    // ͨ����һ�ε��ú���SetupDiGetDeviceInterfaceDetail����ȡUsbHid��Ϣ��Ҫ��󻺳���
                    SetupDiGetDeviceInterfaceDetail( DevInfoSet, &DevInterfaceData, NULL, NULL, &DevRequiredSize, NULL );
                    PSP_DEVICE_INTERFACE_DETAIL_DATA DevDetail;		// ָ��UsbHid��ϸ��Ϣ�Ľṹ��ָ��
                    DevDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc( DevRequiredSize );
                    if ( NULL != DevDetail )
                    {
                        DevDetail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );
                        // Ȼ���ٴε���SetupDiGetDeviceInterfaceDetail��������ȡUsbHid����ϸ��Ϣ��ε�������ʹ�õĻ������Լ���������С
                        if ( SetupDiGetDeviceInterfaceDetail( DevInfoSet, &DevInterfaceData, DevDetail, DevRequiredSize, NULL, NULL ) )
                        {
                            HANDLE DevHandle;								// �����UsbHid�ľ��
                            // ��ȡUsbHid·����CString
                            DevPath = DevDetail->DevicePath;
                            // ������óɹ�����ʹ�ò�����д���ʵ�CreateFile��������ȡUsbHid�����ԣ�����VID��PID���汾�ŵ�
                            // ���ڶ�ռUsbHid������USB���̣�,�޷�ʹ�ö����ʷ�ʽ�򿪣���ʹ�ò�����д���ʵĲſ��Դ򿪲���ȡUsbHid������
                            // ����򿪳ɹ������ȡUsbHid����, ���ɹ��ͼ���������һ��UsbHid
                            DevHandle = CreateFile( DevPath, NULL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
                            if ( INVALID_HANDLE_VALUE != DevHandle )
                            {
                                // ��ȡUsbHid�����Բ�������DevAttr�ṹ����,���رոմ򿪵�UsbHid������ȡʧ�ܣ�������һ��
                                if ( HidD_GetAttributes( DevHandle, &DevAttr ) )
                                {
                                    // �����ȡ�ɹ����������е�VID��PID�Լ�UsbHid�汾�����趨�ıȽϣ�һ����˵������������Ҫ�ҵ�UsbHid
                                    if ( DevAttr.VendorID == Vid && DevAttr.ProductID == Pid && DevAttr.VersionNumber == Pvn )
                                    {
                                        SP_DEVINFO_DATA DevInfoData;
                                        DevInfoData.cbSize = sizeof( SP_DEVINFO_DATA );
                                        if ( TRUE == SetupDiEnumDeviceInfo( DevInfoSet, DevIndex, &DevInfoData ) )
                                        {
                                            // ��ȡ�豸���״̬
                                            ULONG status, ProbNum;
                                            CONFIGRET cr = CM_Get_DevNode_Status( &status, &ProbNum, DevInfoData.DevInst, 0 );
                                            if ( CR_SUCCESS == cr )
                                            {
                                                // ��ȡ�豸�����
                                                cr = CM_Get_Parent( &DevParent, DevInfoData.DevInst, 0 );
                                                if ( CR_SUCCESS == cr )
                                                {
                                                    wchar_t Desc[0x7F] = { 0 };
                                                    HidD_GetProductString( DevHandle, Desc, sizeof( Desc ) );
                                                    DevDesc = Desc;         // �洢�豸������
                                                    PutInfo( "���������豸����Ϊ : %s\n", DevDesc.GetString( ) );
                                                    DevState = State::Find; // ����UsbHid״̬�Ѿ��ҵ�
                                                }
                                            }
                                        }
                                    }
                                    PutInfo( "ID:%02u PID:%04X VID:%04X PVN:%04X\n", DevIndex, DevAttr.ProductID, DevAttr.VendorID, DevAttr.VersionNumber );
                                }
                                CloseHandle( DevHandle );
                            }
                        }
                        free( DevDetail );
                    }
                    else
                    {
                        DevState = Error::ErrMem;
                    }
                }
            }
        }
        // ����SetupDiDestroyDeviceInfoList��������UsbHid��Ϣ����
        SetupDiDestroyDeviceInfoList( DevInfoSet );
        return DevState;
    }

    // ����: ��ʼ��UsbHid������������, ����ֵ : BOOL
    BOOL UsbHid::Startup( )
    {
        if ( State::UnInit == DevState )
        {
            PutInfo( "��ʼ��UsbHid����......" );
            // ��ʼ��UsbHid״ֵ̬
            DevState = State::Close;
            // ��ʼ�����ݻ�����
            ZeroMemory( WriteBuf, sizeof( WriteBuf ) );
            ZeroMemory( ReadBuf, sizeof( ReadBuf ) );
            // ��ʼ�����ݶ�д���
            ReadHandle = WriteHandle = INVALID_HANDLE_VALUE;
            // ��ʼ�����ݶ�д�ص�IO
            ZeroMemory( &ReadOverlap, sizeof( OVERLAPPED ) );
            ZeroMemory( &WriteOverlap, sizeof( OVERLAPPED ) );
            // ����һ���¼����ṩ��WriteFileʹ�ã���WriteFile���ʱ�����Զ����ø��¼�Ϊ����״̬
            WriteOverlap.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            if ( NULL != WriteOverlap.hEvent )
            {
                // ����һ���¼����ṩ��ReadFileʹ�ã���ReadFile���ʱ�����Զ����ø��¼�Ϊ����״̬
                ReadOverlap.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
                if ( NULL != ReadOverlap.hEvent )
                {
                    // ����һ�����߳�
                    ReadThread = CreateThread( NULL, 0, ReadEventThread, (LPVOID)this, 0, NULL );
                    if ( NULL != ReadThread )
                    {
                        PutInfo( "��ʼ���ɹ�!\n" );
                        return TRUE;
                    }
                    CloseHandle( ReadOverlap.hEvent );
                }
                CloseHandle( WriteOverlap.hEvent );
            }
            PutInfo( "��ʼ��ʧ��!\n" );
        }
        DevState = State::UnInit;
        return FALSE;
    }

    // ����: ����UsbHid������������, ����ֵ : ��
    void UsbHid::Cleanup( )
    {
        if ( State::UnInit != DevState )
        {
            // �رյ�ǰ����ʹ�õ�UsbHid
            CloseDev( );
            DevState = State::UnInit;
            // ����д�ص�IO
            CloseHandle( WriteOverlap.hEvent );
            // �����ص�IO�Ͷ��߳�
            SetEvent( ReadOverlap.hEvent ), CloseHandle( ReadOverlap.hEvent );
            if ( ReadThread != INVALID_HANDLE_VALUE )
            {
                WaitForSingleObject( ReadThread, INFINITE );
                CloseHandle( ReadThread );
            }
            PutInfo( "UsbHid����������!\n" );
        }
    }

    // ����: ��UsbHid, ����ֵ : DevState
    DWORD32 UsbHid::OpenDev( )
    {
        if ( State::Find & DevState )
        {
            DevState = State::Open;
            PutInfo( "���ҵ���UsbHid\n" );
            // DevPath�����Ų��ҵ���UsbHid���ֱ�ʹ�ö�д��ʽ��֮����������������ѡ��Ϊ�첽���ʷ�ʽ
            ReadHandle = CreateFile( DevPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
            if ( INVALID_HANDLE_VALUE != ReadHandle )
            {
                DevState |= State::Read;
                SetEvent( ReadOverlap.hEvent ); // ��ִ�ж��߳� (DevState == State::Open)
                PutInfo( "��ǰUsbHid�ɶ�\n" );
            }
            WriteHandle = CreateFile( DevPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
            if ( INVALID_HANDLE_VALUE != WriteHandle )
            {
                DevState |= State::Write;
                PutInfo( "��ǰUsbHid��д\n" );
            }
            PutInfo( "�ѳɹ���UsbHid\n" );
        }
        else
        {
            PutInfo( "���ڴ�ǰ����ָ����UsbHid\n" );
        }
        return DevState;
    }

    // ����: �ر�UsbHid, ����ֵ : ��
    void UsbHid::CloseDev( )
    {
        if ( DevState & State::Open )
        {
            // �رպ�ָ�Ϊö�ٺ�״̬
            DevState = State::Close;
            // ���ô�������
            DevError = Error::ErrNo;
            PutInfo( "UsbHid���ڹر�!\n" );
            // ��������ݵľ��������Ч�������ر�
            if ( INVALID_HANDLE_VALUE != ReadHandle )
            {
                int i = CloseHandle( ReadHandle );
                ReadHandle = INVALID_HANDLE_VALUE;
                // �����м䱻�رյ����жϵ�Ӳ���¼�
                SetEvent( ReadOverlap.hEvent );
            }
            // ���д���ݵľ��������Ч�������ر�
            if ( INVALID_HANDLE_VALUE != WriteHandle )
            {
                int i = CloseHandle( WriteHandle );
                WriteHandle = INVALID_HANDLE_VALUE;
                // �����м䱻�رյ����жϵ�Ӳ���¼�
                SetEvent( WriteOverlap.hEvent );
            }
        }
        else
        {
            PutInfo( "UsbHidδ��!\n" );
        }
    }

    static DWORD WINAPI UsbHid::ReadEventThread( LPVOID lpParam )
    {
        UsbHid * Self = (UsbHid *) lpParam;
        while ( State::UnInit != Self->DevState )
        {
            Self->ReadFileEvent( );
        }
        return 0;
    }

    // ��ȡӲ�����ݵ��߳�����ʹ�õ����첽���ã�����ڵ���ReadFile����ʱ�ṩһ��Overlapped�Ľṹ��
    // �ýṹ�к���һ���¼��ľ��ƽʱ���¼��Ǵ������ź�״̬�ģ�����ȴ��¼��ĺ����ͻᱻ����
    // �Ӷ����̱߳�������������ȷ���غ��¼����������ָ̻߳����в���鷵�ص�������
    void UsbHid::ReadFileEvent( )
    {
        if ( State::Open & DevState )
        {
            // �����Ч�����ReadFile�������� USB_RX_MAX �ֽڵı�������
            if ( INVALID_HANDLE_VALUE != ReadHandle )
            {
                // �����첽��ȡ�᷵�� 0 �� ���ô���Ϊ ERROR_IO_PENDING, ���޷����׼ȷ�Ĵ�����Ϣ
                ReadFile( ReadHandle, ReadBuf, USB_RX_MAX, NULL, &ReadOverlap );

                // ����ȴ�������UsbHid���γ���Ҳ�ᵼ���¼�����������ʱDevStateӦ�ñ�����Ϊ�٣�
                // ����������ж�DevStateΪ�ٵĻ������ô�״̬
                if ( State::Open & DevState )
                {
                    // ���UsbHidû�б����£�����ReadFile���������������
                    // ͨ��GetOverlappedResult��������ȡʵ�ʶ�ȡ�����ֽ���
                    GetOverlappedResult( ReadHandle, &ReadOverlap, &ReadSize, TRUE );
                    // �����ֽ���ΪReadSize���ʾ��������������ݣ�������UsbHid�쳣
                    if ( USB_RX_MAX == ReadSize )
                    {
                        UsbRecvProcess( ReadBuf + 1 );
                    }
                    else
                    {
                        PutError( "���ݶ�ȡ������, ����" );
                    }
                }
                // ������
                DevError = GetLastError( );
                switch ( DevError )
                {
                    case ERROR_IO_PENDING:
                    case ERROR_SUCCESS:
                        break;
                        // �ж��Ƿ�ΪUsbHid�Ͽ�����ֵ
                    case ERROR_DEVICE_NOT_CONNECTED:
                        PutError( "UsbHid���϶Ͽ������ر��豸" );
                        CloseDev( );
                        break;
                    default:
                        PutError( "���̷߳���δ֪����" );
                        break;
                }
            }
        }
        // ���UsbHidδ���������̣߳�ֱ���´ζ�ȡ�¼�������
        else
        {
            WaitForSingleObject( ReadOverlap.hEvent, INFINITE );
        }
    }

    // ���� Hid ����
    void UsbHid::ReSetDev( )
    {
        CONFIGRET cr;
        // �Ƴ��豸��������
        cr = CM_Query_And_Remove_SubTree( DevParent, NULL, NULL, 0, CM_REMOVE_NO_RESTART );
        if ( CR_SUCCESS == cr )
        {
            // �����豸���
            cr = CM_Setup_DevNode( DevParent, CM_SETUP_DEVINST_RESET );
            if ( CR_SUCCESS == cr )
            {
                // ˢ���豸��Ϣ��ɨ��Ӳ���Ķ���
                cr = CM_Reenumerate_DevInst( DevParent, CM_REENUMERATE_NORMAL );
                if ( CR_SUCCESS == cr )
                {
                    PutError( "������ USB ����!" );
                    // ���ô�������
                    DevError = Error::ErrNo;
                }
            }
        }
    }

    // ��UsbHidд�����ݣ�����ֵ : BOOL ��ϸ���������GetLastError() 
    BOOL UsbHid::WriteData( UCHAR * buf, UCHAR len )
    {
        try
        {
            // ���UsbHidû���ҵ����򷵻�ʧ��
            if ( State::Close == DevState ) throw "UsbHidδ��";
            // ��������Ч����˵����UsbHidʧ��
            if ( INVALID_HANDLE_VALUE == WriteHandle ) throw "д�����Ч,������UsbHid��ʧ��";
            if ( DevWriting )
            {
                // ��һ�ε�Writefile�Ĳ����ɹ��ᴥ���ص�IO�ڲ��󶨵��¼�
                switch ( WaitForSingleObject( WriteOverlap.hEvent, UsbHid::TimeOut ) )
                {
                    case WAIT_OBJECT_0:
                        // ������������ϴλ��������ú���д���������ִ��
                        ZeroMemory( WriteBuf, sizeof( WriteBuf ) );
                        DevWriting = FALSE;
                        break;
                    case WAIT_TIMEOUT:
                        // ��ʱ���˳�����ʱ����ܻ��쳣������ס��
                        DevError = Error::ErrWriteTmOut;
                        throw "UsbHidд������ʱ��д�������æ......��";
                }
            }
            DevWriting = TRUE;
            if ( len == 0 || len > USB_MAX ) len = USB_MAX;//  throw "д�����ݳ���Խ��";
            // �������ͻ�������ſ�ʼд���ݵ����ͻ�����
            memcpy( (char *) WriteBuf + 1, buf, len );
            // ����WriteFile����Write���������������ʧ�ܣ�����������ʧ�ܣ�Ҳ������IO������
            if ( WriteFile( WriteHandle, WriteBuf, USB_TX_MAX, NULL, &WriteOverlap ) ) return TRUE;
            // ��ȡ���������
            DevError = GetLastError( );
            switch ( GetLastError( ) )
            {
                // ȷ��IO�Ƿ���𣬷����׳�����
                case ERROR_IO_PENDING:
                case ERROR_SUCCESS:
                    return TRUE;
                    // ȷ��UsbHid�Ƿ񻹴�
                case ERROR_DEVICE_NOT_CONNECTED:
                    CloseDev( );
                    throw "UsbHid�ѶϿ�!";
                    // ���������Ϊ1��˵����UsbHid��֧�ָú���
                case ERROR_INVALID_FUNCTION:
                    throw "��UsbHid��֧��WriteFile����!";
                case ERROR_FILE_NOT_FOUND:
                    throw "ϵͳ�Ҳ���ָ����UsbHid";
                default:
                    throw "����δ֪����";
            }
        }
        catch ( LPSTR inform )
        {
			DevError = GetLastError();
            PutInfo( "error: %lu %s\n", DevError, inform );
            return FALSE;
        }

#ifdef UNIT_TEST
        static void UnitTest( )
        {
            UsbHid hid;
            if ( hid.Startup( ) )
            {
                if ( UsbHid::State::Find & hid.FindDev( 0x9990, 0x0666, 0x0200 ) )
                {
                    hid.ReSetDev( );
                    for ( size_t i = 0; i != UCHAR_MAX; i++ )
                    {
                        if ( UsbHid::State::Write & hid.OpenDev( ) )
                        {
                            for ( size_t j = 0; j != UCHAR_MAX; j++ )
                            {
                                hid.WriteData( (UCHAR *)"01234567890123456789012345678901234567890123456789012345678901234", 16 );
                                Sleep( 1 );
                            }
                            hid.CloseDev( );
                        }
                    }
                }
                hid.Cleanup( );
                return 0;
            }
        }
#endif 

    }

#endif // USBHID_HPP

};
