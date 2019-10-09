#pragma once

#include <string>

class Contact {
public:
    Contact(unsigned int account, const char* nickname)
        : m_account(account), m_nickname(nickname)
    {
    }

public:
    unsigned int m_account = 0; //TODO: refactor to char array ???
    std::string m_nickname = "";
};