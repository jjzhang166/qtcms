#include "settingsactivity.h"
#include <guid.h>
#include <QDebug>

settingsActivity::settingsActivity():
	m_nRef(0),
	m_bMouseTrace(false)
{

}

long __stdcall settingsActivity::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IActivities == iid)
	{
		*ppv = static_cast<IActivities *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall settingsActivity::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	qDebug("Addref:%d",m_nRef);
	return m_nRef;
}

unsigned long __stdcall settingsActivity::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

void settingsActivity::Active( QWebFrame * frame)
{
	m_MainView = (frame->page())->view();
	m_MainView->setMouseTracking(true);
	QWFW_MSGMAP_BEGIN(frame);
	QWFW_MSGMAP("top_act","dblclick","OnTopActDbClick()");
	QWFW_MSGMAP("top_act","mousedown","OnMouseDown()");
	QWFW_MSGMAP("top_act","mouseup","OnMouseUp()");
	QWFW_MSGMAP("top_act","mousemove","OnMouseMove()");
	QWFW_MSGMAP("max","click","OnMaxClick()");
	QWFW_MSGMAP("min","click","OnMinClick()");
	QWFW_MSGMAP("close","click","OnCloseClick()");
	QWFW_MSGMAP("add_ok","click","OnAddUserOk()");
	QWFW_MSGMAP("modify_ok","click","OnModifyUserOk()");
	QWFW_MSGMAP("delete_ok","click","OnDeleteUserOk()");
	QWFW_MSGMAP_END;
}

void settingsActivity::OnTopActDbClick()
{
	if (m_MainView->isMaximized())
	{
		m_MainView->showNormal();
		if (m_MainView->frameGeometry().width() < 950)
		{
			m_MainView->setFixedWidth(950);
		}
	}
	else
	{
		m_MainView->showMaximized();
	}
}

void settingsActivity::OnMinClick()
{
	m_MainView->showMinimized();
}

void settingsActivity::OnMaxClick()
{
	if (m_MainView->isMaximized())
	{
		m_MainView->showNormal();
		if (m_MainView->frameGeometry().width() < 950)
		{
			m_MainView->setFixedWidth(950);
		}
	}else
	{
		m_MainView->showMaximized();
	}
}

void settingsActivity::OnCloseClick()
{
	m_MainView->close();
}

void settingsActivity::OnMouseDown()
{
	m_pos = QCursor::pos();
	m_bMouseTrace = true;
}

void settingsActivity::OnMouseUp()
{
	m_bMouseTrace = false;
}

void settingsActivity::OnMouseMove()
{
	int nMainFormXcoord = m_MainView->frameGeometry().topLeft().x();
	int nMainFormYcoord = m_MainView->frameGeometry().topLeft().y();
	if (m_bMouseTrace)
	{
		if (! m_MainView->isMaximized())
		{
			QPoint curPos = QCursor::pos();
			int dx = curPos.x() - m_pos.x();
			int dy = curPos.y() - m_pos.y();
			nMainFormXcoord += dx;
			nMainFormYcoord += dy;
			m_MainView->move(nMainFormXcoord,nMainFormYcoord);
			m_pos = curPos;
		}
	}
}

void settingsActivity::OnAddUserOk()
{
	IUserManager *Iuser = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Iuser);
	if (NULL == Iuser)
	{
		return;
	}
	QVariant sUserName = QueryValue("add_username");
	QVariant sPassWd = QueryValue("add_passwd");
	QVariant sPassWd2 = QueryValue("add_again_passwd");
	QVariant sLevel = QueryValue("add_level");
	int nLevel = sLevel.toInt();
	int nMask1 = 0xFFFFFFFF;
	int nMask2 = 0xFFFFFFFF;
	Iuser->AddUser(sUserName.toString(),sPassWd.toString(),nLevel,nMask1,nMask2);
	
	Iuser->Release();
}

void settingsActivity::OnModifyUserOk()
{
	IUserManager *Iuser = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Iuser);
	if (NULL == Iuser)
	{
		return;
	}
	QVariant sUserName = QueryValue("user");
	QVariant sOldPassWd = QueryValue("modify_oldpasswd");
	//检查输入密码是否正确
	if (Iuser->CheckUser(sUserName.toString(),sOldPassWd.toString()))
	{
		QVariant sNewPassWd = QueryValue("modify_newpasswd");
		QVariant sNewPassWd2 = QueryValue("modify_again_passwd");

		//获取combox里的权限值，修改权限
		QVariant sLevel = QueryValue("modify_level");
		int nLevel = sLevel.toInt();
		Iuser->ModifyUserLevel(sUserName.toString(),nLevel);
		Iuser->ModifyUserPassword(sUserName.toString(),sNewPassWd.toString());	
	}
	else
	{
		QMessageBox::warning(m_MainView,"warning","passwd error!!!","Ok","Cancel");
		//event
	}
	Iuser->Release();
}

void settingsActivity::OnDeleteUserOk()
{
	IUserManager *Iuser = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Iuser);
	//获取用户列表
	QVariant sUser = QueryValue("user");
	if (sUser.toString().isEmpty())
	{
		QMessageBox::warning(m_MainView,"warning","please select user!!!","Ok","Cancel");
		return;
	}
	QStringList user_list = sUser.toString().split(",");
	for (int i = 0;i<user_list.size();i++)
	{
		sUser = user_list.at(i).toLatin1().data();
		if (Iuser->IsUserExists(sUser.toString()))
		{
			Iuser->RemoveUser(sUser.toString());
		}else
		{
			qDebug("not user.....................");
			//event
		}
	}
	Iuser->Release();
}
