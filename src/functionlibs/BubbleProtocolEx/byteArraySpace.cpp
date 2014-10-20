#include "byteArraySpace.h"


byteArraySpace::byteArraySpace(void):m_nDataLength(0),
	m_nTotalSize(0),
	m_pMemoryHead(NULL),
	m_pDataHead(NULL)
{
}


byteArraySpace::~byteArraySpace(void)
{
	if (NULL!=m_pMemoryHead)
	{
		free(m_pMemoryHead);
		m_pMemoryHead=NULL;
	}
	m_pDataHead=NULL;
}

char * byteArraySpace::getEndLocation(int &nResidueSize)
{
	if (m_pDataHead!=m_pMemoryHead)
	{
		if (m_nDataLength==0)
		{
			m_pDataHead=m_pMemoryHead;
			m_nNOCopy++;
		}else if (m_nTotalSize-(m_pDataHead-m_pMemoryHead)>FRAMEMAXSIZE)
		{
			//do nothing
			m_nDonothing++;
		}else{
			memcpy(m_pMemoryHead,m_pDataHead,m_nDataLength);
			m_pDataHead=m_pMemoryHead;
			m_nCopy++;
		}
	}else{
		//do nothing
	}
	m_nCount++;
	if (m_nCount%25==0)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_nCopy:"<<m_nCopy<<"m_nNOCopy:"<<m_nNOCopy<<"m_nDonothing:"<<m_nDonothing;
	}
	nResidueSize=m_nTotalSize-m_nDataLength-(m_pDataHead-m_pMemoryHead)-1;
	return (char*)(m_pDataHead+m_nDataLength);
}

void byteArraySpace::setSize( int nSize )
{
	if (NULL==m_pMemoryHead)
	{
		m_pMemoryHead=(char*)malloc(nSize*sizeof(char));
		memset(m_pMemoryHead,0,sizeof(char)*nSize);
		m_pDataHead=m_pMemoryHead;
		m_nTotalSize=nSize;
		m_nDataLength=0;
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"size had been set,there is no need to call this function";
	}
	m_nCopy=m_nNOCopy=m_nCount=m_nDonothing=0;
	return;
}

bool byteArraySpace::contains( const char * str )
{
	memset(m_pDataHead+m_nDataLength,0,1);
	if (NULL==strstr(m_pDataHead,str))
	{
		return false;
	}else{
		return true;
	}
}

int byteArraySpace::size()
{
	return m_nDataLength;
}

bool byteArraySpace::startsWith(  char * str )
{
	memset(m_pDataHead+m_nDataLength,0,1);
	if (!str || !*str)
		return true;
	int len = qstrlen(str);
	if (m_nDataLength < len)
		return false;
	return qstrncmp(m_pDataHead, str, len) == 0;
}

char * byteArraySpace::data()
{
	return m_pDataHead;
}

void byteArraySpace::remove( int nPos,int len )
{
	if (nPos!=0)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"only support from zero position";
		abort();
	}
	if (len>=m_nDataLength)
	{
		m_nDataLength=0;
	}else{
		m_nDataLength=m_nDataLength-len;
	}
	if (len>=m_nTotalSize-(m_pDataHead-m_pMemoryHead))
	{
		m_pDataHead=m_pMemoryHead+m_nTotalSize;
	}else{
		m_pDataHead=(char*)(m_pDataHead+len);
	}
	return;
}

bool byteArraySpace::clear()
{
	memset(m_pMemoryHead,0,sizeof(char)*m_nTotalSize);
	m_pDataHead=m_pMemoryHead;
	m_nDataLength=0;
	m_nCopy=m_nNOCopy=m_nCount=m_nDonothing=0;
	return true;
}

int byteArraySpace::indexOf( char *str )
{
	QByteArray tByteArray(m_pDataHead,m_nDataLength);
	return tByteArray.indexOf(str);
}

QString byteArraySpace::mid( int nPos,int nLen )
{
	QByteArray tByteArray(m_pDataHead,m_nDataLength);
	return tByteArray.mid(nPos,nLen);
}

void byteArraySpace::addDataLenth( int nSize )
{
	m_nDataLength=m_nDataLength+nSize;
	if (m_nDataLength>m_nTotalSize)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_nDataLength out of range"<<"m_nDataLength:"<<m_nDataLength<<"m_nTotalSize:"<<m_nTotalSize;
		abort();
	}
}
