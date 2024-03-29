﻿
// nwtClient.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "nwtClient.h"
#include "nwtClientDlg.h"
#include "CLoginDlg.h"
#include "Commands.h"
#include "NwtBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CnwtClientApp

BEGIN_MESSAGE_MAP(CnwtClientApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CnwtClientApp 构造

CnwtClientApp::CnwtClientApp()
{
    // 支持重新启动管理器
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

    // TODO: 在此处添加构造代码，
    // 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CnwtClientApp 对象

CnwtClientApp theApp;

class RecvProcessParam
{
public:
    RecvProcessParam(CLoginDlg* loginDlg, CnwtClientDlg* clientDlg)
        : m_loginDlg(loginDlg), m_clientDlg(clientDlg)
    {
    }

    CLoginDlg* m_loginDlg = nullptr;
    CnwtClientDlg* m_clientDlg = nullptr;
};

// CnwtClientApp 初始化

BOOL CnwtClientApp::InitInstance()
{
    // 如果一个运行在 Windows XP 上的应用程序清单指定要
    // 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
    //则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // 将它设置为包括所有要在应用程序中使用的
    // 公共控件类。
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();


    AfxEnableControlContainer();

    // 创建 shell 管理器，以防对话框包含
    // 任何 shell 树视图控件或 shell 列表视图控件。
    CShellManager *pShellManager = new CShellManager;

    // 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // 标准初始化
    // 如果未使用这些功能并希望减小
    // 最终可执行文件的大小，则应移除下列
    // 不需要的特定初始化例程
    // 更改用于存储设置的注册表项
    // TODO: 应适当修改该字符串，
    // 例如修改为公司或组织名
    SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

    //登录服务器
    int retCode = ConnectServer();
    if (0 == retCode) {
        CLoginDlg loginDlg;
        CnwtClientDlg clientDlg;
        m_running = TRUE;
        RecvProcessParam* rpp = new RecvProcessParam(&loginDlg, &clientDlg); //will delete in RecvProcess()
        AfxBeginThread(RecvProcess, rpp);
        
        int nResponse = loginDlg.DoModal();
        if (nResponse == IDOK) {
            
            clientDlg.m_own.m_account = atoi(loginDlg.m_strAccount);
            clientDlg.m_own.m_nickname = loginDlg.m_strNickname.GetString();

            m_pMainWnd = &clientDlg;
            nResponse = clientDlg.DoModal();
            m_running = FALSE;
            if (nResponse == IDOK)
            {
                // TODO: 在此放置处理何时用
                //  “确定”来关闭对话框的代码
            }
            else if (nResponse == IDCANCEL)
            {
                // TODO: 在此放置处理何时用
                //  “取消”来关闭对话框的代码
            }
            else if (nResponse == -1)
            {
                TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
                TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
            }
        }
    }

    // 删除上面创建的 shell 管理器。
    if (pShellManager != nullptr)
    {
        delete pShellManager;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    //  而不是启动应用程序的消息泵。
    return FALSE;
}

int CnwtClientApp::ConnectServer() {
    CString strText = "";
    WSAData wsaData;
    int retCode = 0;
    retCode = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if (0 != retCode) {
        strText.Format("[ERROR] WSAStarup()调用失败： retCode = %d", retCode);
        AfxMessageBox(strText);
        return -1;
    }
    m_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == m_sock) {
        strText.Format("[ERROR] socket()调用失败： sock = %d", m_sock);
        AfxMessageBox(strText);
        return -1;
    }

    int on = 1;
    retCode = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)& on, sizeof(on));
    if (0 > retCode)
    {
        strText.Format("[ERROR] setsockopt()调用失败： retCode = %d", retCode);
        AfxMessageBox(strText);
        return -1;
    }
    sockaddr_in svrAddr;
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(m_svrPort);
    svrAddr.sin_addr.s_addr = inet_addr(m_svrIP);
    retCode = connect(m_sock, (struct sockaddr*) & svrAddr, sizeof(svrAddr));
    if (0 > retCode) {
        strText.Format("[ERROR] connect()调用失败： retCode = %d", retCode);
        AfxMessageBox(strText);
        return -1;
    }

    return 0;
}

unsigned int CnwtClientApp::RecvProcess(LPVOID lParam) {
    RecvProcessParam* rpp = (RecvProcessParam*)lParam;
    CLoginDlg* pLoginDlg = rpp->m_loginDlg;
    CnwtClientDlg* pClientDlg = rpp->m_clientDlg;
    if (nullptr == pLoginDlg || nullptr == pClientDlg)
    {
        AfxMessageBox("null dlg pointer!");
        return -1;
    }

    CString strNew = "", strOld = "", strRecv = "";
    void* recv = NULL;
    NwtHeader* nwtHead = NULL;
    do
    {
        recv = theApp.Recv();
        nwtHead = (NwtHeader*)recv;
        if (NULL == nwtHead) {
            int errNo = WSAGetLastError();
            strRecv.Format("[DEBUG] 服务端关闭链接： clientSock = %d, errNo = %d", theApp.m_sock, errNo);
            pClientDlg->AppendString(strRecv);
            break;
        }

        if (CMD_LOGIN_RSP == nwtHead->m_cmd) {
            LoginRsp* loginRsp = (LoginRsp*)recv;
            if (LOGIN_FAIL == loginRsp->m_rspCode) {
                pLoginDlg->SetDlgItemText(IDC_STATIC_NOTE, loginRsp->m_rspMsg);
            }
            else {
                pLoginDlg->m_strNickname.Format("%s", loginRsp->m_nickname);
                pLoginDlg->EndDialog(IDOK);
            }
        }

        //TODO: refactor to single function for diff msg type
        if (CMD_INSTANT_MSG == nwtHead->m_cmd) {
            InstantMsg* im = (InstantMsg*)nwtHead;
            char* content = new char[im->m_head.m_contentLength + 1];
            memset(content, 0, im->m_head.m_contentLength + 1);
            memcpy(content, im->m_content, im->m_head.m_contentLength);
            
            auto iterContact = pClientDlg->m_contacts.begin();
            for (; iterContact != pClientDlg->m_contacts.end(); iterContact++) {
                if (iterContact->m_account == im->m_head.m_srcAccount) {
                    break;
                }
            }
            if (iterContact == pClientDlg->m_contacts.end()) {
                strRecv.Format("[ERROR] receive msg from non-contact: account = %d", im->m_head.m_srcAccount);
                AfxMessageBox(strRecv); //TODO: build log system
            }
            else {
                strRecv.Format("[RECV] %s", content);
                iterContact->m_msgs.push_back(strRecv.GetString());
                int index = pClientDlg->m_listContacts.FindStringExact(-1, iterContact->m_showName.c_str());
                if (LB_ERR == index) {
                    strRecv.Format("[ERROR] Contact not in the listbox: nickname = %s", iterContact->m_showName.c_str());
                    AfxMessageBox(strRecv); //TODO: build log system
                }
                else {
                    if (index == pClientDlg->m_listContacts.GetCurSel()) {
                        strRecv.Format("[RECV] %s", content);
                        pClientDlg->AppendString(strRecv);
                    }
                    else {
                        iterContact->m_numOfUnread++;
                        strRecv.Format("(%d)%s", iterContact->m_numOfUnread, iterContact->m_nickname.c_str());
                        iterContact->m_showName = strRecv.GetString();
                        pClientDlg->m_listContacts.DeleteString(index);
                        pClientDlg->m_listContacts.InsertString(index, strRecv.GetString());
                    }
                }
            }

            delete[] content;
        }

        delete[] (char*)recv;
    } while (theApp.m_running);

    delete rpp; //allocate by OnInitDialog()

    return 0;
}

int CnwtClientApp::Send(void* buf, size_t nbytes) {
    return nwtSend(m_sock, buf, nbytes);
}

void* CnwtClientApp::Recv() {
    return nwtRecv(m_sock);
}



