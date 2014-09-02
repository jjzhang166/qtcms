#include "SearchRemoteFile.h"
#include <QDebug>

SearchRemoteFile::SearchRemoteFile(void)
{
	m_tEventNameList<<"foundFile"<<"recFileSearchFinished"<<"recFileSearchFail";
}


SearchRemoteFile::~SearchRemoteFile(void)
{
}

int SearchRemoteFile::startSearchRecFile( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime ,QHostAddress tAddr,QVariantMap tPorts,QString sUserName,QString sPassWord)
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	if (!(nTypes<0||startTime>endTime))
	{
		m_tSearchInfo.nSessionCount=100;
		m_tSearchInfo.nSessionIndex=0;
		m_tSearchInfo.nSessionTotal=0;
		bool bSearchStop=false;
		int nSearchStep=0;
		int nWaitTime=0;
		int nFlag=-1;
		while (bSearchStop==false)
		{
			switch (nSearchStep)
			{
			case 0:{
				//连接到设备
				m_tSearchRemoteFileTcpSocket.connectToHost(tAddr.toString(),(quint16)tPorts["media"].toInt());
				if (m_tSearchRemoteFileTcpSocket.waitForConnected(5000))
				{
					nSearchStep=1;
				}else{
					nSearchStep=5;
					qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as it connect to device fail";
				}
				   }
				break;
			case 1:{
				//发送数据
				QString post="POST /cgi-bin/gw.cgi HTTP/1.1\r\n";
				QString content_type="Content-Type: application/x-www-form-urlencoded\r\n";			
				QString connecttion="Connection: Keep-Alive\r\n";
				QString accept_encoding="Accept-Encoding: gzip\r\n";
				QString accept_language="Accept-Language: zh-CN,en,*\r\n";
				QString user_agent="User-Agent: Mozilla/5.0\r\n";
				QString host="Host: ";
				host.append(tAddr.toString()).append(":").append(QString::number(tPorts["media"].toInt())).append("\r\n\r\n");
				QString sendData(QString("<juan ver=\"%1\" squ=\"%2\" dir=\"%3\">\n    <recsearch usr=\"%4\" pwd=\"%5\" channels=\"%6\" types=\"%7\" date=\"%8\" begin=\"%9\" end=\"%10\" session_index=\"%11\" session_count=\"%12\" />\n</juan>\n").arg("").arg(1).arg("").arg(sUserName).arg(sPassWord).arg(nChannel).arg(nTypes).arg(startTime.date().toString("yyyy-MM-dd")).arg(startTime.time().toString("hh:mm:ss")).arg(endTime.time().toString("hh:mm:ss")).arg(m_tSearchInfo.nSessionIndex).arg(m_tSearchInfo.nSessionCount));
				QString content_length="Content-Length: ";
				content_length.append(QString::number(sendData.size())).append("\r\n");
				QString block=post+content_type+content_length+connecttion+accept_encoding+accept_language+user_agent+host+sendData;
				m_tSearchRemoteFileTcpSocket.write(block.toAscii());
				if (m_tSearchRemoteFileTcpSocket.waitForBytesWritten(2000))
				{
					nSearchStep=2;
				}else{
					nSearchStep=4;
					qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as it write data to socket fail";
				}
				   }
				   break;
			case 2:{
				//接受数据
				if (m_tSearchRemoteFileTcpSocket.waitForReadyRead(3000))
				{
					nSearchStep=3;//有数据，去处理
					nWaitTime--;
				}else{
					if (nWaitTime>2)
					{
						nSearchStep=4;//不再等待，断开连接
						qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as it wait for remote data outTime"<<nWaitTime;
					}else{
						nSearchStep=2;
					}
					nWaitTime++;
				}
				   }
				   break;
			case 3:{
				//处理数据

				//0:解析完整
				//1:等待完整数据
				//2:解析错误
				int nValue=parseSearchData();
				nFlag=nValue;
				if (nValue==0)
				{
					//判断是否需要重新搜索
					if (m_tSearchInfo.nSessionTotal>m_tSearchInfo.nSessionIndex+100)
					{
						//搜索下一百个文件
						nSearchStep=0;
						m_tSearchInfo.nSessionIndex+=100;
						nWaitTime=0;
						nFlag=-1;
						m_tSearchRemoteFileTcpSocket.disconnectFromHost();
					}else{
						//搜索完成，结束
						nSearchStep=4;
					}
				}else if (nValue==1)
				{
					//接着等待数据
					nSearchStep=2;
				}else{
					//解析错误，断开连接
					nSearchStep=4;
					qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as parseSearchData fail";
				}
				   }
				   break;
			case 4:{
				//断开连接
				nSearchStep=5;
				m_tSearchRemoteFileTcpSocket.disconnectFromHost();
				   }
			case 5:{
				//结束
				bSearchStop=true;
				qDebug()<<__FUNCTION__<<__LINE__<<"stop search remote file";
				   }
			}
		}
		//搜索结果，抛出给外界
		if (nFlag==0)
		{
			return 0;
		}else{
			if (m_tRecordList.isEmpty())
			{
				QVariantMap tItem;
				tItem.insert("parm", QString("%1").arg(2));
				eventCallBack(QString("recFileSearchFail"), tItem); 
				qDebug()<<__FUNCTION__<<__LINE__<<"SearchRemoteFile fail and m_tRecordList is total empty";
				return 1;
			}else{
				QVariantMap tItem;
				tItem.insert("parm", QString("%1").arg(3));
				eventCallBack(QString("recFileSearchFail"), tItem); 
				qDebug()<<__FUNCTION__<<__LINE__<<"it can search some remote file,but not total";
				return 1;
			}
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"startSearchRecFile fail as the input params are unCorrect"<<"nTypes::"<<nTypes<<"startTime::"<<startTime.toString()<<"endTime::"<<endTime.toString();
		QVariantMap tItem;
		tItem.insert("parm", QString("%1").arg(1));
		eventCallBack(QString("recFileSearchFail"), tItem); 
		return 2;
	}
}

void SearchRemoteFile::registerEvent( QString eventName,int (__cdecl *proc)(QString ,QVariantMap ,void *),void *pUser )
{
	if (!m_tEventNameList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event:"<<eventName<<"fail,as the module do not support";
		return;
	}else{
		tagSearchRemoteFileProcInfo tProcInfo;
		tProcInfo.Proc=proc;
		tProcInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProcInfo);
		return;
	}
}

void SearchRemoteFile::eventCallBack( QString sEventName,QVariantMap evMap )
{
	if (m_tEventNameList.contains(sEventName))
	{
		tagSearchRemoteFileProcInfo tProcInfo=m_tEventMap.value(sEventName);
		if (NULL!=tProcInfo.Proc)
		{
			tProcInfo.Proc(sEventName,evMap,tProcInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<" is not register,please check";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the module do not support the event of "<<sEventName;
	}
}

int SearchRemoteFile::parseSearchData()
{
	//0:解析完整
	//1:等待完整数据
	//2:解析错误
	int nTotalSize=0;
	m_tBlock+=m_tSearchRemoteFileTcpSocket.readAll();
	if (m_tBlock.contains("HTTP/1.1 200"))
	{
		if (m_tBlock.contains("Content-Length"))
		{
			int nPos=m_tBlock.indexOf("Content-Length: ");
			nPos+=QString("Content-Length: ").size();
			int nWSize=m_tBlock.indexOf("\r\n",nPos);
			int nHeadSize=nWSize;
			nWSize=nWSize-nPos;
			nTotalSize=m_tBlock.mid(nPos,nWSize).toInt()+nHeadSize;
			if (m_tBlock.size()<nTotalSize)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"wait for more data come";
				return 1;
			}else{
				int nPos=m_tBlock.indexOf("<");
				QString sXml=m_tBlock.mid(nPos,m_tBlock.lastIndexOf(">")+1-nPos);
				QDomDocument *pDom=new QDomDocument();
				if (!pDom->setContent(sXml))
				{
					delete pDom;
					pDom=NULL;
					m_tBlock.clear();
					qDebug()<<__FUNCTION__<<__LINE__<<"parseSearchData fail as pDom->setContent(sXml) fail";
					return 2;
				}else{
					if (extractRecordInfo(pDom)==0)
					{
						m_tBlock.clear();
						delete pDom;
						pDom=NULL;
						return 0;
					}else{
						m_tBlock.clear();
						delete pDom;
						pDom=NULL;
						qDebug()<<__FUNCTION__<<__LINE__<<"parseSearchData fail as extractRecordInfo fail";
						return 2;
					}
				}
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"wait for more data come";
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"parseSearchData fail as data do not contains :HTTP/1.1 200";
		m_tBlock.clear();
		return 2;
	}
}

int SearchRemoteFile::extractRecordInfo( QDomDocument* pDom )
{
	//0:true
	//1:false
	if (NULL!=pDom)
	{
		QDomNodeList tSearchNodeList=pDom->elementsByTagName("recsearch");
		QDomNode tSubNode=tSearchNodeList.at(0);
		int nTotalNum=tSubNode.toElement().attribute("session_total").toInt();
		QDomNodeList tRecordList=pDom->elementsByTagName("s");
		int nRecordNum=tRecordList.size();
		QVariantMap tRecordTotal;
		tRecordTotal.insert("total",QString("%1").arg(nTotalNum));
		if (m_tSearchInfo.nSessionIndex==0)
		{
			eventCallBack("recFileSearchFinished",tRecordTotal);
		}else{
			//do nothing
		}
		for(int i=0;i<nRecordNum;i++){
			tagSearchRecordInfo tRecordInfo;
			QDomNode tRecNode=tRecordList.at(i);
			QDomElement tElement=tRecNode.toElement();
			QString sRecordValue=tElement.text();
			QStringList tStrList=sRecordValue.split("|");
			setRecordInfo(tRecordInfo,tStrList);
			m_tRecordList.append(tRecordInfo);
			QVariantMap tItem;
			tItem.insert("channelnum", tRecordInfo.cChannel+1);
			tItem.insert("types", tRecordInfo.cTypes);
			tItem.insert("start", tRecordInfo.tStartTime.toString("yyyy-MM-dd hh:mm:ss"));
			tItem.insert("end", tRecordInfo.tEndTime.toString("yyyy-MM-dd hh:mm:ss"));
			tItem.insert("filename", tRecordInfo.sFileName);
			tItem.insert("index",m_tSearchInfo.nSessionIndex+i);
			eventCallBack("foundFile",tItem);
		}
		m_tSearchInfo.nSessionTotal=nTotalNum;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"extractRecordInfo fail as pDom is null";
		return 1;
	}
	return 0;
}

void SearchRemoteFile::setRecordInfo( tagSearchRecordInfo &tRecord,QStringList tStrList )
{
	if (!tStrList.isEmpty())
	{
		QDateTime time1 = QDateTime::currentDateTime();
		QDateTime time2 = QDateTime::currentDateTimeUtc();
		int timeDifference = qAbs(time1.time().hour() - time2.time().hour())*3600;

		int nTypeNum=tStrList.at(3).toInt();
		tRecord.cChannel=tStrList.at(2).toInt();
		tRecord.cTypes=nTypeNum;
		tRecord.tStartTime=QDateTime::fromTime_t(tStrList.at(4).toInt());
		tRecord.tStartTime=tRecord.tStartTime.addSecs(0-timeDifference);
		tRecord.tEndTime=QDateTime::fromTime_t(tStrList.at(5).toInt());
		tRecord.tEndTime=tRecord.tEndTime.addSecs(0-timeDifference);
		tRecord.sFileName=tRecord.tStartTime.toString("yyyyMMddhhmmss")+"_"+tRecord.tEndTime.toString("yyyyMMddhhmmss");

		QString sType;
		if (1 == (int)(nTypeNum & 1) )
		{
			sType += "T";
		}
		if (2 == (int)(nTypeNum & 2))
		{
			sType += "M";
		}
		if (4 == (int)(nTypeNum & 4))
		{
			sType += "S";
		}
		if (8 == (int)(nTypeNum & 8))
		{
			sType += "H";
		}
		tRecord.sFileName+="_"+sType;
		return;
	}else{
		return;
	}
}
