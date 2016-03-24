#include "simsocket.h"


#include "rdpsimhandler.h"

SimSocket::SimSocket()
{

}

SimSocket::~SimSocket()
{

}

void SimSocket::init(QString sessionId, bool isControl, int handle, int vncPort)
{
	m_sessionId = sessionId;
	m_isControl = isControl;
	m_socketDescriptor = handle;

	m_tcpSocket = NULL;
	m_forceClose = false;

    m_vncPort = vncPort;

    connect(this, SIGNAL(sigCreateSocket()), this, SLOT(onNewSocket()));
    connect(this, SIGNAL(sigClose()), this, SLOT(onCloseConnection()));
    connect(this, SIGNAL(sigWrite(const QByteArray&)), this, SLOT(onWrite(const QByteArray&)));
}


void SimSocket::createSocket()
{
    emit sigCreateSocket();
}

void SimSocket::onNewSocket()
{
	m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(onSimConnected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onSimRead()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSimError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onSimDisconnected()));

	if (m_isControl)
	{
        m_tcpSocket->setSocketDescriptor(m_socketDescriptor);
        onSimConnected();
	}
	else
	{
        m_tcpSocket->connectToHost("127.0.0.1", m_vncPort);
	}
}

void SimSocket::write(const QByteArray& buf)
{
    emit sigWrite(buf);
}

void SimSocket::onWrite(const QByteArray& buf)
{
	QMutexLocker locker(&m_bufMutex);
	m_buf.append(buf);

	//LOG_DEBUG() << "buf size=" << buf.size() << buf.toHex();

	if (m_tcpSocket == NULL || m_buf.isEmpty())
	{
		return;
	}

	int writelen = 0;
	do 
	{
		if (!m_tcpSocket->isValid())
		{
			qDebug() << "Connection::write error 1. sessionid" << m_sessionId << "error string:" << m_tcpSocket->errorString();
			LOG_ERROR()  << "Connection::write error 1. sessionid" << m_sessionId << "len=" << m_buf.size() << "writelen=" << writelen << "error string:" << m_tcpSocket->errorString();
			break;
		}
		int writesize = m_tcpSocket->write(m_buf.mid(writelen, m_buf.size() - writelen), m_buf.size() - writelen);
		if (writesize == -1)
		{
			qDebug() << "Connection::write error 2. sessionid" << m_sessionId;
			LOG_ERROR()  << "Connection::write error 2. sessionid" << m_sessionId << "len=" << m_buf.size() << "writelen=" << writelen << "error string:" << m_tcpSocket->errorString();
			break;
		}
		writelen += writesize;
		//m_sendBuf.remove(0, writesize);

	} while (m_buf.size() > writelen);

	m_buf.clear();
}




// viewer模拟器连接成功，server端
void SimSocket::onSimConnected()
{
	LOG_DEBUG()  << "sessionId=" << m_sessionId;
	emit sigSimConnected();
}

void SimSocket::onSimRead()
{
	if (m_tcpSocket == NULL || m_tcpSocket->bytesAvailable() <= 0)
	{
		return;
	}

	QByteArray readData = m_tcpSocket->readAll();
    emit sigSimRead(readData);
}


void SimSocket::onSimError(QAbstractSocket::SocketError errCode)
{
	LOG_DEBUG()  << "sessionId=" << m_sessionId << "error:" << errCode;

	if (!m_forceClose)
	{
		// 控制端与viewer的链接
        emit sigSimError(m_tcpSocket->errorString());
	}
}

void SimSocket::onSimDisconnected()
{
	LOG_DEBUG()  << "sessionId=" << m_sessionId;
	if (!m_forceClose)
	{
		// 控制端与viewer的链接
		emit sigSimDisconnected();
	}
}


void SimSocket::close()
{
    emit sigClose();
}

void SimSocket::onCloseConnection()
{
	LOG_DEBUG()  << "sessionId=" << m_sessionId;
	disconnect(this, 0);

	if (m_tcpSocket)
	{
		m_forceClose = true;
		m_tcpSocket->abort();
		delete m_tcpSocket;
		m_tcpSocket = NULL;
	}
	else
	{
		LOG_DEBUG()  << "m_tcpSocket is null."  << "sessionId=" << m_sessionId;
	}
	
}