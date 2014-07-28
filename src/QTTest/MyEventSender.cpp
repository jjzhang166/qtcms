#include "MyEventSender.h"
#include <QDebug>

MyEventSender::MyEventSender(int argc,char *argv[]):QApplication(argc,argv)
{
	
}


MyEventSender::~MyEventSender(void)
{
}

bool MyEventSender::notify( QObject *receiver, QEvent *e )
{
	bool bFlag=false;
	try
	{
		bFlag=QApplication::notify(receiver,e);
	}catch(QEvent *){
		qDebug()<<__FUNCTION__<<__LINE__<<"there catch a exception"<<receiver<<e->type();
		throw;
	}
	return bFlag;
}
