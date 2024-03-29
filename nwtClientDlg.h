﻿
// nwtClientDlg.h: 头文件
//

#pragma once

#include "Contact.h"
#include <vector>

// CnwtClientDlg 对话框
class CnwtClientDlg : public CDialogEx
{
// 构造
public:
    CnwtClientDlg(CWnd* pParent = nullptr);    // 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_NWTCLIENT_DIALOG };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


// 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    CListBox m_listContacts;
    std::vector<Contact> m_contacts;
    Contact m_own = Contact(0, "");
    CEdit m_editMsgList;
    CEdit m_editMsgSend;

private:
    int LoadContacts(const char* filename);

public:
    afx_msg void OnBnClickedSend();
    void AppendString(CString strText);
    afx_msg void OnLbnSelchangeListContacts();
};
