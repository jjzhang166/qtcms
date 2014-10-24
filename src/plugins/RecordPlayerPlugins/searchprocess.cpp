#include "searchprocess.h"
#include <QStringList>
#include "ILocalRecordSearchEx.h"
#include "guid.h"

#include <QDebug>

SearchProcess::SearchProcess()
	:m_fileKey(0)
{

}

SearchProcess::~SearchProcess()
{
// 	this->quit();
// 	this->wait();
	while (QThread::isRunning())
	{
		msleep(10);
	}
}

void SearchProcess::setPara( int wndId, QString date, QString start, QString end, int types )
{
	m_wndId = wndId;
	m_date = date;
	m_start = start;
	m_end = end;
	m_types = types;
}

void SearchProcess::setContext( ILocalRecordSearchEx *pLocalSch )
{
	if (!pLocalSch)
	{
		return;
	}
	m_pLocalRecordSearch = pLocalSch;
}

void SearchProcess::run()
{
	m_fileKey = 0;
	m_fileMap.clear();
	//get query interface
	ILocalRecordSearchEx *pRecSchEx = NULL;
	m_pLocalRecordSearch->QueryInterface(IID_ILocalRecordSearchEx, (void**)&pRecSchEx);
	if (NULL == pRecSchEx)
	{
		return;
	}
	pRecSchEx->searchVideoFileEx(m_wndId, m_date, m_start, m_end, m_types);
	pRecSchEx->Release();

	emit sigSchRet(m_wndId, m_fileMap);
}

void SearchProcess::transRecordFilesEx( QVariantMap &evMap )
{
	QString fileinfo;
	QStringList infoList;
	QVariantMap::const_iterator it;
	for(it=evMap.begin();it!=evMap.end();it++)
	{
		infoList<<"\\\"" + it.key() + "\\\":\\\"" + it.value().toString() + "\\\"";
	}
	fileinfo = "{" + infoList.join(",") + "}";
	QString key = "index_" + QString::number(m_fileKey);
	m_fileMap.insert(key,fileinfo);
	m_fileKey++;
}
