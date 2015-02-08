#pragma once
#include "IConfigManager.h"
class manipulateUserInfo
{
public:
	manipulateUserInfo(void);
	~manipulateUserInfo(void);
public:
	void inputUserInfo();
	void outputUserInfo();
private:
	bool getOutputUserInfoFilePath(QString &sFilePath);
	bool getInputUserInfoFilePath(QString &sFilePath);
private:
	IConfigManager *m_pIConfigManager;
};

