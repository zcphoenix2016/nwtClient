
// nwtClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "nwtClient.h"
#include "nwtClientDlg.h"
#include "afxdialogex.h"
#include "Commands.h"
#include "NwtHeader.h"

#include <fstream>

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


// CnwtClientDlg 对话框



CnwtClientDlg::CnwtClientDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_NWTCLIENT_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CnwtClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CONTACTS, m_listContacts);
    DDX_Control(pDX, IDC_EDIT_MSGLIST, m_editMsgList);
    DDX_Control(pDX, IDC_EDIT_MSGSEND, m_editMsgSend);
}

BEGIN_MESSAGE_MAP(CnwtClientDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(ID_SEND, &CnwtClientDlg::OnBnClickedSend)
    ON_LBN_SELCHANGE(IDC_LIST_CONTACTS, &CnwtClientDlg::OnLbnSelchangeListContacts)
END_MESSAGE_MAP()

// CnwtClientDlg 消息处理程序

BOOL CnwtClientDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
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
    SetIcon(m_hIcon, TRUE);            // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标

    // TODO: 在此添加额外的初始化代码
    CString strCaption = "";
    strCaption.Format("%s(%d)", m_own.m_nickname.c_str(), m_own.m_account);
    SetWindowText(strCaption);

    LoadContacts("Contacts.txt"); //TODO: remove hardcode filename

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CnwtClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CnwtClientDlg::OnPaint()
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
HCURSOR CnwtClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CnwtClientDlg::AppendString(CString strText) {
    CString strOld = "", strNew = "";
    m_editMsgList.GetWindowText(strOld);
    strNew = strOld + (strOld.IsEmpty() ? "" : "\r\n") + strText;
    m_editMsgList.SetWindowText(strNew);
}

void CnwtClientDlg::OnBnClickedSend()
{
    CString strSelContact = "";
    m_listContacts.GetText(m_listContacts.GetCurSel(), strSelContact);
    auto iterContact = m_contacts.begin();
    for (; iterContact != m_contacts.end(); ++iterContact) {
        if (iterContact->m_nickname == std::string(strSelContact.GetString())) {
            break;
        }
    }
    if (iterContact == m_contacts.end()) {
        CString strText = "";
        strText.Format("[ERROR] 联系人未注册： nickname = %s", strSelContact.GetString());
        MessageBox(strText, "提示信息");
        return;
    }

    CString strMsgSend = "";
    m_editMsgSend.GetWindowText(strMsgSend);
    char* imMsg = new char[sizeof(NwtHeader) + strMsgSend.GetLength()];
    InstantMsg* im = (InstantMsg*)imMsg;
    im->m_head.m_cmd = CMD_INSTANT_MSG;
    im->m_head.m_srcAccount = m_own.m_account;
    im->m_head.m_tarAccount = iterContact->m_account;
    im->m_head.m_contentLength = strMsgSend.GetLength();
    memcpy(im->m_content, strMsgSend.GetString(), strMsgSend.GetLength());
    int want = sizeof(NwtHeader) + im->m_head.m_contentLength;
    if (want != theApp.Send((void*)im, want)) {
        CString strText = "";
        int errNo = WSAGetLastError();
        strText.Format("[ERROR] 消息发送失败： errNo = %d", errNo);
        MessageBox(strText, "提示信息");
    }
    else {
        strMsgSend.Format("    [SEND] %s", strMsgSend.GetString());
        iterContact->m_msgs.push_back(strMsgSend.GetString());
        AppendString(strMsgSend);
        m_editMsgSend.SetWindowText("");
    }
    
    delete[] imMsg;
}

int CnwtClientDlg::LoadContacts(const char* filename) {
    std::ifstream contacts(filename);
    if (!contacts) {
        CString strText = "";
        strText.Format("[ERROR] 打开联系人文件失败： filename = %s", filename);
        MessageBox(strText, "错误提示");
        return -1;
    }
    else {
        string::size_type commaPos = 0;
        string line = "", account = "", nickname = "";
        while (getline(contacts, line)) {
            commaPos = line.find(',');
            account = line.substr(0, commaPos);
            nickname = line.substr(commaPos + 1, line.size() - commaPos - 1);
            m_contacts.emplace_back(atoi(account.c_str()), nickname.c_str());
            m_listContacts.AddString(nickname.c_str());
        }
        contacts.close();
        m_listContacts.SetCurSel(0);
    }

    return 0;
}


void CnwtClientDlg::OnLbnSelchangeListContacts()
{
    int index = m_listContacts.GetCurSel();
    CString strShowName;
    m_listContacts.GetText(index, strShowName);
    string showName = strShowName.GetString();
    auto iterContact = m_contacts.begin();
    for (; iterContact != m_contacts.end(); iterContact++) {
        if (iterContact->m_showName == showName) {
            break;
        }
    }

    if (iterContact == m_contacts.end()) {
        //TODO: report error.
        return;
    }

    m_editMsgList.SetSel(0, -1);
    m_editMsgList.Clear();
    CString strMsg = "";
    auto iterMsg = iterContact->m_msgs.begin();
    for (; iterMsg != iterContact->m_msgs.end(); iterMsg++) {
        strMsg.Format("%s", iterMsg->c_str());
        AppendString(strMsg);
    }

    if (0 != iterContact->m_numOfUnread) {
        iterContact->m_numOfUnread = 0;
        iterContact->m_showName = iterContact->m_nickname;
    }
    m_listContacts.DeleteString(index);
    m_listContacts.InsertString(index, iterContact->m_nickname.c_str());
    m_listContacts.SetCurSel(index);
}
