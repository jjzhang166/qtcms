#include "configmgr.h"
#include "guid.h"
#include <QFile>

#include <QDebug>

#define qDebug() qDebug()<<__FUNCTION__<<__LINE__

ConfigMgr::ConfigMgr(QObject *parent)
	: QObject(parent)
{

}

ConfigMgr::~ConfigMgr()
{

}

long __stdcall ConfigMgr::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IConfigManager == iid){
		*ppv = static_cast<IConfigManager*>(this);
	}else if (IID_IPcomBase == iid){
		*ppv = static_cast<IPcomBase*>(this);
	}else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase*>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall ConfigMgr::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall ConfigMgr::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef--;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet){
		delete this;
	}
	return nRet;
}

int ConfigMgr::Import( const QString &sFilePath )
{
	if (!QFile::exists(sFilePath)){
		qDebug()<<"file "<<sFilePath<<" don't exist!";
		return 1;
	}
	QFile infile(sFilePath);
	if (!infile.open(QIODevice::ReadOnly)){
		qDebug()<<"read file "<<sFilePath<<" fail!";
		return 1;
	}
	sqlite3 *pdb = getSqlInterface();
	if (!pdb){
		qDebug()<<"get sqlite3 interface error!";
		return 1;
	}
	QDomDocument xml;
	xml.setContent(&infile);
	QDomNode sysNode = xml.elementsByTagName("system").at(0);
	QDomNodeList tablist = sysNode.childNodes();
	for (int tabNum = 0; tabNum < tablist.size(); tabNum++){
		QDomNode tabNode = tablist.at(tabNum);
		QString tabName = tabNode.toElement().attribute("name");
		if (importData(pdb, tabName, tabNode)){
			qDebug()<<"import table "<<tabName<<" fail";
			continue;
		}
	}
	releaseSqlInterface(pdb);
	infile.close();
	return 0;
}

int ConfigMgr::Export( const QString &sFilePath )
{
	//clear an existing file
	if (QFile::exists(sFilePath)){
		QDir dir;
		dir.remove(sFilePath);
	}
	QFile outfile(sFilePath);
	if (!outfile.open(QIODevice::WriteOnly)){
		qDebug()<<"open file "<<sFilePath<<" fail";
		return 1;
	}
	//get sqlite3 interface
	sqlite3 *pdb = getSqlInterface();
	if (!pdb){
		qDebug()<<"get sqlite3 interface error!";
		return 1;
	}
	//write xml head
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version = \'1.0\'");
	doc.appendChild(instruction);
	QDomElement root = doc.createElement("system");
	doc.appendChild(root);
	
	//search database
	//get table name
	char *pErr = NULL;
	sqlite3_stmt *pstmt = NULL;
	QString sql = "select name from sqlite_master where type='table';";
	int ret = sqlite3_prepare_v2(pdb, sql.toLatin1().data(), -1, &pstmt, (const char**)&pErr);
	if (SQLITE_OK != ret){
		qDebug()<<pErr;
		releaseSqlInterface(pdb);
		return 1;
	}
	QStringList tabList;
	ret = sqlite3_step(pstmt);
	while (SQLITE_ROW == ret){
		const char* txt = (const char*)sqlite3_column_text(pstmt, 0);
		if (qstrcmp("sqlite_sequence", txt)){
			tabList.append(QString(txt));
		}
		ret = sqlite3_step(pstmt);
	}
	ret = sqlite3_finalize(pstmt);
	if (SQLITE_OK != ret){
		qDebug()<<"finalize error!";
		releaseSqlInterface(pdb);
		return 1;
	}
	//search each table
	QDomElement table, item;
	foreach (QString tabName, tabList){
		QStringList columnList = getTableColumn(pdb, tabName);
		if (columnList.isEmpty()){
			qDebug()<<"get table "<<tabName<<" column fail";
			continue;
		}
		sql = QString("select %1 from %2;").arg(columnList.join(",")).arg(tabName);
		ret = sqlite3_prepare_v2(pdb, sql.toLatin1().data(), -1, &pstmt, (const char**)&pErr);
		if (SQLITE_OK != ret){
			qDebug()<<"read table "<<tabName<<" fail";
			continue;
		}
		//add table node
		table = doc.createElement("table");
		table.setAttribute("name", tabName);
		root.appendChild(table);
		//read table info
		ret = sqlite3_step(pstmt);
		while (SQLITE_ROW == ret){
			//read each item
			item = doc.createElement("item");
			for (int index = 0; index < columnList.size(); index++){
				QString attribute = columnList[index];
				const char* attrValue = (const char*)sqlite3_column_text(pstmt, index);
				item.setAttribute(attribute, QString(attrValue));
			}
			//add item node to table node
			table.appendChild(item);
			ret = sqlite3_step(pstmt);
		}
		ret = sqlite3_finalize(pstmt);
		if (SQLITE_OK != ret){
			qDebug()<<"finalize stmt error on table "<<tabName;
			continue;
		}
		//add table node to root node
		root.appendChild(table);
	}
	//write to file
	QTextStream out(&outfile);
	doc.save(out, 4);
	outfile.close();
	releaseSqlInterface(pdb);

	return 0;
}

sqlite3* ConfigMgr::getSqlInterface()
{
	QString path = QCoreApplication::applicationDirPath() + "/system.db";
	sqlite3 *pdb = NULL;
	int ret = sqlite3_open(path.toLatin1().data(), &pdb);
	if (SQLITE_OK != ret){
		qDebug()<<"open database "<<path<<" fail";
		return NULL;
	}
	return pdb;
}

void ConfigMgr::releaseSqlInterface( sqlite3* pdb )
{
	sqlite3_close(pdb);
}

int ConfigMgr::importData( sqlite3* pdb, QString tabName, QDomNode node )
{
	char *pErr = NULL;
	sqlite3_stmt *pstmt = NULL;
	//set journal_mode off
	int ret = sqlite3_exec(pdb, "pragma journal_mode =off", NULL, NULL, &pErr);
	if (SQLITE_OK != ret){
		qDebug()<<pErr;
		return 1;
	}
	//clear old data
	QString sql = QString("delete from %1;").arg(tabName);
	ret = sqlite3_prepare_v2(pdb, sql.toLatin1().data(), -1, &pstmt, (const char**)&pErr);
	if (SQLITE_OK != ret){
		qDebug()<<pErr;
		return 1;
	}
	sqlite3_step(pstmt);
	ret = sqlite3_finalize(pstmt);
	if (SQLITE_OK != ret){
		qDebug()<<"finalize stmt error in table "<<tabName;
		return 1;
	}
	//get table column name
	QStringList columnList = getTableColumn(pdb, tabName);
	if (columnList.isEmpty()){
		qDebug()<<"get table "<<tabName<<" column fail";
		return 1;
	}
	//get each item and insert into database
	QDomNodeList itemList = node.childNodes();
	for (int index = 0; index < itemList.size(); index++){
		QDomNode item = itemList.at(index);
		//fill sql
		sql = QString("insert into %1(%2) values(").arg(tabName).arg(columnList.join(","));
		for (int col = 0; col < columnList.size(); col++){
			QString column = columnList.at(col);
			sql += QString("'%1',").arg(item.toElement().attribute(column));
		}
		sql = sql.replace(sql.lastIndexOf(","), 1, ")");
		//insert into database
		ret = sqlite3_prepare_v2(pdb, sql.toLatin1().data(), -1, &pstmt, (const char**)&pErr);
		if (SQLITE_OK != ret){
			qDebug()<<"insert item error in table "<<tabName<<" error:"<<pErr;
			continue;
		}
		sqlite3_step(pstmt);
		sqlite3_finalize(pstmt);
	}
	return 0;
}

QStringList ConfigMgr::getTableColumn( sqlite3 *pdb, QString tabName )
{
	char *pErr = NULL;
	sqlite3_stmt *pstmt = NULL;
	QStringList columnList;
	QString sql = QString("PRAGMA table_info('%1');").arg(tabName);
	int ret = sqlite3_prepare_v2(pdb, sql.toLatin1().data(), -1, &pstmt, (const char**)&pErr);
	if (SQLITE_OK != ret){
		qDebug()<<pErr;
		return columnList;
	}
	ret = sqlite3_step(pstmt);
	while (SQLITE_ROW == ret){
		const char* txt = (const char*)sqlite3_column_text(pstmt, 1);
		QString column(txt);
		if (QString("id") != column){
			columnList.append(column);
		}
		ret = sqlite3_step(pstmt);
	}
	sqlite3_finalize(pstmt);
	return columnList;
}
