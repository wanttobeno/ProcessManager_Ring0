
// ProcessManagerDlg.h : 头文件
//

#pragma once


// CProcessManagerDlg 对话框
#include <WinIoCtl.h>

#define CTL_PROCESS_COUNT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)


#define CTL_PROCESS_INFOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)


typedef  struct _PROCESS_INFOR_ 
{
	ULONG  ulProcessID;
	WCHAR  wzImageName[64];
	WCHAR  wzImagePath[512];
}PROCESS_INFOR,*PPROCESS_INFOR;
// CProcessManagerDlg 对话框
class CProcessManagerDlg : public CDialogEx
{
	// 构造
public:
	CProcessManagerDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CProcessManagerDlg()
	{
		if (m_ProcessInfor!=NULL)
		{
			delete m_ProcessInfor;
			m_ProcessInfor = NULL;
		}
	}
	// 对话框数据
	enum { IDD = IDD_MY_DIALOG };

	ULONG  m_ulProcessCount;
	PPROCESS_INFOR  m_ProcessInfor;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	void CProcessManagerDlg::CreatStatusBar(void);
	BOOL CProcessManagerDlg::GetProcessCount();
	BOOL CProcessManagerDlg::GetProcessInfor();
	VOID SetDataToControlList();
	VOID CProcessManagerDlg::FixProcessImageName(WCHAR* wzImageName,WCHAR* wzImagePath);
	// 实现
protected:
	HICON m_hIcon;
	CStatusBar  m_StatusBar;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_List;
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
