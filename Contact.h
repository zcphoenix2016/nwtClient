#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

class Contact {
public:
    Contact(unsigned int account, const char* nickname)
        : m_account(account), m_nickname(nickname)
    {
        m_showName = m_nickname;
    }

public:
    unsigned int m_account = 0; //TODO: refactor to char array ???
    string m_nickname = "";
    string m_showName = "";
    unsigned int m_numOfUnread = 0;
    vector<string> m_msgs;
};