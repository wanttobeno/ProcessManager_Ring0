#ifndef ProcessManager_H__
#define ProcessManager_H__

#include<ntifs.h>

typedef enum WIN_VERSION {
	WINDOWS_UNKNOW,
	WINDOWS_XP,
	WINDOWS_7,
	WINDOWS_8,
	WINDOWS_8_1
} WIN_VERSION;

typedef  struct _PROCESS_INFOR_ 
{
	ULONG  ulProcessID;
	WCHAR  wzImageName[64];
	WCHAR  wzImagePath[512];
}PROCESS_INFOR,*PPROCESS_INFOR;

#define DEVICE_NAME  L"\\Device\\ProcessManagerDevice"
#define LINK_NAME    L"\\??\\ProcessManagerLink"

#define CTL_PROCESS_COUNT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_PROCESS_INFOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)
VOID UnloadDriver(PDRIVER_OBJECT DriverObject);
NTSTATUS DefaultDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp);
NTSTATUS ControlDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp);

typedef 
NTSTATUS (*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);
PVOID GetFunctionAddressByName(WCHAR *wzFunction);
WIN_VERSION GetWindowsVersion();
VOID SetGlobalOffset();
ULONG GetProcessCount();
VOID SetGolbalMember();
VOID GetProcessInfor(PVOID OutputBuffer);
BOOLEAN GetProcessIDByEProcess(PEPROCESS EProcess,ULONG* ulProcessID);
BOOLEAN GetProcessPathByEProcess(PEPROCESS EProcess,WCHAR* wzProcessPath);

extern PPEB PsGetProcessPeb(PEPROCESS Process);

#endif  // ProcessManager_H__






