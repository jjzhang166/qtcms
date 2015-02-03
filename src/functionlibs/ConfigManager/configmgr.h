#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include <QObject>
#include "IConfigManager.h"
#include <QMutex>
#include <QtXml>
#include "sqlite3.h"

class ConfigMgr : public QObject,
	public IConfigManager
{
	Q_OBJECT

public:
	ConfigMgr(QObject *parent = 0);
	~ConfigMgr();
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();
	//IConfigManager
	virtual int Import(const QString &sFilePath);
	virtual int Export(const QString &sFilePath);	
private:
	sqlite3* getSqlInterface();
	QStringList getTableColumn(sqlite3 *pdb, QString tabName);
	void releaseSqlInterface(sqlite3 *pdb);
	int importData(sqlite3* pdb, QString tabName, QDomNode node);
private:
	int m_nRef;
	QMutex m_csRef;
};

#endif // CONFIGMGR_H
