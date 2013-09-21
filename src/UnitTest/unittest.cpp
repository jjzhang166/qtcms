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
// 	IUserManager *Itest = NULL;
// 	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Itest);
// 	QVERIFY2(NULL != Itest, "Failure");
// 	int nRet = Itest->AddUser("caostorm","bubble",0,-1,-1);
// 	QVERIFY2(0 == nRet,"AddUser");
// 	int nCount = Itest->GetUserCount();
// 	QVERIFY2(1 == nCount,"GetUserCount");
// 	Itest->Release();
}

void UnitTest::testCase1_z()
{
	IUserManager *ltest = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&ltest);
	QVERIFY2(NULL != ltest,"Faiure");
	int nRet;
// 	nRet = ltest->AddUser("abc","123",0,-1,-1);
// 	QVERIFY2(0 == nRet,"AddUser");
// 	nRet = ltest->RemoveUser("abc");
// 	QVERIFY2(0 == nRet,"RemoveUser");
	nRet = ltest->ModifyUserPassword("abc","123456");
	QVERIFY2(0 == nRet,"ModifyUserPasswd");
	nRet = ltest->ModifyUserLevel("abc",111);
	QVERIFY2(0 == nRet,"ModifyUserLevel");
	nRet = ltest->ModifyUserAuthorityMask("abc",123,123);
	QVERIFY2(0 == nRet,"ModifyUserAuthrityMask");
	bool isExist = ltest->IsUserExists("abc");
	QVERIFY2(1 == isExist,"IsUserExits");
	bool isTrue = ltest->CheckUser("abc","123456");
	QVERIFY2(1 == isTrue,"CheckUser");
	nRet = ltest->GetUserCount();
	QVERIFY2(4 == nRet,"GetUserCount");
}