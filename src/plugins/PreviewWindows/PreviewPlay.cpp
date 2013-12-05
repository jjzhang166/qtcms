#include "PreviewPlay.h"


PreviewPlay::PreviewPlay(QThread *parent):QThread(parent)
{
}


PreviewPlay::~PreviewPlay(void)
{
}

void PreviewPlay::MyThreadPreviewPlay()
{
	//QVariantMap::const_iterator it;
	//for (it=evMap.begin();it!=evMap.end();++it)
	//{
	//	QString sKey=it.key();
	//	QString sValue=it.value().toString();
	//	qDebug()<<sKey;
	//	qDebug()<<sValue;
	//}
	qDebug()<<"MyThreadPreviewPlay";
}
