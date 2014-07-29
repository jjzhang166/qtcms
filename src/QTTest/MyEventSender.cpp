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
		if (e->type()==QEvent::Close)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"CMS CLOSE EVENT";
		}
	}catch(QEvent *pe){
		qDebug()<<__FUNCTION__<<__LINE__<<"there catch a exception"<<receiver<<pe->type();
		throw;
	}catch(std::exception &exp){
		qDebug()<<__FUNCTION__<<__LINE__<<"there catch a exception"<<exp.what();
		throw exp;
	}
	catch(...){
		qDebug()<<__FUNCTION__<<__LINE__<<"unkown exception";
		throw ;
	}
	return bFlag;
}
