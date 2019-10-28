// CLoginDlg.cpp: 实现文件
//

#include "pch.h"
#include "nwtClient.h"
#include "CLoginDlg.h"
#include "afxdialogex.h"
#include "NwtHeader.h"
#include "Commands.h"


// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_LOGIN_DIALOG, pParent)
    , m_strAccount(_T(""))
    , m_strPassword(_T(""))
    , m_strNickname(_T(""))
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_ACCOUNT, m_strAccount);
    DDX_Text(pDX, IDC_EDIT_PWD, m_strPassword);
    DDX_Control(pDX, IDC_STATIC_NOTE, m_staticNote);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLoginDlg::OnBnClickedButtonLogin)
END_MESSAGE_MAP()


// CLoginDlg 消息处理程序


void CLoginDlg::OnBnClickedButtonLogin()
{
    UpdateData(TRUE);
    LoginReq loginReq;
    loginReq.m_head = NwtHeader(CMD_LOGIN_REQ, atoi(m_strAccount), 0, sizeof(LoginReq) - sizeof(NwtHeader));
    loginReq.m_account = atoi(m_strAccount);
    memset(&loginReq.m_password, 0, sizeof(loginReq.m_password));
    memcpy(loginReq.m_password, m_strPassword.GetString(), m_strPassword.GetLength());
    if (sizeof(LoginReq) != theApp.Send(&loginReq, sizeof(LoginReq))) {
        CString strText = "";
        int errNo = WSAGetLastError();
        strText.Format("[ERROR] 消息发送失败： errNo = %d", errNo);
        MessageBox(strText, "提示信息");
    }
}
