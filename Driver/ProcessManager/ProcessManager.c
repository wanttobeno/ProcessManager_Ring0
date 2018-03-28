#include "ProcessManager.h"
#ifdef _WIN64
#ifndef WIN64
#define  WIN64 _WIN64
#endif // !WIN64
#endif // _WIN64

////////////////////////////////////////////////////////////

ULONG_PTR    ObjectHeaderSize = 0;
ULONG_PTR    ObjectTypeOffsetOf_Object_Header = 0;
ULONG_PTR    ObjectTableOffsetOf_EPROCESS = 0;

WIN_VERSION     WinVersion = WINDOWS_UNKNOW;
ULONGLONG		SYSTEM_ADDRESS_START = 0;
PEPROCESS       CurrentEProcess = NULL;

ULONG_PTR		Eprocess_ActiveProcessLinks_Offset = 0;
ULONG_PTR		Eprocess_ImageFileName_Offset = 0; 
ULONG_PTR       Peb_ProcessParameters_Offset = 0;
ULONG_PTR       ProcessParameters_ImagePathName_Offset = 0;

NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject,PUNICODE_STRING  RegisterPath)
{
	PDEVICE_OBJECT	DeviceObject;
	NTSTATUS		Status;
	ULONG			i;

	UNICODE_STRING	uniDeviceName;
	UNICODE_STRING	uniLinkName;
	RtlInitUnicodeString(&uniDeviceName,DEVICE_NAME);
	RtlInitUnicodeString(&uniLinkName,LINK_NAME);

	//创建设备对象;
	Status = IoCreateDevice(DriverObject,0,&uniDeviceName,FILE_DEVICE_UNKNOWN,0,FALSE,&DeviceObject);
	if (!NT_SUCCESS(Status))
		return Status;

	//创建符号链接;
	Status = IoCreateSymbolicLink(&uniLinkName,&uniDeviceName);
	for (i = 0; i<IRP_MJ_MAXIMUM_FUNCTION; i ++)
	{
		DriverObject->MajorFunction[i] = DefaultDispatch;
	}
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlDispatch;
	DriverObject->DriverUnload = UnloadDriver;
	
#ifdef WIN64
/*	__asm
	{
		xchg rax,rbx
	}
*/	DbgPrint("X64: ProcessManager IS RUNNING!!!\n");
#else
/*	__asm
	{
		xor eax,eax
	}
*/	DbgPrint("X86: ProcessManager IS RUNNING!!!\n");
#endif
	
	WinVersion = GetWindowsVersion();
	SetGolbalMember();
	CurrentEProcess = PsGetCurrentProcess();
	KdPrint(("CurrentEProcess = 0x%x ,0x%x", (ULONG_PTR)CurrentEProcess,&CurrentEProcess));
	
	return STATUS_SUCCESS;
}

NTSTATUS ControlDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	NTSTATUS  Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION   IrpSp;
	PVOID     InputBuffer  = NULL;
	PVOID     OutputBuffer = NULL;
	ULONG_PTR InputSize  = 0;
	ULONG_PTR OutputSize = 0;
	ULONG_PTR IoControlCode = 0;
	ULONG     ulCount = 0;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
	OutputBuffer = Irp->UserBuffer;
	InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
	switch(IoControlCode)
	{
	case CTL_PROCESS_COUNT:
		{
			ulCount = GetProcessCount();

			memcpy(OutputBuffer,&ulCount,OutputSize);
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
		}
		break;
	case CTL_PROCESS_INFOR:
		{
			GetProcessInfor(OutputBuffer);
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = STATUS_SUCCESS;
		}
		break;
	default:
		{
			Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			Irp->IoStatus.Information = 0;
		}
		break;
	}
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return Status;
}

NTSTATUS DefaultDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING  uniLinkName;
	PDEVICE_OBJECT	NextObject = NULL;
	PDEVICE_OBJECT  CurrentObject = NULL;
	RtlInitUnicodeString(&uniLinkName,LINK_NAME);

	IoDeleteSymbolicLink(&uniLinkName);
	CurrentObject = DriverObject->DeviceObject;
	while (CurrentObject != NULL) 
	{
		NextObject = CurrentObject->NextDevice;
		IoDeleteDevice(CurrentObject);
		CurrentObject = NextObject;
	}

#ifdef WIN64
	DbgPrint("X64: ProcessManager IS STOPPED!!!\n");
#else
	DbgPrint("X86: ProcessManager IS STOPPED!!!\n");
#endif
	return;
}

WIN_VERSION GetWindowsVersion()
{
	RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
	pfnRtlGetVersion RtlGetVersion = NULL;
	WIN_VERSION WinVersion;
	WCHAR wzRtlGetVersion[] = L"RtlGetVersion";

	RtlGetVersion = GetFunctionAddressByName(wzRtlGetVersion);    
	if (RtlGetVersion)
	{
		RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
	} 
	else 
	{
		PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);
	}

	DbgPrint("Build Number: %d\r\n", osverInfo.dwBuildNumber);

	if (osverInfo.dwMajorVersion == 5 && osverInfo.dwMinorVersion == 1) 
	{
		DbgPrint("WINDOWS_XP\r\n");
		WinVersion = WINDOWS_XP;
	}
	else if (osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1)
	{
		DbgPrint("WINDOWS 7\r\n");
		WinVersion = WINDOWS_7;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 2 &&
		osverInfo.dwBuildNumber == 9200)
	{
		DbgPrint("WINDOWS 8\r\n");
		WinVersion = WINDOWS_8;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 3 && 
		osverInfo.dwBuildNumber == 9600)
	{
		DbgPrint("WINDOWS 8.1\r\n");
		WinVersion = WINDOWS_8_1;
	}
	else
	{
		DbgPrint("WINDOWS_UNKNOW\r\n");
		WinVersion = WINDOWS_UNKNOW;
	}

	return WinVersion;
}

BOOLEAN IsWin64()
{
	int nNum = 0;
	int nSize = sizeof(&nNum);
	if (nSize == 8)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID SetGolbalMember()
{
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			SYSTEM_ADDRESS_START = 0x80000000;
			Eprocess_ActiveProcessLinks_Offset = 0x88;
			Eprocess_ImageFileName_Offset = 0x174; 

			Peb_ProcessParameters_Offset = 0x010;
			ProcessParameters_ImagePathName_Offset = 0x038;
		}
		break;
	case WINDOWS_7:
		{
			if(IsWin64())
			{
				SYSTEM_ADDRESS_START = 0x80000000000;
				Eprocess_ActiveProcessLinks_Offset = 0x188;
				Eprocess_ImageFileName_Offset = 0x2e0;

				Peb_ProcessParameters_Offset = 0x020;
				ProcessParameters_ImagePathName_Offset = 0x60;
			}
			else
			{
				SYSTEM_ADDRESS_START = 0x80000000;
				Eprocess_ActiveProcessLinks_Offset = 0x0b8;
				Eprocess_ImageFileName_Offset = 0x16c;

				Peb_ProcessParameters_Offset = 0x010;
				ProcessParameters_ImagePathName_Offset = 0x038;
			}
		}
		break;
	}
}

VOID  SetGlobalOffset() 
{
	switch(WinVersion)
	{
	case 7601:    
		{
			ObjectTableOffsetOf_EPROCESS = 0x200;
		}
		break;
	case 2600:   
		{
			ObjectHeaderSize  = 0x18;
			ObjectTypeOffsetOf_Object_Header = 0x8;
			ObjectTableOffsetOf_EPROCESS = 0x0c4;
		}
		break;
	}
}

PVOID GetFunctionAddressByName(WCHAR *wzFunction)
{
	UNICODE_STRING uniFunction;  
	PVOID AddrBase = NULL;

	if (wzFunction && wcslen(wzFunction) > 0)
	{
		RtlInitUnicodeString(&uniFunction, wzFunction);     
		AddrBase = MmGetSystemRoutineAddress(&uniFunction); 
	}
	return AddrBase;
}

ULONG GetProcessCount()
{
	PEPROCESS EProcessCurrent = CurrentEProcess;
	PEPROCESS EProcessPre = NULL;
	PLIST_ENTRY Temp = NULL;
	ULONG   ulCount = 0;
	LIST_ENTRY*     ActiveProcessLinks = NULL;
	__try
	{
		do
		{
			ulCount++;
			ActiveProcessLinks = (LIST_ENTRY*)((ULONG_PTR)EProcessCurrent + Eprocess_ActiveProcessLinks_Offset);
			EProcessCurrent = (PEPROCESS)((ULONG_PTR)(ActiveProcessLinks->Flink) - Eprocess_ActiveProcessLinks_Offset);
		} while (EProcessCurrent != CurrentEProcess);
	}
	__except(1)
	{
		DbgPrint("ProcessNum = %d/n", ulCount);
	}
	DbgPrint("EnumProcessList exception !");
	return ulCount;
}

VOID GetProcessInfor(PVOID OutputBuffer)
{
	PEPROCESS EProcessCurrent = CurrentEProcess;
	PEPROCESS EProcessPre = NULL;
	PLIST_ENTRY Temp = NULL;
	ULONG   ulCount = 0;
	ULONG   ulProcessID = 0;
	PPROCESS_INFOR   ProcessInfor = (PPROCESS_INFOR)OutputBuffer;
	WCHAR wzProcessPath[512] = {0};
	LIST_ENTRY*     ActiveProcessLinks = NULL;
	__try
	{
		do
		{
			if (GetProcessIDByEProcess(EProcessCurrent, &ProcessInfor[ulCount].ulProcessID))
			{
				switch (ProcessInfor[ulCount].ulProcessID)
				{
				case 0:
				{
					wcscpy(ProcessInfor[ulCount].wzImageName, L"System Idle Process");
				}
				break;
				case 4:
				{
					wcscpy(ProcessInfor[ulCount].wzImageName, L"System");
				}
				break;
				default:
				{
					if (GetProcessPathByEProcess(EProcessCurrent, wzProcessPath) == TRUE)
					{
						DbgPrint("%S\r\n", wzProcessPath);
						wcscpy(ProcessInfor[ulCount].wzImagePath, wzProcessPath);
						memset(wzProcessPath, 0, sizeof(WCHAR) * 512);
					}
				}
				break;
				}
			}
			else
			{
				DbgPrint("无效的PID\n");
			}
			ActiveProcessLinks = (LIST_ENTRY*)((ULONG_PTR)EProcessCurrent + Eprocess_ActiveProcessLinks_Offset);
			EProcessCurrent = (PEPROCESS)((ULONG_PTR)(ActiveProcessLinks->Flink) - Eprocess_ActiveProcessLinks_Offset);
			ulCount++;
		} while (EProcessCurrent != CurrentEProcess);
	}
	__except (1)
	{
		DbgPrint("EnumProcessList exception !");
	}
}

BOOLEAN GetProcessIDByEProcess(PEPROCESS EProcess,ULONG* ulProcessID)
{
	if (EProcess==NULL||!MmIsAddressValid(EProcess))
	{
		return FALSE;
	}
	*ulProcessID = (ULONG)PsGetProcessId(EProcess);        
	if (*ulProcessID%0x4!=0)
	{
		DbgPrint("Invalid EPEOCESS\n");
		return FALSE;
	}
	return TRUE;
}

BOOLEAN GetProcessPathByEProcess(PEPROCESS EProcess,WCHAR* wzProcessPath)
{
	PPEB  Peb = NULL;
	KAPC_STATE ApcState;
	ULONG_PTR  ProcessParameters = 0;

	if (EProcess==NULL||!MmIsAddressValid(EProcess))
		return FALSE;
	Peb = PsGetProcessPeb(EProcess);
	if (Peb==NULL)
		return FALSE;
	//切换到用户层,才能读取到正确数据
	KeStackAttachProcess(EProcess, &ApcState);   
	ProcessParameters = *(ULONG_PTR*)((ULONG_PTR)Peb+Peb_ProcessParameters_Offset);
	memcpy(wzProcessPath,((PUNICODE_STRING)((ULONG_PTR)ProcessParameters+ProcessParameters_ImagePathName_Offset))->Buffer,
		((PUNICODE_STRING)((ULONG_PTR)ProcessParameters+ProcessParameters_ImagePathName_Offset))->Length);
	KeUnstackDetachProcess(&ApcState);
	return TRUE;
}

