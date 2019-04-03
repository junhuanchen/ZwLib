
#ifndef USBHID_HPP
#define USBHID_HPP

#include "../../FrameWork.hpp"

// ATL的CString
#include <atlstr.h> 

// UsbHid消息WM_DEVICECHANGE的支持
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

// 设定的读写缓冲区总长度(报告ID 为 1 + 下位机UsbHid描述符指定缓冲区 0x40)

#define	USB_MAX		0x40		// USB 传输长度 与 Stm32.h 保持一致

#define USB_RX_MAX USB_MAX + 1
#define USB_TX_MAX USB_MAX + 1

class UsbHid
{
private:
    DEVINST DevParent;                              // 找到的 UsbHid 设备的父结点
    CString DevPath;								// 保存找到的UsbHid路径
    HANDLE WriteThread, ReadThread;					// UsbHidWrite和Read线程的句柄
    HANDLE WriteHandle, ReadHandle;					// UsbHidWrite和Read文件的句柄
    OVERLAPPED WriteOverlap, ReadOverlap;			// Write与Read数据的重叠IO
    DWORD ReadSize;									// 上一次读取到的数据长度
    UCHAR WriteBuf[USB_TX_MAX], ReadBuf[USB_RX_MAX];// Write与Read数据的缓冲区
    DWORD LogSwitch;                                // 日志输出开关
    BOOL DevWriting;								// 写入数据状态
public:
    DWORD DevState;									// 记录当前的UsbHid状态
    DWORD DevError;									// 记录GetLastError（正值）和UsbHid的Error值（负值）
    CString DevDesc;                                // 记录设备描述
    const DWORD TimeOut = 5000;	// UsbHid打开后线程阻塞的超时时间

    // 以下为UsbHid状态量
    enum State
    {
        Close = 0x00, // 未打开
        Find = 0x01, // 已发现
        Open = 0x02, // 已打开
        Read = 0x04, // UsbHid可读
        Write = 0x08, // UsbHid可写
        UnInit = 0x10, // 未初始化
    };

    enum Error
    {
        ErrNo = 0, // 没有错误（ERROR_SUCCESS）
        ErrMem = -1, // 申请内存失败（导致枚举失败）
        ErrOpen = -2, // 打开失败（UsbHid不可读写）
        ErrWriteTmOut = -3, // 写入操作超时
    };

    enum Log
    {
        Stop = 0x0,  // 停用
        Stream = 0x1,  // 流
        File = 0x2,  // 文件
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
            PutInfo( "警告：没有主动销毁UsbHid\n" );
            Cleanup( );
        }
    };

    virtual void UsbHid::UsbRecvProcess( UCHAR Buf[USB_MAX] )
    {
        PutInfo( "debug : %.*s\n", USB_MAX, Buf );
    }

    // 功能: 搜索UsbHid, 返回值 : DevState
    DWORD32 UsbHid::FindDev( DWORD32 Pid, DWORD32 Vid, DWORD32 Pvn )
    {
        PutInfo( "搜索并列举当前已存在的UsbHid\n" );
        if ( State::Close != DevState )
        {
            PutInfo( "查找 UsbHid 前请确认DevState为Close状态\n" );
            return DevState;	// 确认UsbHid状态为关闭
        }
        // 初始化检索数据
        HIDD_ATTRIBUTES DevAttr;							// 保存UsbHid的属性
        DevAttr.Size = sizeof( DevAttr );						// 对DevAttr结构体的Size初始化为结构体大小
        SP_DEVICE_INTERFACE_DATA DevInterfaceData;			// 保存UsbHid的驱动接口信息
        DevInterfaceData.cbSize = sizeof( DevInterfaceData );	// 对DevInterfaceData结构体的cbSize初始化为结构体大小
        GUID HidGuid;										// 保存HIDUsbHid的接口类GUID
        HidD_GetHidGuid( &HidGuid );							// 调用HidD_GetHidGuid函数获取HIDUsbHid的GUID，并保存在HidGuid中

        HDEVINFO DevInfoSet;								// 保存获取到的UsbHid信息集合句柄
        // 提供在UsbHid信息结构中使用的控制选项可以是以下数值
        // DIGCF_PRESENT - 只返回当前存在的UsbHid
        // DIGCF_ALLCLASSES - 返回所有已安装的UsbHid如果这个标志设置了，ClassGuid参数将被忽略
        // DIGCF_PROFILE - 只返回当前硬件配置文件中的UsbHid
        // DIGCF_INTERFACEDEVICE - 返回所有支持的UsbHid
        // DIGCF_DEFAULT - 只返回与系统默认UsbHid相关的UsbHid
        // 指定 DIGCF_DEVICEINTERFACE | DIGCF_PRESENT 返回当前硬件配置文件中存在的UsbHid
        DevInfoSet = SetupDiGetClassDevs( &HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT );

        // 查找指定UsbHid(枚举UsbHid集合)
        if ( INVALID_HANDLE_VALUE != DevInfoSet )
        {
            // 若需要搜索全部相关UsbHid则修改循环判断(DevState == DEV_NO)并增设CString数组以及对应的访问全局索引
            DWORD DevIndex, DevRequiredSize;				// 当前枚举的UsbHidID 与 保存UsbHid详细信息的缓冲长度

            for ( SetLastError( NO_ERROR ), DevIndex = 0; State::Close == DevState && ERROR_NO_MORE_ITEMS != GetLastError( ); DevIndex++ )
            {
                // 调用SetupDiEnumDeviceInterfaces在UsbHid信息集合中获取编号为DevIndex的UsbHid信息
                if ( SetupDiEnumDeviceInterfaces( DevInfoSet, NULL, &HidGuid, DevIndex, &DevInterfaceData ) )
                {
                    // 通过第一次调用函数SetupDiGetDeviceInterfaceDetail来获取UsbHid信息需要多大缓冲区
                    SetupDiGetDeviceInterfaceDetail( DevInfoSet, &DevInterfaceData, NULL, NULL, &DevRequiredSize, NULL );
                    PSP_DEVICE_INTERFACE_DETAIL_DATA DevDetail;		// 指向UsbHid详细信息的结构体指针
                    DevDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc( DevRequiredSize );
                    if ( NULL != DevDetail )
                    {
                        DevDetail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );
                        // 然后再次调用SetupDiGetDeviceInterfaceDetail函数来获取UsbHid的详细信息这次调用设置使用的缓冲区以及缓冲区大小
                        if ( SetupDiGetDeviceInterfaceDetail( DevInfoSet, &DevInterfaceData, DevDetail, DevRequiredSize, NULL, NULL ) )
                        {
                            HANDLE DevHandle;								// 保存打开UsbHid的句柄
                            // 提取UsbHid路径到CString
                            DevPath = DevDetail->DevicePath;
                            // 如果调用成功，则使用不带读写访问的CreateFile函数来获取UsbHid的属性，包括VID、PID、版本号等
                            // 对于独占UsbHid（例如USB键盘）,无法使用读访问方式打开，需使用不带读写访问的才可以打开并获取UsbHid的属性
                            // 如果打开成功，则获取UsbHid属性, 不成功就继续查找下一个UsbHid
                            DevHandle = CreateFile( DevPath, NULL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
                            if ( INVALID_HANDLE_VALUE != DevHandle )
                            {
                                // 获取UsbHid的属性并保存在DevAttr结构体中,并关闭刚打开的UsbHid，若获取失败，查找下一个
                                if ( HidD_GetAttributes( DevHandle, &DevAttr ) )
                                {
                                    // 如果获取成功，则将属性中的VID、PID以及UsbHid版本号与设定的比较，一致则说明它就是我们要找的UsbHid
                                    if ( DevAttr.VendorID == Vid && DevAttr.ProductID == Pid && DevAttr.VersionNumber == Pvn )
                                    {
                                        SP_DEVINFO_DATA DevInfoData;
                                        DevInfoData.cbSize = sizeof( SP_DEVINFO_DATA );
                                        if ( TRUE == SetupDiEnumDeviceInfo( DevInfoSet, DevIndex, &DevInfoData ) )
                                        {
                                            // 获取设备结点状态
                                            ULONG status, ProbNum;
                                            CONFIGRET cr = CM_Get_DevNode_Status( &status, &ProbNum, DevInfoData.DevInst, 0 );
                                            if ( CR_SUCCESS == cr )
                                            {
                                                // 获取设备父结点
                                                cr = CM_Get_Parent( &DevParent, DevInfoData.DevInst, 0 );
                                                if ( CR_SUCCESS == cr )
                                                {
                                                    wchar_t Desc[0x7F] = { 0 };
                                                    HidD_GetProductString( DevHandle, Desc, sizeof( Desc ) );
                                                    DevDesc = Desc;         // 存储设备描述符
                                                    PutInfo( "搜索到的设备描述为 : %s\n", DevDesc.GetString( ) );
                                                    DevState = State::Find; // 设置UsbHid状态已经找到
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
        // 调用SetupDiDestroyDeviceInfoList函数销毁UsbHid信息集合
        SetupDiDestroyDeviceInfoList( DevInfoSet );
        return DevState;
    }

    // 功能: 初始化UsbHid函数操作环境, 返回值 : BOOL
    BOOL UsbHid::Startup( )
    {
        if ( State::UnInit == DevState )
        {
            PutInfo( "初始化UsbHid环境......" );
            // 初始化UsbHid状态值
            DevState = State::Close;
            // 初始化数据缓冲区
            ZeroMemory( WriteBuf, sizeof( WriteBuf ) );
            ZeroMemory( ReadBuf, sizeof( ReadBuf ) );
            // 初始化数据读写句柄
            ReadHandle = WriteHandle = INVALID_HANDLE_VALUE;
            // 初始化数据读写重叠IO
            ZeroMemory( &ReadOverlap, sizeof( OVERLAPPED ) );
            ZeroMemory( &WriteOverlap, sizeof( OVERLAPPED ) );
            // 创建一个事件，提供给WriteFile使用，当WriteFile完成时，会自动设置该事件为触发状态
            WriteOverlap.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            if ( NULL != WriteOverlap.hEvent )
            {
                // 创建一个事件，提供给ReadFile使用，当ReadFile完成时，会自动设置该事件为触发状态
                ReadOverlap.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
                if ( NULL != ReadOverlap.hEvent )
                {
                    // 创建一个读线程
                    ReadThread = CreateThread( NULL, 0, ReadEventThread, (LPVOID)this, 0, NULL );
                    if ( NULL != ReadThread )
                    {
                        PutInfo( "初始化成功!\n" );
                        return TRUE;
                    }
                    CloseHandle( ReadOverlap.hEvent );
                }
                CloseHandle( WriteOverlap.hEvent );
            }
            PutInfo( "初始化失败!\n" );
        }
        DevState = State::UnInit;
        return FALSE;
    }

    // 功能: 销毁UsbHid函数操作环境, 返回值 : 无
    void UsbHid::Cleanup( )
    {
        if ( State::UnInit != DevState )
        {
            // 关闭当前正在使用的UsbHid
            CloseDev( );
            DevState = State::UnInit;
            // 销毁写重叠IO
            CloseHandle( WriteOverlap.hEvent );
            // 销毁重叠IO和读线程
            SetEvent( ReadOverlap.hEvent ), CloseHandle( ReadOverlap.hEvent );
            if ( ReadThread != INVALID_HANDLE_VALUE )
            {
                WaitForSingleObject( ReadThread, INFINITE );
                CloseHandle( ReadThread );
            }
            PutInfo( "UsbHid环境已销毁!\n" );
        }
    }

    // 功能: 打开UsbHid, 返回值 : DevState
    DWORD32 UsbHid::OpenDev( )
    {
        if ( State::Find & DevState )
        {
            DevState = State::Open;
            PutInfo( "打开找到的UsbHid\n" );
            // DevPath保存着查找到的UsbHid，分别使用读写方式打开之，并保存其句柄并且选择为异步访问方式
            ReadHandle = CreateFile( DevPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
            if ( INVALID_HANDLE_VALUE != ReadHandle )
            {
                DevState |= State::Read;
                SetEvent( ReadOverlap.hEvent ); // 可执行读线程 (DevState == State::Open)
                PutInfo( "当前UsbHid可读\n" );
            }
            WriteHandle = CreateFile( DevPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
            if ( INVALID_HANDLE_VALUE != WriteHandle )
            {
                DevState |= State::Write;
                PutInfo( "当前UsbHid可写\n" );
            }
            PutInfo( "已成功打开UsbHid\n" );
        }
        else
        {
            PutInfo( "请在打开前搜索指定的UsbHid\n" );
        }
        return DevState;
    }

    // 功能: 关闭UsbHid, 返回值 : 无
    void UsbHid::CloseDev( )
    {
        if ( DevState & State::Open )
        {
            // 关闭后恢复为枚举后状态
            DevState = State::Close;
            // 重置错误数据
            DevError = Error::ErrNo;
            PutInfo( "UsbHid正在关闭!\n" );
            // 如果读数据的句柄不是无效句柄，则关闭
            if ( INVALID_HANDLE_VALUE != ReadHandle )
            {
                int i = CloseHandle( ReadHandle );
                ReadHandle = INVALID_HANDLE_VALUE;
                // 重置中间被关闭导致中断的硬件事件
                SetEvent( ReadOverlap.hEvent );
            }
            // 如果写数据的句柄不是无效句柄，则关闭
            if ( INVALID_HANDLE_VALUE != WriteHandle )
            {
                int i = CloseHandle( WriteHandle );
                WriteHandle = INVALID_HANDLE_VALUE;
                // 重置中间被关闭导致中断的硬件事件
                SetEvent( WriteOverlap.hEvent );
            }
        }
        else
        {
            PutInfo( "UsbHid未打开!\n" );
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

    // 读取硬件数据的线程由于使用的是异步调用，因而在调用ReadFile函数时提供一个Overlapped的结构，
    // 该结构中含有一个事件的句柄平时该事件是处于无信号状态的，因而等待事件的函数就会被挂起，
    // 从而该线程被阻塞当数据正确返回后，事件被触发，线程恢复运行并检查返回的数据量
    void UsbHid::ReadFileEvent( )
    {
        if ( State::Open & DevState )
        {
            // 句柄有效则调用ReadFile函数请求 USB_RX_MAX 字节的报告数据
            if ( INVALID_HANDLE_VALUE != ReadHandle )
            {
                // 由于异步读取会返回 0 和 设置错误为 ERROR_IO_PENDING, 即无法获得准确的错误信息
                ReadFile( ReadHandle, ReadBuf, USB_RX_MAX, NULL, &ReadOverlap );

                // 如果等待过程中UsbHid被拔出，也会导致事件触发，但此时DevState应该被设置为假，
                // 因此在这里判断DevState为假的话就重置打开状态
                if ( State::Open & DevState )
                {
                    // 如果UsbHid没有被拔下，则是ReadFile函数正常操作完成
                    // 通过GetOverlappedResult函数来获取实际读取到的字节数
                    GetOverlappedResult( ReadHandle, &ReadOverlap, &ReadSize, TRUE );
                    // 接收字节数为ReadSize则表示获得了完整的数据，否则是UsbHid异常
                    if ( USB_RX_MAX == ReadSize )
                    {
                        UsbRecvProcess( ReadBuf + 1 );
                    }
                    else
                    {
                        PutError( "数据读取不完整, 请检查" );
                    }
                }
                // 检查错误
                DevError = GetLastError( );
                switch ( DevError )
                {
                    case ERROR_IO_PENDING:
                    case ERROR_SUCCESS:
                        break;
                        // 判断是否为UsbHid断开错误值
                    case ERROR_DEVICE_NOT_CONNECTED:
                        PutError( "UsbHid故障断开即将关闭设备" );
                        CloseDev( );
                        break;
                    default:
                        PutError( "读线程发生未知错误" );
                        break;
                }
            }
        }
        // 如果UsbHid未打开则阻塞线程，直到下次读取事件被触发
        else
        {
            WaitForSingleObject( ReadOverlap.hEvent, INFINITE );
        }
    }

    // 重置 Hid 驱动
    void UsbHid::ReSetDev( )
    {
        CONFIGRET cr;
        // 移除设备但不重启
        cr = CM_Query_And_Remove_SubTree( DevParent, NULL, NULL, 0, CM_REMOVE_NO_RESTART );
        if ( CR_SUCCESS == cr )
        {
            // 重置设备结点
            cr = CM_Setup_DevNode( DevParent, CM_SETUP_DEVINST_RESET );
            if ( CR_SUCCESS == cr )
            {
                // 刷新设备信息（扫描硬件改动）
                cr = CM_Reenumerate_DevInst( DevParent, CM_REENUMERATE_NORMAL );
                if ( CR_SUCCESS == cr )
                {
                    PutError( "已重置 USB 驱动!" );
                    // 重置错误数据
                    DevError = Error::ErrNo;
                }
            }
        }
    }

    // 对UsbHid写入数据，返回值 : BOOL 详细错误请调用GetLastError() 
    BOOL UsbHid::WriteData( UCHAR * buf, UCHAR len )
    {
        try
        {
            // 如果UsbHid没有找到，则返回失败
            if ( State::Close == DevState ) throw "UsbHid未打开";
            // 如果句柄无效，则说明打开UsbHid失败
            if ( INVALID_HANDLE_VALUE == WriteHandle ) throw "写句柄无效,可能是UsbHid打开失败";
            if ( DevWriting )
            {
                // 上一次的Writefile的操作成功会触发重叠IO内部绑定的事件
                switch ( WaitForSingleObject( WriteOverlap.hEvent, UsbHid::TimeOut ) )
                {
                    case WAIT_OBJECT_0:
                        // 传输完成清理上次缓冲区，让后续写入操作继续执行
                        ZeroMemory( WriteBuf, sizeof( WriteBuf ) );
                        DevWriting = FALSE;
                        break;
                    case WAIT_TIMEOUT:
                        // 超时后退出（有时候可能会异常操作卡住）
                        DevError = Error::ErrWriteTmOut;
                        throw "UsbHid写操作超时（写入操作正忙......）";
                }
            }
            DevWriting = TRUE;
            if ( len == 0 || len > USB_MAX ) len = USB_MAX;//  throw "写入数据长度越界";
            // 锁定发送缓冲区后才开始写数据到发送缓冲区
            memcpy( (char *) WriteBuf + 1, buf, len );
            // 调用WriteFile函数Write数据如果函数返回失败，则可能是真的失败，也可能是IO挂起了
            if ( WriteFile( WriteHandle, WriteBuf, USB_TX_MAX, NULL, &WriteOverlap ) ) return TRUE;
            // 获取最后错误代码
            DevError = GetLastError( );
            switch ( GetLastError( ) )
            {
                // 确认IO是否挂起，否则，抛出错误
                case ERROR_IO_PENDING:
                case ERROR_SUCCESS:
                    return TRUE;
                    // 确认UsbHid是否还打开
                case ERROR_DEVICE_NOT_CONNECTED:
                    CloseDev( );
                    throw "UsbHid已断开!";
                    // 如果最后错误为1，说明该UsbHid不支持该函数
                case ERROR_INVALID_FUNCTION:
                    throw "该UsbHid不支持WriteFile函数!";
                case ERROR_FILE_NOT_FOUND:
                    throw "系统找不到指定的UsbHid";
                default:
                    throw "发生未知错误";
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
