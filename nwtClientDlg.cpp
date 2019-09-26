
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
END_MESSAGE_MAP()

class RecvProcessParam
{
public:
    RecvProcessParam(unsigned int clientSock, CnwtClientDlg* clientDlg)
        : m_clientSock(clientSock), m_clientDlg(clientDlg)
    {
    }

    unsigned int m_clientSock = INVALID_SOCKET;
    CnwtClientDlg* m_clientDlg = nullptr;
};

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
    CString strText = "", strCaptain = "提示信息";
    int retCode = 0;
    retCode = ConnectServer();
    if (0 == retCode) {
        RecvProcessParam* rpp = new RecvProcessParam(m_sock, this); //will delete in RecvProcess()
        AfxBeginThread(RecvProcess, rpp);
    }

    Login();
    LoadContacts("Contacts.txt");

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

int CnwtClientDlg::Login() {
    char buf[1024] = { 0 };
    NwtHeader nwtHead(CMD_LOGIN, m_own.m_account, 0, 0);
    memcpy(buf, &nwtHead, sizeof(NwtHeader));
    int retCode = send(m_sock, buf, sizeof(NwtHeader), 0);
    if (0 > retCode) {
        CString strText = "";
        int errNo = WSAGetLastError();
        strText.Format("[ERROR] 消息发送失败： errNo = %d", errNo);
        MessageBox(strText, "提示信息");
        return -1;
    }

    return 0;
}

int CnwtClientDlg::ConnectServer() {
    CString strText = "", strCaptain = "提示信息";
    WSAData wsaData;
    int retCode = 0;
    retCode = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if (0 != retCode) {
        strText.Format("[ERROR] WSAStarup()调用失败： retCode = %d", retCode);
        MessageBox(strText, strCaptain);
        return -1;
    }
    m_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == m_sock) {
        strText.Format("[ERROR] socket()调用失败： sock = %d", m_sock);
        MessageBox(strText, strCaptain);
        return -1;
    }

    int on = 1;
    retCode = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)& on, sizeof(on));
    if (0 > retCode)
    {
        strText.Format("[ERROR] setsockopt()调用失败： retCode = %d", retCode);
        MessageBox(strText, strCaptain);
        return -1;
    }
    sockaddr_in svrAddr;
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(m_svrPort);
    svrAddr.sin_addr.s_addr = inet_addr(m_svrIP);
    retCode = connect(m_sock, (struct sockaddr*) & svrAddr, sizeof(svrAddr));
    if (0 > retCode) {
        strText.Format("[ERROR] connect()调用失败： retCode = %d", retCode);
        MessageBox(strText, strCaptain);
        return -1;
    }

    return 0;
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
    auto iter = m_contacts.cbegin();
    for (; iter != m_contacts.cend(); ++iter) {
        if (iter->m_nickname == std::string(strSelContact.GetString())) {
            break;
        }
    }
    if (iter == m_contacts.cend()) {
        CString strText = "";
        strText.Format("[ERROR] 联系人未注册： nickname = %s", strSelContact);
        MessageBox(strText, "提示信息");
        return;
    }

    CString strMsgSend = "";
    m_editMsgSend.GetWindowText(strMsgSend);
    NwtHeader nwtHead(CMD_INSTANT_MSG, m_own.m_account, iter->m_account, strMsgSend.GetLength());
    char buf[1024] = { 0 };
    memcpy(buf, &nwtHead, sizeof(NwtHeader));
    memcpy(buf + sizeof(NwtHeader), strMsgSend.GetString(), nwtHead.m_contentLength);
    int retCode = send(m_sock, buf, sizeof(NwtHeader) + nwtHead.m_contentLength, 0); //TODO: refactor to single function to do loop send
    if (0 > retCode) {
        CString strText = "";
        int errNo = WSAGetLastError();
        strText.Format("[ERROR] 消息发送失败： errNo = %d", errNo);
        MessageBox(strText, "提示信息");
        return;
    }

    AppendString("[SEND] " + strMsgSend);
    m_editMsgSend.SetWindowText("");
}

UINT CnwtClientDlg::RecvProcess(LPVOID lParam) {
    RecvProcessParam* rpp = (RecvProcessParam*)lParam;
    unsigned int clientSock = rpp->m_clientSock;
    CnwtClientDlg* pClientDlg = rpp->m_clientDlg;
    if (INVALID_SOCKET == clientSock || nullptr == pClientDlg)
    {
        AfxMessageBox("invalid client socket or null dlg pointer!");
        return -1;
    }

    CString strNew = "", strOld = "", strRecv = "";
    char buf[1024] = { 0 };
    int rval = 0;
    do
    {
        memset(buf, 0, sizeof(buf));
        rval = recv(clientSock, buf, 1024, 0);//TODO: refactor to single function for loop-recv
        if (0 >= rval) {
            int errNo = WSAGetLastError();
            if (0 > rval) {
                strRecv.Format("[ERROR] recv()失败： clientSock = %d, errNo = %d", clientSock, errNo);
            }
            else {
                strRecv.Format("[DEBUG] 客户端关闭链接： clientSock = %d, errNo = %d", clientSock, errNo);
            }
            pClientDlg->AppendString(strRecv);
            break;
        }
        NwtHeader* nwtHead = (NwtHeader*)buf;
        if (CMD_INSTANT_MSG == nwtHead->m_cmd) {
            char* content = new char[nwtHead->m_contentLength + 1];
            memset(content, 0, nwtHead->m_contentLength + 1);
            memcpy(content, buf + sizeof(NwtHeader), nwtHead->m_contentLength);
            strRecv.Format("[RECV] rval=%d, srcAccount=%d, targetAccount=%d, contentLength=%d, content=%s",
                rval, nwtHead->m_srcAccount, nwtHead->m_tarAccount, nwtHead->m_contentLength, content);
            pClientDlg->AppendString(strRecv);

            delete[] content;
        }

    } while (1);

    delete rpp; //allocate by OnInitDialog()

    return 0;
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
        std::string::size_type commaPos = 0;
        std::string line = "", account = "", nickname = "";
        while (getline(contacts, line)) {
            commaPos = line.find(',');
            account = line.substr(0, commaPos);
            nickname = line.substr(commaPos + 1, line.size() - commaPos - 1);
            m_contacts.emplace_back(std::stoull(account), nickname.c_str());
            m_listContacts.AddString(nickname.c_str());
        }
        contacts.close();
        m_listContacts.SetCurSel(0);
    }

    return 0;
}
