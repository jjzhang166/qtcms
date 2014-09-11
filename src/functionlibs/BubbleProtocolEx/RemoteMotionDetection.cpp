#include "bubbleprotocolex.h"

void BubbleProtocolEx::mdSocketStateChange(QAbstractSocket::SocketState sockState)
{

}

void BubbleProtocolEx::mdSocketConnetced()
{

}

void BubbleProtocolEx::mdSocketDisconnected()
{

}

void BubbleProtocolEx::mdSocketError(QAbstractSocket::SocketError socketError)
{

}

int BubbleProtocolEx::startMotionDetection()
{
	m_sockMD.connectToHost(m_tDeviceInfo.tIpAddr,m_tDeviceInfo.tPorts["http"].toInt(),QIODevice::ReadWrite);
	if (!m_sockMD.waitForConnected(5000))
	{
		qDebug("[%s:%d] %s:%s",__FILE__,__LINE__,__FUNCTION__,m_sockMD.errorString());
		return -1;
	}

	// keep alive
	m_sockMD.setSocketOption(QAbstractSocket::KeepAliveOption,QVariant(1));

	// nodelay
	m_sockMD.setSocketOption(QAbstractSocket::LowDelayOption,QVariant(1));

	// timer & work object thread
	m_MdWorkObj.moveToThread(&m_MdThread);
	m_MdTimer.moveToThread(&m_MdThread);
	m_MdThread.start();

	connect(&m_MdTimer,SIGNAL(timeout()),&m_MdWorkObj,SLOT(motionDetectionEcho()));
	// echo everytime
	m_MdTimer.setInterval(1000);
	m_MdTimer.start();

	m_bQuitMd = false;

	return 0;
}

int BubbleProtocolEx::stopMotionDetection()
{
	// 断开socket
	m_sockMD.disconnectFromHost();
	m_sockMD.close();
	
	// 关闭计时器
	m_MdTimer.stop();

	// 等待线程退出
	m_bQuitMd = true;
	m_MdThread.quit();
	m_MdThread.wait();
	return 0;
}

void BubbleProtocolEx::motionDetectionEcho()
{
	// 避免echo重入
	m_csMdEcho.lock();
	char sRecvBuffer[1024]; // 接受缓冲
	QString sPack;
	QString sLR;
	QString sContent;

	// 请求用户名密码
	QString sUserandPwd;
	sUserandPwd = m_tDeviceInfo.sUserName + QString(":") + m_tDeviceInfo.sPassword;
	QString sAuthorization(sUserandPwd.toAscii().toBase64());

	// 请求字符串
	QString sReq;
	sReq.clear();
	sReq += QString("GET /NetSDK/Video/motionDetection/channel/1/status HTTP/1.1\r\n");
	sReq += QString("Host: ") + m_tDeviceInfo.tIpAddr.toString() + QString("\r\n");
	sReq += QString("Connection: keep-alive\r\n");
	sReq += QString("Authorization: Basic ") + sAuthorization + QString("\r\n");
	sReq += QString("Cookie: juanipcam_lang=zh-cn\r\n\r\n");

	// 发送数据
	qint64 nTotleWrite = 0;
	do 
	{
		if (m_bQuitMd)
		{
			// 退出Md循环
			m_sockMD.disconnectFromHost();
			m_sockMD.close();

			m_csMdEcho.unlock();
			return;
		}

		qint64 nWrite = m_sockMD.write(sReq.toAscii().data() + nTotleWrite,sReq.length());
		if (-1 == nWrite)
		{
			// 写失败,关闭socket,退出本次echo
			m_sockMD.disconnectFromHost();
			m_sockMD.close();

			// 开启断线重连-- to be modified

			m_csMdEcho.unlock();
			return;
		}
		nTotleWrite += nWrite;

		if ( ! m_sockMD.waitForBytesWritten(1000) )
		{
			// 写超时,关闭socket,退出本次echo
			m_sockMD.disconnectFromHost();
			m_sockMD.close();

			// 开启断线重连-- to be modified
			m_csMdEcho.unlock();
			return;
		}
	} while (nTotleWrite < sReq.length());

	// 接受数据
	while(1) // to be modified 需要加入循环强制退出机制
	{
		if (m_bQuitMd)
		{
			// 退出Md循环
			m_sockMD.disconnectFromHost();
			m_sockMD.close();

			m_csMdEcho.unlock();
			return;
		}

		if (m_sockMD.waitForReadyRead(50))
		{
			qint64 nRead = m_sockMD.read(sRecvBuffer,sizeof(sRecvBuffer) - 1); // 预留结束符位置读取
			if (-1 == nRead)
			{
				// 读失败，关闭socket，退出本次echo
				m_sockMD.disconnectFromHost();
				m_sockMD.close();

				// 开启断线重连 -- to be modified
				m_csMdEcho.unlock();
				return;
			}

			sRecvBuffer[nRead] = 0;
			sPack += QString(sRecvBuffer);

			// 检查http头是否接收完整,\r\n\r\n或者\n\n,未接收完则返回循环继续接受，已接收完则继续执行后续代码
			sLR = QString("\r\n");
			if (!sPack.contains(QString("\r\n\r\n")))
			{
				// 未找到\r\n的行尾
				sLR = QString("\n");
				if (!sPack.contains(QString("\n\n")))
				{
					// 未找到\n的行尾，头未收完
					// 下个循环继续接收
					continue;
				}
			}

			// 获取content-length字段
			int nContentLength = 0;
			if (sPack.contains(QString("content-length"),Qt::CaseInsensitive))
			{
				int nContentLengthStart = sPack.indexOf(QString("content-length"),0,Qt::CaseInsensitive);
				int nContentLengthEnd = sPack.indexOf(sLR,nContentLengthStart);
				QString sContentLength = sPack.mid(nContentLengthStart,nContentLengthEnd = nContentLengthStart);
				sContentLength.remove(QChar(' '));
				int nContentLength = sContentLength.mid(sContentLength.indexOf(QChar(':')) + 1).toInt();
			}

			if (0 != nContentLength)
			{
				// 数据中带负载
				sContent = sPack.mid(sPack.indexOf(sLR + sLR) + QString(sLR + sLR).length());
				if (sContent.length() == nContentLength)
				{
					// 接收完成
					// 退出接收循环
					break;
				}
				else if (sContent.length() > nContentLength)
				{
					// 数据不正确
					m_sockMD.disconnectFromHost();
					m_sockMD.close();

					// 开启断线重连 -- to be modified
					m_csMdEcho.unlock();
					return;
				}
				else
				{
					// 需要更多数据
					continue;
				}
			}
			else
			{
				// 无content，接收完成
				// 退出接收循环
				break;
			}
		}
	}

	// 接收数据解析
	QString sFirstLine = sPack.left(sPack.indexOf(sLR));
	if (sFirstLine.contains(QString("200")))
	{
		// 200 OK
		if (sContent == "true")
		{
			QVariantMap eparam;
			eparam.insert("signal",QVariant(true));
			eventProcCall("MDSignal",eparam);
		}
		else if (sContent == "false")
		{
			QVariantMap eparam;
			eparam.insert("signal",QVariant(false));
			eventProcCall("MDSignal",eparam);
		}
	}
	else if (sFirstLine.contains(QString("404")))
	{
		// 404 page not found
	}


	m_csMdEcho.unlock();
}