
// ProcessManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManager.h"
#include "ProcessManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

static UINT indicators[] =
{
	IDR_STATUSBAR_STRING
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框


HANDLE g_hDevice = NULL;
HANDLE
OpenDevice(LPCTSTR lpDevicePath)
{
	HANDLE hDevice = CreateFile(lpDevicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
	}

	return hDevice;

}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CProcessManagerDlg 对话框


CProcessManagerDlg::CProcessManagerDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CProcessManagerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


	m_ulProcessCount = 0;
	m_ProcessInfor = NULL;
}

void CProcessManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}

BEGIN_MESSAGE_MAP(CProcessManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CProcessManagerDlg 消息处理程序

BOOL CProcessManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	
	

	CreatStatusBar();
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List.InsertColumn(0, L"ID", LVCFMT_LEFT, 40);
	m_List.InsertColumn(1, L"进程ID", LVCFMT_LEFT, 60);
	m_List.InsertColumn(2, L"名称", LVCFMT_LEFT, 120);
	m_List.InsertColumn(3, L"路径", LVCFMT_LEFT, 300);

	CRect rect;
	GetWindowRect(&rect);
	rect.bottom += 20;
	MoveWindow(rect);

	//获得进程个数
	if (GetProcessCount() == TRUE)
	{
		CString strStatusMsg;
		strStatusMsg.Format(L"有%d个进程", m_ulProcessCount);
		m_StatusBar.SetPaneText(0, strStatusMsg);   //在状态条上显示文字
	}

	//获得进程信息
	if (GetProcessInfor() == TRUE)
	{
		//向ControlList上设置数据
		SetDataToControlList();
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL CProcessManagerDlg::GetProcessCount()
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	g_hDevice = OpenDevice(L"\\\\.\\ProcessManagerLink");

	if (g_hDevice == (HANDLE)-1)
	{
		AfxMessageBox(_T("打开驱动失败，请安装并启动驱动！"));
		SendMessage(WM_CLOSE,0,0);
		return FALSE;
	}

	dwRet = DeviceIoControl(g_hDevice, CTL_PROCESS_COUNT,
		NULL,
		0,
		&m_ulProcessCount,
		sizeof(ULONG),
		&dwReturnSize,
		NULL);

	if (dwRet == 0)
	{
		CloseHandle(g_hDevice);
		return FALSE;
	}
	CloseHandle(g_hDevice);
	return  TRUE;
}

BOOL CProcessManagerDlg::GetProcessInfor()
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	if (m_ProcessInfor != NULL)
	{
		delete m_ProcessInfor;
		m_ProcessInfor = NULL;
	}

	m_ProcessInfor = new PROCESS_INFOR[m_ulProcessCount];
	if (m_ProcessInfor == NULL)
	{
		return FALSE;
	}

	memset(m_ProcessInfor, 0, sizeof(PROCESS_INFOR)*m_ulProcessCount);
	g_hDevice = OpenDevice(L"\\\\.\\ProcessManagerLink");

	if (g_hDevice == (HANDLE)-1)
	{
		delete m_ProcessInfor;
		m_ProcessInfor = NULL;
		return FALSE;
	}

	dwRet = DeviceIoControl(g_hDevice, CTL_PROCESS_INFOR,
		NULL,
		0,
		m_ProcessInfor,
		sizeof(PROCESS_INFOR)*m_ulProcessCount,
		&dwReturnSize,
		NULL);

	if (dwRet == 0)
	{
		delete m_ProcessInfor;
		m_ProcessInfor = NULL;
		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);
	return  TRUE;
}

VOID CProcessManagerDlg::SetDataToControlList()
{
	int i = 0;

	if (m_ProcessInfor == NULL)
	{
		return;
	}
	CString strId;
	for (i = 0; i < m_ulProcessCount; i++)
	{
		CString  strProcessID;

		strProcessID.Format(L"%d", m_ProcessInfor[i].ulProcessID);
		strId.Format(L"%d",i+1);
		
		int nNewItem = m_List.InsertItem(m_List.GetItemCount(),strId);
		
		m_List.SetItemText(nNewItem, 1, strProcessID);
		FixProcessImageName(m_ProcessInfor[i].wzImageName, m_ProcessInfor[i].wzImagePath);
		m_List.SetItemText(nNewItem, 2, m_ProcessInfor[i].wzImageName);
		m_List.SetItemText(nNewItem, 3, m_ProcessInfor[i].wzImagePath);
	}
}

VOID CProcessManagerDlg::FixProcessImageName(WCHAR* wzImageName, WCHAR* wzImagePath)
{
	WCHAR* wzTemp = NULL;
	if (wzImagePath != NULL)
	{
		wzTemp = wcsrchr(wzImagePath, '\\');
		if (wzTemp != NULL)
		{
			wzTemp++;
			wcscpy(wzImageName, wzTemp);
		}
	}
}

void CProcessManagerDlg::CreatStatusBar(void)
{
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(indicators,
		sizeof(indicators) / sizeof(UINT)))                    //创建状态条并设置字符资源的ID
	{
		return;
	}
	CRect rc;
	::GetWindowRect(m_StatusBar.m_hWnd, rc);
	m_StatusBar.MoveWindow(rc);                           //移动状态条到指定位置
}

void CProcessManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CProcessManagerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CProcessManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CProcessManagerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_StatusBar.m_hWnd != NULL){    //当对话框大小改变时 状态条大小也随之改变
		CRect rc;
		rc.top = cy - 20;
		rc.left = 0;
		rc.right = cx;
		rc.bottom = cy;
		m_StatusBar.MoveWindow(rc);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx - 10);
	}
}