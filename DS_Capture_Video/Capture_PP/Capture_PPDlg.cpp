
// Capture_PPDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Capture_PP.h"
#include "Capture_PPDlg.h"
#include "afxdialogex.h"
#include "Utill.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCapture_PPDlg 对话框



CCapture_PPDlg::CCapture_PPDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CAPTURE_PP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCapture_PPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComboxVideo);
	DDX_Control(pDX, IDC_COMBO2, m_ComboxAudio);
}

BEGIN_MESSAGE_MAP(CCapture_PPDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCapture_PPDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCapture_PPDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CCapture_PPDlg 消息处理程序

BOOL CCapture_PPDlg::OnInitDialog()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_pVideoListBuf = new char *[DEF_BUF_MAX];
	for (int i = 0; i < DEF_BUF_MAX; i++)
	{
		m_pVideoListBuf[i] = new char[DEF_BUF_MAX];
		memset(m_pVideoListBuf[i], 0, DEF_BUF_MAX);
	}
	
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
void CCapture_PPDlg::DestoryData()
{
	for (int i = 0; i < DEF_BUF_MAX; i++)
	{
		DEF_SAFE_DELETE_BUF(m_pVideoListBuf[i]);
	}
	DEF_SAFE_DELETE_BUF(m_pVideoListBuf);
	TRACE("CCapture_PPDlg::DestoryData().");
}
void CCapture_PPDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCapture_PPDlg::OnPaint()
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
HCURSOR CCapture_PPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

WCHAR * CCapture_PPDlg::CHARCONVWCHAR(char *in_chbuf)
{
	int iLen = MultiByteToWideChar(0, 0, in_chbuf, -1, NULL, 0);

	TCHAR *tchBuf = new TCHAR[iLen +1];

	MultiByteToWideChar(0, 0, in_chbuf, -1, tchBuf, iLen + 1);

	return tchBuf;
}
//获取设备列表
void CCapture_PPDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bre = m_objCaptureManager.InitDevice(GetDlgItem(IDC_STATIC)->m_hWnd);
	if (!bre)
	{
		TRACE("m_objCaptureManager.InitDevice Failed.");
		return;
	}

	bre = m_objCaptureManager.GetDeviceList(m_pVideoListBuf, NULL);
	if (!bre)
	{
		TRACE("m_objCaptureManager.GetDeviceList Failed.");
		return;
	}
	for (int i = 0; i < DEF_BUF_MAX; i++)
	{
		if (*m_pVideoListBuf[i] == '\0')
			break;

		TCHAR *pB = CHARCONVWCHAR(m_pVideoListBuf[i]);

		m_ComboxVideo.InsertString(i,pB);

		DEF_SAFE_DELETE_BUF(pB);
	}
	m_ComboxVideo.SetCurSel(0);
}

//打开预览
void CCapture_PPDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = m_ComboxVideo.GetCurSel();

	BOOL bre = m_objCaptureManager.OpenDevice(m_pVideoListBuf[i], nullptr);
	if (!bre)
	{
		TRACE("m_objCaptureManager.OpenDevice Failed.");
	}
	
	bre = m_objCaptureManager.StartCapture();
	if (!bre)
	{
		TRACE("m_objCaptureManager.StartCapture Failed.");
	}
}


void CCapture_PPDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_objCaptureManager.CloseDevice();

	DestoryData();

	CDialogEx::OnClose();
}
