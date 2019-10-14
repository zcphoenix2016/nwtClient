
// nwtClient.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
    #error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"        // 主符号


// CnwtClientApp:
// 有关此类的实现，请参阅 nwtClient.cpp
//

class CnwtClientApp : public CWinApp
{
public:
    CnwtClientApp();

// 重写
public:
    virtual BOOL InitInstance();

private:
    int ConnectServer();
    static unsigned int RecvProcess(LPVOID lParam);

public:
    int Send(void* buf, size_t nbytes);
    int Recv(void* buf, size_t nbytes);

public:
    unsigned int m_sock = INVALID_SOCKET;
    BOOL m_running = FALSE;

private:
    unsigned int m_svrPort = 8888;
    const char* m_svrIP = "127.0.0.1";

// 实现

    DECLARE_MESSAGE_MAP()
};

extern CnwtClientApp theApp;
