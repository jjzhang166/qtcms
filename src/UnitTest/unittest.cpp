#include "unittest.h"
#include <guid.h>
#include <IUserManager.h>

UnitTest::UnitTest()
{

}

UnitTest::~UnitTest()
{

}

void UnitTest::testCase1()
{
	IUserManager *Itest = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Itest);
	QVERIFY2(NULL != Itest, "Failure");
	int nRet = Itest->AddUser("caostorm","bubble",0,-1,-1);
	QVERIFY2(0 == nRet,"AddUser");
	int nCount = Itest->GetUserCount();
	QVERIFY2(1 == nCount,"GetUserCount");
	Itest->Release();
}