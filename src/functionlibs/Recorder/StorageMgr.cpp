#include "StorageMgr.h"
#include "guid.h"
#include <QDir>
#include <QDateTime>

#include <QList>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")

StorageMgr::StorageMgr(void):
	m_pDisksSetting(NULL),
	m_currdisk('0')
{
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDiskSetting,(void **)&m_pDisksSetting);
	if (!m_pDisksSetting)
	{
		qDebug("can not create diskssetting instance");
	}

}


StorageMgr::~StorageMgr(void)
{
	if (m_pDisksSetting)
	{
		m_pDisksSetting->Release();
	}
}

int StorageMgr::getFilePackageSize()
{
	int filesize = 128;
	if (m_pDisksSetting)
	{
		m_pDisksSetting->getFilePackageSize(filesize);
	}

	return filesize;
}
QString StorageMgr::getUseDisks()
{
	QString sDisks;
	if (m_pDisksSetting)
	{
		m_pDisksSetting->getUseDisks(sDisks);
	}
	return sDisks;
}
bool StorageMgr::getLoopRecording()
{
	bool loop = false;
	if (m_pDisksSetting)
	{
		loop = getLoopRecording();
	}
	return loop;
}
int StorageMgr::getFreeSizeForDisk()
{
	int spacereservedsize;
	if (m_pDisksSetting)
	{
		m_pDisksSetting->getDiskSpaceReservedSize(spacereservedsize);
	}
	return spacereservedsize;
}

QString StorageMgr::getFileSavePath(QString devname,int nChannelNum)
{
	QString filesavepath = "none";
	
	QString udisk = getUsableDisk();
	if ("0" != udisk)
	{
		filesavepath = udisk + ":/REC";
		QDateTime datetime = QDateTime::currentDateTime();//获取系统现在的时间
		filesavepath += "/"+datetime.toString("yyyy-MM-dd"); 

		filesavepath += "/"+devname;

		char sChannelNum[3];
		sprintf(sChannelNum,"%02d",nChannelNum+1);
		filesavepath += "/CHL" + QString("%1").arg(QString(sChannelNum));

		filesavepath += "/" + datetime.toString("hhmmss") + ".avi";
	}

	return filesavepath;
}

QString StorageMgr::getUsableDisk()//返回'0'说明没有找到满足条件的分区
{
	QString qsdisks ; 
	//char getdisk = '0';
	QString getdisk = "0";
	int retrycount = 3;
	int freesizem;
	bool brecove = m_pDisksSetting->getLoopRecording();
	//使用默认大小
	if(0 != m_pDisksSetting->getDiskSpaceReservedSize(freesizem))
		freesizem = 128;

	//查找满足条件的分区
	if (0 == m_pDisksSetting->getUseDisks(qsdisks))
	{
		QStringList sdlist = qsdisks.split(":");
		if (0 != sdlist.size())
		{
			while(retrycount>0)
			{
				foreach(QString stritem,sdlist)
				{
					quint64 FreeByteAvailable;
					quint64 TotalNumberOfBytes;
					quint64 TotalNumberOfFreeBytes;
					QString sdisk = stritem+":";
					if (GetDiskFreeSpaceExQ(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes))
					{
						if (TotalNumberOfFreeBytes > (quint64)freesizem * 1024 * 1024)
						{
							getdisk = stritem;
							break;
						}
					}
				}

				if (brecove && "0" == getdisk)
				{
					deleteOldDir(sdlist);
				}

				retrycount --;
			}
			
		}

		
	}

	return getdisk;
	
}

void StorageMgr::deleteOldDir(const QStringList& diskslist)
{

	int deletetotalsize;
	int freesizem;
	if(0 != m_pDisksSetting->getDiskSpaceReservedSize(freesizem))
		freesizem = 128;

	freesizem=freesizem+200;
	bool flag=false;
	QList<unsigned int> datetimelist;
	QDateTime earliestTime;
	earliestTime = QDateTime::fromString("2030-12-31","yyyy-MM-dd");
	foreach(QString sdisk,diskslist)
	{
		QString spath = sdisk+":/REC/";
		QDir dir(spath);
		dir.setFilter(QDir::AllDirs);
		QFileInfoList fileList = dir.entryInfoList();
		foreach(QFileInfo fi,fileList)
		{
			QDateTime dtime = QDateTime::fromString(fi.fileName(),"yyyy-MM-dd");
			if (!datetimelist.contains(dtime.toTime_t()))
			datetimelist.append(dtime.toTime_t());
			//if (dtime<earliestTime&&dtime.toString("yyyy-MM-dd")!="")
			//	earliestTime = dtime;
		}
	}

	foreach(QString sdisk,diskslist)
	{
		unsigned int oldestdate=1900000000;
		foreach(unsigned int datelist,datetimelist){
			oldestdate=qMin(oldestdate,datelist);
			QDateTime datetime=QDateTime::fromTime_t(datelist);
		}
		datetimelist.removeAt(datetimelist.indexOf(oldestdate));
		QDateTime earliestTime=QDateTime::fromTime_t(oldestdate);
		QString spath = sdisk+":/REC/" + earliestTime.toString("yyyy-MM-dd");
		QDir dir(spath);
		dir.setFilter(QDir::AllDirs);
		QFileInfoList fileList = dir.entryInfoList();

		foreach(QFileInfo fi,fileList)
		{
			QString spath=sdisk+":/REC/" + earliestTime.toString("yyyy-MM-dd")+"/"+fi.fileName();
			if (fi.fileName()=="."||fi.fileName()=="..")
			{
				continue;
			}
			QDir dir(spath);
			dir.setFilter(QDir::AllDirs);
			QFileInfoList filechllist=dir.entryInfoList();
			foreach(QFileInfo fil,filechllist){
				QString spath=sdisk+":/REC/" + earliestTime.toString("yyyy-MM-dd")+"/"+fi.fileName()+"/"+fil.fileName();
				if (fil.fileName()!="."&&fil.fileName()!="..")
				{
					QDir dir(spath);
					dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
					QFileInfoList fileList = dir.entryInfoList();
					foreach(QFileInfo finame,fileList){
						QString spath=sdisk+":/REC/" + earliestTime.toString("yyyy-MM-dd")+"/"+fi.fileName()+"/"+fil.fileName()+"/"+finame.fileName();
						QFile file(spath);
						if (file.exists())
						{
							freesizem=freesizem-file.size();
						}else{
							freesizem=freesizem-fil.size();
						}				
						deleteDir(spath);
						if (freesizem<0)
						{
							flag=true;
							break;
						}
					}
				}		
			}
			if (flag==true)
				break;
		}
		if (flag==true)
			break;
	}

}

bool StorageMgr::deleteDir(const QString& dirpath)
{
	if (dirpath.isEmpty())
		return false;
	QFile file(dirpath);
	if (file.exists())
	{
		QFile::remove(dirpath);
		return true;
	}
	QDir dir(dirpath);
	if (!dir.exists())
		return true;

	dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();
	foreach(QFileInfo fi,fileList)
	{
		if (fi.isFile())
			fi.dir().remove(fi.fileName());
		else
			deleteDir(fi.absoluteFilePath());
	}
	return dir.rmpath(dir.absolutePath());
}

bool StorageMgr::GetDiskFreeSpaceEx(char* lpDirectoryName, quint64* lpFreeBytesAvailableToCaller, quint64* lpTotalNumberOfBytes, quint64* lpTotalNumberOfFreeBytes)
{
	return GetDiskFreeSpaceExQ(lpDirectoryName,lpFreeBytesAvailableToCaller,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes);
}

