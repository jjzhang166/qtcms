#ifndef SEARCHPROCESS_H
#define SEARCHPROCESS_H

#include <QThread>
#include <QVariantMap>
#include "ILocalRecordSearch.h"

class SearchProcess : public QThread
{
	Q_OBJECT

public:
	SearchProcess();
	~SearchProcess();
	void setPara(int wndId, QString date, QString start, QString end, int types);
	void setContext(ILocalRecordSearch *pLocalSch);
	void transRecordFilesEx( QVariantMap &evMap );

signals:
	void sigSchRet(int wnd, QVariantMap vmap); 
protected:
	void run();
private:
	int m_wndId;
	int m_types;
	int m_fileKey;
	QString m_date;
	QString m_start;
	QString m_end;
	QVariantMap m_fileMap;

	ILocalRecordSearch *m_pLocalRecordSearch;
};

#endif // SEARCHPROCESS_H
